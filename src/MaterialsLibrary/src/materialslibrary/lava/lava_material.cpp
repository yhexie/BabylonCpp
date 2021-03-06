#include <babylon/materialslibrary/lava/lava_material.h>

#include <babylon/cameras/camera.h>
#include <babylon/core/json.h>
#include <babylon/core/time.h>
#include <babylon/engine/engine.h>
#include <babylon/engine/scene.h>
#include <babylon/materials/effect.h>
#include <babylon/materials/effect_creation_options.h>
#include <babylon/materials/effect_fallbacks.h>
#include <babylon/materials/material_helper.h>
#include <babylon/materials/standard_material.h>
#include <babylon/materials/textures/base_texture.h>
#include <babylon/mesh/abstract_mesh.h>
#include <babylon/mesh/mesh.h>
#include <babylon/mesh/sub_mesh.h>
#include <babylon/mesh/vertex_buffer.h>

namespace BABYLON {
namespace MaterialsLibrary {

LavaMaterial::LavaMaterial(const std::string& iName, Scene* scene)
    : PushMaterial{iName, scene}
    , noiseTexture{nullptr}
    , fogColor{nullptr}
    , speed{1.f}
    , movingSpeed{1.f}
    , lowFrequencySpeed{1.f}
    , fogDensity{0.15f}
    , diffuseColor{Color3(1.f, 1.f, 1.f)}
    , _diffuseTexture{nullptr}
    , _lastTime{std::chrono::milliseconds(0)}
    , _disableLighting{false}
    , _maxSimultaneousLights{4}
    , _worldViewProjectionMatrix{Matrix::Zero()}
    , _renderId{-1}
{
}

LavaMaterial::~LavaMaterial()
{
}

bool LavaMaterial::needAlphaBlending()
{
  return (alpha < 1.f);
}

bool LavaMaterial::needAlphaTesting()
{
  return false;
}

BaseTexture* LavaMaterial::getAlphaTestTexture()
{
  return nullptr;
}

bool LavaMaterial::isReadyForSubMesh(AbstractMesh* mesh, SubMesh* subMesh,
                                     bool useInstances)
{
  if (isFrozen()) {
    if (_wasPreviouslyReady && subMesh->effect()) {
      return true;
    }
  }

  if (!subMesh->_materialDefines) {
    subMesh->_materialDefines = std::make_unique<LavaMaterialDefines>();
  }

  auto defines
    = *(static_cast<LavaMaterialDefines*>(subMesh->_materialDefines.get()));
  auto scene = getScene();

  if (!checkReadyOnEveryCall && subMesh->effect()) {
    if (_renderId == scene->getRenderId()) {
      return true;
    }
  }

  auto engine = scene->getEngine();

  // Textures
  if (defines._areTexturesDirty) {
    defines._needUVs = false;
    if (scene->texturesEnabled()) {
      if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
        if (!_diffuseTexture->isReady()) {
          return false;
        }
        else {
          defines._needUVs              = true;
          defines.defines[LMD::DIFFUSE] = true;
        }
      }
    }
  }

  // Misc.
  MaterialHelper::PrepareDefinesForMisc(
    mesh, scene, false, pointsCloud(), fogEnabled(), defines,
    LMD::LOGARITHMICDEPTH, LMD::POINTSIZE, LMD::FOG);

  // Lights
  defines._needNormals = MaterialHelper::PrepareDefinesForLights(
    scene, mesh, defines, false, _maxSimultaneousLights, _disableLighting,
    LMD::SPECULARTERM, LMD::SHADOWFULLFLOAT);

  // Values that need to be evaluated on every frame
  MaterialHelper::PrepareDefinesForFrameBoundValues(
    scene, engine, defines, useInstances, LMD::CLIPPLANE, LMD::ALPHATEST,
    LMD::INSTANCES);

  // Attribs
  MaterialHelper::PrepareDefinesForAttributes(
    mesh, defines, true, true, false, LMD::NORMAL, LMD::UV1, LMD::UV2,
    LMD::VERTEXCOLOR, LMD::VERTEXALPHA);

  // Get correct effect
  if (defines.isDirty()) {
    defines.markAsProcessed();
    scene->resetCachedMaterial();

    // Fallbacks
    auto fallbacks = std::make_unique<EffectFallbacks>();
    if (defines[LMD::FOG]) {
      fallbacks->addFallback(1, "FOG");
    }

    MaterialHelper::HandleFallbacksForShadows(defines, *fallbacks,
                                              _maxSimultaneousLights);

    if (defines.NUM_BONE_INFLUENCERS > 0) {
      fallbacks->addCPUSkinningFallback(0, mesh);
    }

    // Attributes
    std::vector<std::string> attribs = {VertexBuffer::PositionKindChars};

    if (defines[LMD::NORMAL]) {
      attribs.emplace_back(VertexBuffer::NormalKindChars);
    }

    if (defines[LMD::UV1]) {
      attribs.emplace_back(VertexBuffer::UVKindChars);
    }

    if (defines[LMD::UV2]) {
      attribs.emplace_back(VertexBuffer::UV2KindChars);
    }

    if (defines[LMD::VERTEXCOLOR]) {
      attribs.emplace_back(VertexBuffer::ColorKindChars);
    }

    MaterialHelper::PrepareAttributesForBones(attribs, mesh, defines,
                                              *fallbacks);
    MaterialHelper::PrepareAttributesForInstances(attribs, defines,
                                                  LMD::INSTANCES);

    // Legacy browser patch
    const std::string shaderName{"lava"};
    auto join = defines.toString();

    const std::vector<std::string> uniforms{
      "world",         "view",          "viewProjection", "vEyePosition",
      "vLightsType",   "vDiffuseColor", "vFogInfos",      "vFogColor",
      "pointSize",     "vDiffuseInfos", "mBones",         "vClipPlane",
      "diffuseMatrix", "depthValues",   "time",           "speed",
      "movingSpeed",   "fogColor",      "fogDensity",     "lowFrequencySpeed"};

    const std::vector<std::string> samplers{"diffuseSampler", "noiseTexture"};
    const std::vector<std::string> uniformBuffers{};

    EffectCreationOptions options;
    options.attributes            = std::move(attribs);
    options.uniformsNames         = std::move(uniforms);
    options.uniformBuffersNames   = std::move(uniformBuffers);
    options.samplers              = std::move(samplers);
    options.materialDefines       = &defines;
    options.defines               = std::move(join);
    options.maxSimultaneousLights = _maxSimultaneousLights;
    options.fallbacks             = std::move(fallbacks);
    options.onCompiled            = onCompiled;
    options.onError               = onError;
    options.indexParameters
      = {{"maxSimultaneousLights", _maxSimultaneousLights}};

    MaterialHelper::PrepareUniformsAndSamplersList(options);
    subMesh->setEffect(
      scene->getEngine()->createEffect(shaderName, options, engine), defines);
  }

  if (!subMesh->effect()->isReady()) {
    return false;
  }

  _renderId           = scene->getRenderId();
  _wasPreviouslyReady = true;

  return true;
}

void LavaMaterial::bindForSubMesh(Matrix* world, Mesh* mesh, SubMesh* subMesh)
{
  auto scene = getScene();

  auto defines
    = static_cast<LavaMaterialDefines*>(subMesh->_materialDefines.get());
  if (!defines) {
    return;
  }

  auto effect   = subMesh->effect();
  _activeEffect = effect;

  // Matrices
  bindOnlyWorldMatrix(*world);
  _activeEffect->setMatrix("viewProjection", scene->getTransformMatrix());

  // Bones
  MaterialHelper::BindBonesParameters(mesh, _activeEffect);

  if (_mustRebind(scene, effect)) {
    // Textures
    if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
      _activeEffect->setTexture("diffuseSampler", _diffuseTexture);

      _activeEffect->setFloat2("vDiffuseInfos",
                               _diffuseTexture->coordinatesIndex,
                               _diffuseTexture->level);
      _activeEffect->setMatrix("diffuseMatrix",
                               *_diffuseTexture->getTextureMatrix());
    }

    if (noiseTexture) {
      _activeEffect->setTexture("noiseTexture", noiseTexture);
    }

    // Clip plane
    MaterialHelper::BindClipPlane(_activeEffect, scene);

    // Point size
    if (pointsCloud()) {
      _activeEffect->setFloat("pointSize", pointSize);
    }

    _activeEffect->setVector3("vEyePosition",
                              scene->_mirroredCameraPosition ?
                                *scene->_mirroredCameraPosition :
                                scene->activeCamera->position);
  }

  _activeEffect->setColor4("vDiffuseColor", _scaledDiffuse,
                           alpha * mesh->visibility);

  if (scene->lightsEnabled() && !_disableLighting) {
    MaterialHelper::BindLights(scene, mesh, _activeEffect, *defines,
                               _maxSimultaneousLights, LMD::SPECULARTERM);
  }

  // View
  if (scene->fogEnabled() && mesh->applyFog()
      && scene->fogMode() != Scene::FOGMODE_NONE) {
    _activeEffect->setMatrix("view", scene->getViewMatrix());
  }

  // Fog
  MaterialHelper::BindFogParameters(scene, mesh, _activeEffect);

  _lastTime += scene->getEngine()->getDeltaTime();
  _effect->setFloat("time", Time::fpMillisecondsDuration<float>(_lastTime)
                              * speed / 1000.f);

  if (!fogColor) {
    fogColor = std::make_unique<Color3>(Color3::Black());
  }
  _activeEffect->setColor3("fogColor", *fogColor);
  _activeEffect->setFloat("fogDensity", fogDensity);

  _activeEffect->setFloat("lowFrequencySpeed", lowFrequencySpeed);
  _activeEffect->setFloat("movingSpeed", movingSpeed);

  _afterBind(mesh, _activeEffect);
}

std::vector<IAnimatable*> LavaMaterial::getAnimatables()
{
  std::vector<IAnimatable*> results;

  if (_diffuseTexture && _diffuseTexture->animations.size() > 0) {
    results.emplace_back(_diffuseTexture);
  }

  if (noiseTexture && noiseTexture->animations.size() > 0) {
    results.emplace_back(noiseTexture);
  }

  return results;
}

void LavaMaterial::dispose(bool forceDisposeEffect, bool forceDisposeTextures)
{
  if (_diffuseTexture) {
    _diffuseTexture->dispose();
  }

  if (noiseTexture) {
    noiseTexture->dispose();
  }

  Material::dispose(forceDisposeEffect, forceDisposeTextures);
}

Material* LavaMaterial::clone(const std::string& /*name*/,
                              bool /*cloneChildren*/) const
{
  return nullptr;
}

Json::object LavaMaterial::serialize() const
{
  return Json::object();
}

LavaMaterial* LavaMaterial::Parse(const Json::value& /*source*/,
                                  Scene* /*scene*/,
                                  const std::string& /*rootUrl*/)
{
  return nullptr;
}

} // end of namespace MaterialsLibrary
} // end of namespace BABYLON
