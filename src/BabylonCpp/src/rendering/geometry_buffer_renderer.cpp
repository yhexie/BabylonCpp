#include <babylon/rendering/geometry_buffer_renderer.h>

#include <babylon/bones/skeleton.h>
#include <babylon/core/string.h>
#include <babylon/engine/engine.h>
#include <babylon/engine/scene.h>
#include <babylon/materials/effect.h>
#include <babylon/materials/effect_creation_options.h>
#include <babylon/materials/material.h>
#include <babylon/materials/textures/multi_render_target.h>
#include <babylon/mesh/_instances_batch.h>
#include <babylon/mesh/abstract_mesh.h>
#include <babylon/mesh/mesh.h>
#include <babylon/mesh/sub_mesh.h>
#include <babylon/mesh/vertex_buffer.h>

namespace BABYLON {

GeometryBufferRenderer::GeometryBufferRenderer(Scene* scene, float ratio)
    : _scene{scene}
    , _multiRenderTarget{nullptr}
    , _effect{nullptr}
    , _viewMatrix{Matrix::Zero()}
    , _projectionMatrix{Matrix::Zero()}
    , _transformMatrix{Matrix::Zero()}
    , _worldViewProjection{Matrix::Zero()}
    , _cachedDefines{""}
{
  auto engine = scene->getEngine();

  // Render target
  IMultiRenderTargetOptions options;
  options.generateMipMaps      = false;
  options.generateDepthTexture = true;
  _multiRenderTarget           = std::make_unique<MultiRenderTarget>(
    "gBuffer", Size{static_cast<int>(engine->getRenderWidth() * ratio),
                    static_cast<int>(engine->getRenderHeight() * ratio)},
    2, _scene, options);
  if (!isSupported()) {
    return;
  }

  _multiRenderTarget->wrapU = TextureConstants::CLAMP_ADDRESSMODE;
  _multiRenderTarget->wrapV = TextureConstants::CLAMP_ADDRESSMODE;
  _multiRenderTarget->setRefreshRate(1);
  _multiRenderTarget->renderParticles = false;
  _multiRenderTarget->renderList      = {};

  // set default depth value to 1.0 (far away)
  _multiRenderTarget->onClearObservable.add([](Engine* engine) {
    engine->clear(Color4(0.f, 0.f, 0.f, 1.f), true, true, true);
  });

  // Custom render function
  _multiRenderTarget->customRenderFunction
    = [this](const std::vector<SubMesh*>& opaqueSubMeshes,
             const std::vector<SubMesh*>& /*transparentSubMeshes*/,
             const std::vector<SubMesh*>& alphaTestSubMeshes) {

        for (auto& opaqueSubMesh : opaqueSubMeshes) {
          renderSubMesh(opaqueSubMesh);
        }

        for (auto& alphaTestSubMesh : alphaTestSubMeshes) {
          renderSubMesh(alphaTestSubMesh);
        }
      };
}

GeometryBufferRenderer::~GeometryBufferRenderer()
{
}

void GeometryBufferRenderer::renderSubMesh(SubMesh* subMesh)
{
  auto mesh   = subMesh->getRenderingMesh();
  auto scene  = _scene;
  auto engine = scene->getEngine();

  // Culling
  engine->setState(subMesh->getMaterial()->backFaceCulling());

  // Managing instances
  auto batch = mesh->_getInstancesRenderList(subMesh->_id);

  if (batch->mustReturn) {
    return;
  }

  auto hardwareInstancedRendering
    = (engine->getCaps().instancedArrays != 0)
      && (stl_util::contains(batch->visibleInstances, subMesh->_id))
      && (!batch->visibleInstances[subMesh->_id].empty());

  if (isReady(subMesh, hardwareInstancedRendering)) {
    engine->enableEffect(_effect);
    mesh->_bind(subMesh, _effect, Material::TriangleFillMode);
    auto material = subMesh->getMaterial();

    _effect->setMatrix("viewProjection", scene->getTransformMatrix());
    _effect->setMatrix("view", scene->getViewMatrix());

    // Alpha test
    if (material && material->needAlphaTesting()) {
      auto alphaTexture = material->getAlphaTestTexture();
      _effect->setTexture("diffuseSampler", alphaTexture);
      _effect->setMatrix("diffuseMatrix", *alphaTexture->getTextureMatrix());
    }

    // Bones
    if (mesh->useBones() && mesh->computeBonesUsingShaders()) {
      _effect->setMatrices("mBones",
                           mesh->skeleton()->getTransformMatrices(mesh));
    }

    // Draw
    mesh->_processRendering(subMesh, _effect, Material::TriangleFillMode, batch,
                            hardwareInstancedRendering,
                            [this](bool /*isInstance*/, Matrix world,
                                   Material* /*effectiveMaterial*/) {
                              _effect->setMatrix("world", world);
                            });
  }
}

void GeometryBufferRenderer::setRenderList(const std::vector<Mesh*>& meshes)
{
  _multiRenderTarget->renderList.clear();
  _multiRenderTarget->renderList.reserve(meshes.size());
  for (auto& mesh : meshes) {
    _multiRenderTarget->renderList.emplace_back(mesh);
  }
}

bool GeometryBufferRenderer::isSupported() const
{
  return _multiRenderTarget->isSupported();
}

bool GeometryBufferRenderer::isReady(SubMesh* subMesh, bool useInstances)
{
  auto material = subMesh->getMaterial();
  if (material && material->disableDepthWrite) {
    return false;
  }

  std::vector<std::string> defines;

  std::vector<std::string> attribs{VertexBuffer::PositionKindChars,
                                   VertexBuffer::NormalKindChars};

  auto mesh = subMesh->getMesh();

  // Alpha test
  if (material && material->needAlphaTesting()) {
    defines.emplace_back("#define ALPHATEST");
    if (mesh->isVerticesDataPresent(VertexBuffer::UVKind)) {
      attribs.emplace_back(VertexBuffer::UVKindChars);
      defines.emplace_back("#define UV1");
    }
    if (mesh->isVerticesDataPresent(VertexBuffer::UV2Kind)) {
      attribs.emplace_back(VertexBuffer::UV2KindChars);
      defines.emplace_back("#define UV2");
    }
  }

  // Bones
  if (mesh->useBones() && mesh->computeBonesUsingShaders()) {
    attribs.emplace_back(VertexBuffer::MatricesIndicesKindChars);
    attribs.emplace_back(VertexBuffer::MatricesWeightsKindChars);
    if (mesh->numBoneInfluencers() > 4) {
      attribs.emplace_back(VertexBuffer::MatricesIndicesExtraKindChars);
      attribs.emplace_back(VertexBuffer::MatricesWeightsExtraKindChars);
    }
    defines.emplace_back("#define NUM_BONE_INFLUENCERS "
                         + std::to_string(mesh->numBoneInfluencers()));
    defines.emplace_back("#define BonesPerMesh "
                         + std::to_string(mesh->skeleton()->bones.size() + 1));
  }
  else {
    defines.emplace_back("#define NUM_BONE_INFLUENCERS 0");
  }

  // Instances
  if (useInstances) {
    defines.emplace_back("#define INSTANCES");
    attribs.emplace_back(VertexBuffer::World0KindChars);
    attribs.emplace_back(VertexBuffer::World1KindChars);
    attribs.emplace_back(VertexBuffer::World2KindChars);
    attribs.emplace_back(VertexBuffer::World3KindChars);
  }

  // Get correct effect
  std::string join = String::join(defines, '\n');
  if (_cachedDefines != join) {
    _cachedDefines = join;

    EffectCreationOptions options;
    options.attributes = std::move(attribs);
    options.uniformsNames
      = {"world", "mBones", "viewProjection", "diffuseMatrix", "view"};
    options.samplers = {"diffuseSampler"};
    options.defines  = std::move(join);

    _effect = _scene->getEngine()->createEffect("geometry", options,
                                                _scene->getEngine());
  }

  return _effect->isReady();
}

MultiRenderTarget* GeometryBufferRenderer::getGBuffer()
{
  return _multiRenderTarget.get();
}

void GeometryBufferRenderer::dispose(bool doNotRecurse)
{
  getGBuffer()->dispose(doNotRecurse);
}

} // end of namespace BABYLON
