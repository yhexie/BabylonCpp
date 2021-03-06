#include <babylon/materials/standard_material.h>

#include <babylon/animations/animation.h>
#include <babylon/babylon_stl_util.h>
#include <babylon/bones/skeleton.h>
#include <babylon/cameras/camera.h>
#include <babylon/engine/engine.h>
#include <babylon/engine/scene.h>
#include <babylon/lights/directional_light.h>
#include <babylon/lights/hemispheric_light.h>
#include <babylon/lights/light.h>
#include <babylon/lights/point_light.h>
#include <babylon/lights/shadows/shadow_generator.h>
#include <babylon/lights/spot_light.h>
#include <babylon/materials/color_curves.h>
#include <babylon/materials/effect.h>
#include <babylon/materials/effect_creation_options.h>
#include <babylon/materials/effect_fallbacks.h>
#include <babylon/materials/fresnel_parameters.h>
#include <babylon/materials/material_helper.h>
#include <babylon/materials/standard_material_defines.h>
#include <babylon/materials/textures/base_texture.h>
#include <babylon/materials/textures/color_grading_texture.h>
#include <babylon/materials/textures/refraction_texture.h>
#include <babylon/materials/textures/render_target_texture.h>
#include <babylon/materials/uniform_buffer.h>
#include <babylon/mesh/abstract_mesh.h>
#include <babylon/mesh/mesh.h>
#include <babylon/mesh/sub_mesh.h>
#include <babylon/mesh/vertex_buffer.h>
#include <babylon/tools/serialization_helper.h>

namespace BABYLON {

bool StandardMaterial::_DiffuseTextureEnabled      = true;
bool StandardMaterial::_AmbientTextureEnabled      = true;
bool StandardMaterial::_OpacityTextureEnabled      = true;
bool StandardMaterial::_ReflectionTextureEnabled   = true;
bool StandardMaterial::_EmissiveTextureEnabled     = true;
bool StandardMaterial::_SpecularTextureEnabled     = true;
bool StandardMaterial::_BumpTextureEnabled         = true;
bool StandardMaterial::_FresnelEnabled             = true;
bool StandardMaterial::_LightmapTextureEnabled     = true;
bool StandardMaterial::_RefractionTextureEnabled   = true;
bool StandardMaterial::_ColorGradingTextureEnabled = true;

StandardMaterial::StandardMaterial(const std::string& iName, Scene* scene)
    : PushMaterial{iName, scene}
    , ambientColor{Color3(0.f, 0.f, 0.f)}
    , diffuseColor{Color3(1.f, 1.f, 1.f)}
    , specularColor{Color3(1.f, 1.f, 1.f)}
    , emissiveColor{Color3(0.f, 0.f, 0.f)}
    , specularPower{64.f}
    , parallaxScaleBias{0.05f}
    , indexOfRefraction{0.98f}
    , invertRefractionY{true}
    , customShaderNameResolve{nullptr}
    , _worldViewProjectionMatrix{Matrix::Zero()}
    , _globalAmbientColor{Color3(0.f, 0.f, 0.f)}
    , _useLogarithmicDepth{false}
    , _diffuseTexture{nullptr}
    , _ambientTexture{nullptr}
    , _opacityTexture{nullptr}
    , _reflectionTexture{nullptr}
    , _emissiveTexture{nullptr}
    , _specularTexture{nullptr}
    , _bumpTexture{nullptr}
    , _lightmapTexture{nullptr}
    , _refractionTexture{nullptr}
    , _useAlphaFromDiffuseTexture{false}
    , _useEmissiveAsIllumination{false}
    , _linkEmissiveWithDiffuse{false}
    , _useReflectionFresnelFromSpecular{false}
    , _useSpecularOverAlpha{false}
    , _useReflectionOverAlpha{false}
    , _disableLighting{false}
    , _useParallax{false}
    , _useParallaxOcclusion{false}
    , _roughness{0.f}
    , _useLightmapAsShadowmap{false}
    , _diffuseFresnelParameters{nullptr}
    , _opacityFresnelParameters{nullptr}
    , _reflectionFresnelParameters{nullptr}
    , _refractionFresnelParameters{nullptr}
    , _emissiveFresnelParameters{nullptr}
    , _useGlossinessFromSpecularMapAlpha{false}
    , _maxSimultaneousLights{4}
    , _invertNormalMapX{false}
    , _invertNormalMapY{false}
    , _twoSidedLighting{false}
    , _cameraColorGradingTexture{nullptr}
    , _cameraColorCurves{nullptr}
{
  getRenderTargetTextures = [this]() {
    _renderTargets.clear();

    if (StandardMaterial::ReflectionTextureEnabled()
        && _reflectionTexture->isRenderTarget) {
      _renderTargets.emplace_back(_reflectionTexture);
    }

    if (StandardMaterial::RefractionTextureEnabled()
        && _refractionTexture->isRenderTarget) {
      _renderTargets.emplace_back(_refractionTexture);
    }

    return _renderTargets;
  };
}

StandardMaterial::StandardMaterial(const StandardMaterial& other)
    : PushMaterial{other.name, other.getScene()}
{
  // Base material
  other.copyTo(dynamic_cast<PushMaterial*>(this));

  // Standard material
  ambientColor            = other.ambientColor;
  diffuseColor            = other.diffuseColor;
  specularColor           = other.specularColor;
  emissiveColor           = other.emissiveColor;
  specularPower           = other.specularPower;
  parallaxScaleBias       = other.parallaxScaleBias;
  indexOfRefraction       = other.indexOfRefraction;
  invertRefractionY       = other.invertRefractionY;
  customShaderNameResolve = other.customShaderNameResolve;

  _renderTargets             = other._renderTargets;
  _worldViewProjectionMatrix = other._worldViewProjectionMatrix;
  _globalAmbientColor        = other._globalAmbientColor;
  _useLogarithmicDepth       = other._useLogarithmicDepth;

  _diffuseTexture                   = other._diffuseTexture;
  _ambientTexture                   = other._ambientTexture;
  _opacityTexture                   = other._ambientTexture;
  _reflectionTexture                = other._reflectionTexture;
  _emissiveTexture                  = other._emissiveTexture;
  _specularTexture                  = other._specularTexture;
  _bumpTexture                      = other._bumpTexture;
  _lightmapTexture                  = other._lightmapTexture;
  _refractionTexture                = other._refractionTexture;
  _useAlphaFromDiffuseTexture       = other._useAlphaFromDiffuseTexture;
  _useEmissiveAsIllumination        = other._useEmissiveAsIllumination;
  _linkEmissiveWithDiffuse          = other._linkEmissiveWithDiffuse;
  _useReflectionFresnelFromSpecular = other._useReflectionFresnelFromSpecular;
  _useSpecularOverAlpha             = other._useSpecularOverAlpha;
  _useReflectionOverAlpha           = other._useReflectionOverAlpha;
  _disableLighting                  = other._disableLighting;
  _useParallax                      = other._useParallax;
  _useParallaxOcclusion             = other._useParallaxOcclusion;
  _roughness                        = other._roughness;
  _useLightmapAsShadowmap           = other._useLightmapAsShadowmap;

  if (other._diffuseFresnelParameters) {
    _diffuseFresnelParameters = other._diffuseFresnelParameters->clone();
  }
  if (other._opacityFresnelParameters) {
    _opacityFresnelParameters = other._opacityFresnelParameters->clone();
  }
  if (other._reflectionFresnelParameters) {
    _reflectionFresnelParameters = other._reflectionFresnelParameters->clone();
  }
  if (other._refractionFresnelParameters) {
    _refractionFresnelParameters = other._refractionFresnelParameters->clone();
  }
  if (other._emissiveFresnelParameters) {
    _emissiveFresnelParameters = other._emissiveFresnelParameters->clone();
  }

  _useGlossinessFromSpecularMapAlpha = other._useGlossinessFromSpecularMapAlpha;
  _maxSimultaneousLights             = other._maxSimultaneousLights;
  _invertNormalMapX                  = other._invertNormalMapX;
  _invertNormalMapY                  = other._invertNormalMapY;
  _twoSidedLighting                  = other._twoSidedLighting;
  _cameraColorGradingTexture         = other._cameraColorGradingTexture;
  _cameraColorCurves                 = other._cameraColorCurves;
}

StandardMaterial::~StandardMaterial()
{
}

const char* StandardMaterial::getClassName() const
{
  return "StandardMaterial";
}

IReflect::Type StandardMaterial::type() const
{
  return IReflect::Type::STANDARDMATERIAL;
}

void StandardMaterial::setAmbientColor(const Color3& color)
{
  ambientColor = color;
}

void StandardMaterial::setDiffuseColor(const Color3& color)
{
  diffuseColor = color;
}

void StandardMaterial::setSpecularColor(const Color3& color)
{
  specularColor = color;
}
void StandardMaterial::setEmissiveColor(const Color3& color)
{
  emissiveColor = color;
}

bool StandardMaterial::useLogarithmicDepth() const
{
  return _useLogarithmicDepth;
}

void StandardMaterial::setUseLogarithmicDepth(bool value)
{
  _useLogarithmicDepth
    = value && getScene()->getEngine()->getCaps().fragmentDepthSupported;
  _markAllSubMeshesAsMiscDirty();
}

bool StandardMaterial::needAlphaBlending()
{
  return (alpha < 1.f) || (_opacityTexture != nullptr)
         || _shouldUseAlphaFromDiffuseTexture()
         || (_opacityFresnelParameters
             && _opacityFresnelParameters->isEnabled());
}

bool StandardMaterial::needAlphaTesting()
{
  return _diffuseTexture != nullptr && _diffuseTexture->hasAlpha();
}

bool StandardMaterial::_shouldUseAlphaFromDiffuseTexture()
{
  return _diffuseTexture != nullptr && _diffuseTexture->hasAlpha()
         && _useAlphaFromDiffuseTexture;
}

BaseTexture* StandardMaterial::getAlphaTestTexture()
{
  return _diffuseTexture;
}

bool StandardMaterial::isReadyForSubMesh(AbstractMesh* mesh, SubMesh* subMesh,
                                         bool useInstances)
{
  if (isFrozen()) {
    if (_wasPreviouslyReady && subMesh->effect()) {
      return true;
    }
  }

  if (!subMesh->_materialDefines) {
    subMesh->_materialDefines = std::make_unique<StandardMaterialDefines>();
  }

  auto scene = getScene();
  auto defines
    = *(static_cast<StandardMaterialDefines*>(subMesh->_materialDefines.get()));
  if (!checkReadyOnEveryCall && subMesh->effect()) {
    if (defines._renderId == scene->getRenderId()) {
      return true;
    }
  }

  auto engine = scene->getEngine();

  // Lights
  defines._needNormals = MaterialHelper::PrepareDefinesForLights(
    scene, mesh, defines, true, _maxSimultaneousLights, _disableLighting,
    SMD::SPECULARTERM, SMD::SHADOWFLOAT);

  // Textures
  if (defines._areTexturesDirty) {
    defines._needUVs = false;
    if (scene->texturesEnabled()) {
      if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
        if (!_diffuseTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs              = true;
          defines.defines[SMD::DIFFUSE] = true;
        }
      }
      else {
        defines.defines[SMD::DIFFUSE] = false;
      }

      if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
        if (!_ambientTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs              = true;
          defines.defines[SMD::AMBIENT] = true;
        }
      }
      else {
        defines.defines[SMD::AMBIENT] = false;
      }

      if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
        if (!_opacityTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs                 = true;
          defines.defines[SMD::OPACITY]    = true;
          defines.defines[SMD::OPACITYRGB] = _opacityTexture->getAlphaFromRGB;
        }
      }
      else {
        defines.defines[SMD::OPACITY] = false;
      }

      if (_reflectionTexture && StandardMaterial::ReflectionTextureEnabled()) {
        if (!_reflectionTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needNormals             = true;
          defines.defines[SMD::REFLECTION] = true;

          defines.defines[SMD::ROUGHNESS]           = (_roughness > 0);
          defines.defines[SMD::REFLECTIONOVERALPHA] = _useReflectionOverAlpha;
          defines.defines[SMD::INVERTCUBICMAP]
            = (_reflectionTexture->coordinatesMode
               == TextureConstants::INVCUBIC_MODE);
          defines.defines[SMD::REFLECTIONMAP_3D] = _reflectionTexture->isCube;

          switch (_reflectionTexture->coordinatesMode) {
            case TextureConstants::CUBIC_MODE:
            case TextureConstants::INVCUBIC_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_CUBIC);
              break;
            case TextureConstants::EXPLICIT_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_EXPLICIT);
              break;
            case TextureConstants::PLANAR_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_PLANAR);
              break;
            case TextureConstants::PROJECTION_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_PROJECTION);
              break;
            case TextureConstants::SKYBOX_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_SKYBOX);
              break;
            case TextureConstants::SPHERICAL_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_SPHERICAL);
              break;
            case TextureConstants::EQUIRECTANGULAR_MODE:
              defines.setReflectionMode(SMD::REFLECTIONMAP_EQUIRECTANGULAR);
              break;
            case TextureConstants::FIXED_EQUIRECTANGULAR_MODE:
              defines.setReflectionMode(
                SMD::REFLECTIONMAP_EQUIRECTANGULAR_FIXED);
              break;
            case TextureConstants::FIXED_EQUIRECTANGULAR_MIRRORED_MODE:
              defines.setReflectionMode(
                SMD::REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED);
              break;
          }
        }
      }
      else {
        defines.defines[SMD::REFLECTION] = false;
      }

      if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
        if (!_emissiveTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs               = true;
          defines.defines[SMD::EMISSIVE] = true;
        }
      }
      else {
        defines.defines[SMD::EMISSIVE] = false;
      }

      if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
        if (!_lightmapTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs               = true;
          defines.defines[SMD::LIGHTMAP] = true;
          defines.defines[SMD::USELIGHTMAPASSHADOWMAP]
            = _useLightmapAsShadowmap;
        }
      }
      else {
        defines.defines[SMD::LIGHTMAP] = false;
      }

      if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
        if (!_specularTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs                 = true;
          defines.defines[SMD::SPECULAR]   = true;
          defines.defines[SMD::GLOSSINESS] = _useGlossinessFromSpecularMapAlpha;
        }
      }
      else {
        defines.defines[SMD::SPECULAR] = false;
      }

      if (scene->getEngine()->getCaps().standardDerivatives && _bumpTexture
          && StandardMaterial::BumpTextureEnabled()) {
        // Bump texure can not be none blocking.
        if (!_bumpTexture->isReady()) {
          return false;
        }
        else {
          defines._needUVs           = true;
          defines.defines[SMD::BUMP] = true;

          defines.defines[SMD::INVERTNORMALMAPX] = _invertNormalMapX;
          defines.defines[SMD::INVERTNORMALMAPY] = _invertNormalMapY;

          defines.defines[SMD::PARALLAX]          = _useParallax;
          defines.defines[SMD::PARALLAXOCCLUSION] = _useParallaxOcclusion;
        }
      }
      else {
        defines.defines[SMD::BUMP] = false;
      }

      if (_refractionTexture && StandardMaterial::RefractionTextureEnabled()) {
        if (!_refractionTexture->isReadyOrNotBlocking()) {
          return false;
        }
        else {
          defines._needUVs                 = true;
          defines.defines[SMD::REFRACTION] = true;

          defines.defines[SMD::REFRACTIONMAP_3D] = _refractionTexture->isCube;
        }
      }
      else {
        defines.defines[SMD::REFRACTION] = false;
      }

      if (_cameraColorGradingTexture
          && StandardMaterial::ColorGradingTextureEnabled()) {
        // Camera Color Grading can not be none blocking.
        if (!_cameraColorGradingTexture->isReady()) {
          return false;
        }
        else {
          defines.defines[SMD::CAMERACOLORGRADING] = true;
        }
      }
      else {
        defines.defines[SMD::CAMERACOLORGRADING] = false;
      }

      defines.defines[SMD::TWOSIDEDLIGHTING]
        = !_backFaceCulling && _twoSidedLighting;
    }
    else {
      defines.defines[SMD::DIFFUSE]            = false;
      defines.defines[SMD::AMBIENT]            = false;
      defines.defines[SMD::OPACITY]            = false;
      defines.defines[SMD::REFLECTION]         = false;
      defines.defines[SMD::EMISSIVE]           = false;
      defines.defines[SMD::LIGHTMAP]           = false;
      defines.defines[SMD::BUMP]               = false;
      defines.defines[SMD::REFRACTION]         = false;
      defines.defines[SMD::CAMERACOLORGRADING] = false;
    }

    defines.defines[SMD::CAMERACOLORCURVES]
      = (_cameraColorCurves != nullptr && _cameraColorCurves != nullptr);

    defines.defines[SMD::ALPHAFROMDIFFUSE]
      = _shouldUseAlphaFromDiffuseTexture();

    defines.defines[SMD::EMISSIVEASILLUMINATION] = _useEmissiveAsIllumination;

    defines.defines[SMD::LINKEMISSIVEWITHDIFFUSE] = _linkEmissiveWithDiffuse;

    defines.defines[SMD::SPECULAROVERALPHA] = _useSpecularOverAlpha;
  }

  if (defines._areFresnelDirty) {
    if (StandardMaterial::FresnelEnabled()) {
      // Fresnel
      if ((_diffuseFresnelParameters && _diffuseFresnelParameters->isEnabled())
          || (_opacityFresnelParameters
              && _opacityFresnelParameters->isEnabled())
          || (_emissiveFresnelParameters
              && _emissiveFresnelParameters->isEnabled())
          || (_refractionFresnelParameters
              && _refractionFresnelParameters->isEnabled())
          || (_reflectionFresnelParameters
              && _reflectionFresnelParameters->isEnabled())) {

        defines.defines[SMD::DIFFUSEFRESNEL]
          = (_diffuseFresnelParameters
             && _diffuseFresnelParameters->isEnabled());

        defines.defines[SMD::OPACITYFRESNEL]
          = (_opacityFresnelParameters
             && _opacityFresnelParameters->isEnabled());

        defines.defines[SMD::REFLECTIONFRESNEL]
          = (_reflectionFresnelParameters
             && _reflectionFresnelParameters->isEnabled());

        defines.defines[SMD::REFLECTIONFRESNELFROMSPECULAR]
          = _useReflectionFresnelFromSpecular;

        defines.defines[SMD::REFRACTIONFRESNEL]
          = (_refractionFresnelParameters
             && _refractionFresnelParameters->isEnabled());

        defines.defines[SMD::EMISSIVEFRESNEL]
          = (_emissiveFresnelParameters
             && _emissiveFresnelParameters->isEnabled());

        defines._needNormals          = true;
        defines.defines[SMD::FRESNEL] = true;
      }
    }
    else {
      defines.defines[SMD::FRESNEL] = false;
    }
  }

  // Misc.
  MaterialHelper::PrepareDefinesForMisc(
    mesh, scene, _useLogarithmicDepth, pointsCloud(), fogEnabled(), defines,
    SMD::LOGARITHMICDEPTH, SMD::POINTSIZE, SMD::FOG);

  // Attribs
  MaterialHelper::PrepareDefinesForAttributes(
    mesh, defines, true, true, true, SMD::NORMAL, SMD::UV1, SMD::UV2,
    SMD::VERTEXCOLOR, SMD::VERTEXALPHA, SMD::MORPHTARGETS_NORMAL,
    SMD::MORPHTARGETS);

  // Values that need to be evaluated on every frame
  MaterialHelper::PrepareDefinesForFrameBoundValues(
    scene, engine, defines, useInstances, SMD::CLIPPLANE, SMD::ALPHATEST,
    SMD::INSTANCES);

  if (scene->_mirroredCameraPosition && defines[SMD::BUMP]) {
    defines.defines[SMD::INVERTNORMALMAPX] = !_invertNormalMapX;
    defines.defines[SMD::INVERTNORMALMAPY] = !_invertNormalMapY;
    defines.markAsUnprocessed();
  }

  // Get correct effect
  if (defines.isDirty()) {
    defines.markAsProcessed();
    scene->resetCachedMaterial();

    // Fallbacks
    auto fallbacks = std::make_unique<EffectFallbacks>();
    if (defines[SMD::REFLECTION]) {
      fallbacks->addFallback(0, "REFLECTION");
    }

    if (defines[SMD::SPECULAR]) {
      fallbacks->addFallback(0, "SPECULAR");
    }

    if (defines[SMD::BUMP]) {
      fallbacks->addFallback(0, "BUMP");
    }

    if (defines[SMD::PARALLAX]) {
      fallbacks->addFallback(1, "PARALLAX");
    }

    if (defines[SMD::PARALLAXOCCLUSION]) {
      fallbacks->addFallback(0, "PARALLAXOCCLUSION");
    }

    if (defines[SMD::SPECULAROVERALPHA]) {
      fallbacks->addFallback(0, "SPECULAROVERALPHA");
    }

    if (defines[SMD::FOG]) {
      fallbacks->addFallback(1, "FOG");
    }

    if (defines[SMD::POINTSIZE]) {
      fallbacks->addFallback(0, "POINTSIZE");
    }

    if (defines[SMD::LOGARITHMICDEPTH]) {
      fallbacks->addFallback(0, "LOGARITHMICDEPTH");
    }

    MaterialHelper::HandleFallbacksForShadows(defines, *fallbacks,
                                              _maxSimultaneousLights);

    if (defines[SMD::SPECULARTERM]) {
      fallbacks->addFallback(0, "SPECULARTERM");
    }

    if (defines[SMD::DIFFUSEFRESNEL]) {
      fallbacks->addFallback(1, "DIFFUSEFRESNEL");
    }

    if (defines[SMD::OPACITYFRESNEL]) {
      fallbacks->addFallback(2, "OPACITYFRESNEL");
    }

    if (defines[SMD::REFLECTIONFRESNEL]) {
      fallbacks->addFallback(3, "REFLECTIONFRESNEL");
    }

    if (defines[SMD::EMISSIVEFRESNEL]) {
      fallbacks->addFallback(4, "EMISSIVEFRESNEL");
    }

    if (defines[SMD::FRESNEL]) {
      fallbacks->addFallback(4, "FRESNEL");
    }

    // Attributes
    std::vector<std::string> attribs{VertexBuffer::PositionKindChars};

    if (defines[SMD::NORMAL]) {
      attribs.emplace_back(VertexBuffer::NormalKindChars);
    }

    if (defines[SMD::UV1]) {
      attribs.emplace_back(VertexBuffer::UVKindChars);
    }

    if (defines[SMD::UV2]) {
      attribs.emplace_back(VertexBuffer::UV2KindChars);
    }

    if (defines[SMD::VERTEXCOLOR]) {
      attribs.emplace_back(VertexBuffer::ColorKindChars);
    }

    MaterialHelper::PrepareAttributesForBones(attribs, mesh, defines,
                                              *fallbacks);
    MaterialHelper::PrepareAttributesForInstances(attribs, defines,
                                                  SMD::INSTANCES);
    MaterialHelper::PrepareAttributesForMorphTargets(attribs, mesh, defines,
                                                     SMD::NORMAL);

    std::string shaderName{"default"};
    const auto join = defines.toString();
    std::vector<std::string> uniforms{"world",
                                      "view",
                                      "viewProjection",
                                      "vEyePosition",
                                      "vLightsType",
                                      "vAmbientColor",
                                      "vDiffuseColor",
                                      "vSpecularColor",
                                      "vEmissiveColor",
                                      "vFogInfos",
                                      "vFogColor",
                                      "pointSize",
                                      "vDiffuseInfos",
                                      "vAmbientInfos",
                                      "vOpacityInfos",
                                      "vReflectionInfos",
                                      "vEmissiveInfos",
                                      "vSpecularInfos",
                                      "vBumpInfos",
                                      "vLightmapInfos",
                                      "vRefractionInfos",
                                      "mBones",
                                      "vClipPlane",
                                      "diffuseMatrix",
                                      "ambientMatrix",
                                      "opacityMatrix",
                                      "reflectionMatrix",
                                      "emissiveMatrix",
                                      "specularMatrix",
                                      "bumpMatrix",
                                      "lightmapMatrix",
                                      "refractionMatrix",
                                      "depthValues",
                                      "diffuseLeftColor",
                                      "diffuseRightColor",
                                      "opacityParts",
                                      "reflectionLeftColor",
                                      "reflectionRightColor",
                                      "emissiveLeftColor",
                                      "emissiveRightColor",
                                      "refractionLeftColor",
                                      "refractionRightColor",
                                      "logarithmicDepthConstant"};

    std::vector<std::string> samplers{
      "diffuseSampler",        "ambientSampler",      "opacitySampler",
      "reflectionCubeSampler", "reflection2DSampler", "emissiveSampler",
      "specularSampler",       "bumpSampler",         "lightmapSampler",
      "refractionCubeSampler", "refraction2DSampler"};
    std::vector<std::string> uniformBuffers{"Material", "Scene"};

    if (defines[SMD::CAMERACOLORCURVES]) {
      ColorCurves::PrepareUniforms(uniforms);
    }
    if (defines[SMD::CAMERACOLORGRADING]) {
      ColorGradingTexture::PrepareUniformsAndSamplers(uniforms, samplers);
    }

    std::unordered_map<std::string, unsigned int> indexParameters{
      {"maxSimultaneousLights", _maxSimultaneousLights},
      {"maxSimultaneousMorphTargets", defines.NUM_MORPH_INFLUENCERS}};

    EffectCreationOptions options;
    options.attributes            = std::move(attribs);
    options.uniformsNames         = std::move(uniforms);
    options.uniformBuffersNames   = std::move(uniformBuffers);
    options.samplers              = std::move(samplers);
    options.materialDefines       = &defines;
    options.defines               = std::move(join);
    options.fallbacks             = std::move(fallbacks);
    options.onCompiled            = onCompiled;
    options.onError               = onError;
    options.indexParameters       = std::move(indexParameters);
    options.maxSimultaneousLights = _maxSimultaneousLights;

    MaterialHelper::PrepareUniformsAndSamplersList(options);

    if (customShaderNameResolve) {
      shaderName = customShaderNameResolve(shaderName, uniforms, uniformBuffers,
                                           samplers, defines);
    }

    subMesh->setEffect(
      scene->getEngine()->createEffect(shaderName, options, engine), defines);

    buildUniformLayout();
  }

  if (!subMesh->effect()->isReady()) {
    return false;
  }

  defines._renderId   = scene->getRenderId();
  _wasPreviouslyReady = true;

  return true;
}

void StandardMaterial::buildUniformLayout()
{
  // Order is important !
  _uniformBuffer->addUniform("diffuseLeftColor", 4);
  _uniformBuffer->addUniform("diffuseRightColor", 4);
  _uniformBuffer->addUniform("opacityParts", 4);
  _uniformBuffer->addUniform("reflectionLeftColor", 4);
  _uniformBuffer->addUniform("reflectionRightColor", 4);
  _uniformBuffer->addUniform("refractionLeftColor", 4);
  _uniformBuffer->addUniform("refractionRightColor", 4);
  _uniformBuffer->addUniform("emissiveLeftColor", 4);
  _uniformBuffer->addUniform("emissiveRightColor", 4);

  _uniformBuffer->addUniform("vDiffuseInfos", 2);
  _uniformBuffer->addUniform("vAmbientInfos", 2);
  _uniformBuffer->addUniform("vOpacityInfos", 2);
  _uniformBuffer->addUniform("vReflectionInfos", 2);
  _uniformBuffer->addUniform("vEmissiveInfos", 2);
  _uniformBuffer->addUniform("vLightmapInfos", 2);
  _uniformBuffer->addUniform("vSpecularInfos", 2);
  _uniformBuffer->addUniform("vBumpInfos", 3);

  _uniformBuffer->addUniform("diffuseMatrix", 16);
  _uniformBuffer->addUniform("ambientMatrix", 16);
  _uniformBuffer->addUniform("opacityMatrix", 16);
  _uniformBuffer->addUniform("reflectionMatrix", 16);
  _uniformBuffer->addUniform("emissiveMatrix", 16);
  _uniformBuffer->addUniform("lightmapMatrix", 16);
  _uniformBuffer->addUniform("specularMatrix", 16);
  _uniformBuffer->addUniform("bumpMatrix", 16);
  _uniformBuffer->addUniform("refractionMatrix", 16);
  _uniformBuffer->addUniform("vRefractionInfos", 4);
  _uniformBuffer->addUniform("vSpecularColor", 4);
  _uniformBuffer->addUniform("vEmissiveColor", 3);
  _uniformBuffer->addUniform("vDiffuseColor", 4);
  _uniformBuffer->addUniform("pointSize", 1);

  _uniformBuffer->create();
}

void StandardMaterial::unbind()
{
  if (_activeEffect) {
    if (_reflectionTexture && _reflectionTexture->isRenderTarget) {
      _activeEffect->setTexture("reflection2DSampler", nullptr);
    }

    if (_refractionTexture && _refractionTexture->isRenderTarget) {
      _activeEffect->setTexture("refraction2DSampler", nullptr);
    }
  }

  PushMaterial::unbind();
}

void StandardMaterial::bindForSubMesh(Matrix* world, Mesh* mesh,
                                      SubMesh* subMesh)
{
  auto scene = getScene();

  auto definesTmp
    = static_cast<StandardMaterialDefines*>(subMesh->_materialDefines.get());
  if (!definesTmp) {
    return;
  }
  auto defines = *definesTmp;

  auto effect   = subMesh->effect();
  _activeEffect = effect;

  // Matrices
  bindOnlyWorldMatrix(*world);

  // Bones
  MaterialHelper::BindBonesParameters(mesh, effect);
  if (_mustRebind(scene, effect, mesh->visibility)) {
    _uniformBuffer->bindToEffect(effect, "Material");

    bindViewProjection(effect);
    if (!_uniformBuffer->useUbo() || !isFrozen() || !_uniformBuffer->isSync()) {

      if (StandardMaterial::FresnelEnabled() && defines[SMD::FRESNEL]) {
        // Fresnel
        if (_diffuseFresnelParameters
            && _diffuseFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("diffuseLeftColor",
                                       _diffuseFresnelParameters->leftColor,
                                       _diffuseFresnelParameters->power, "");
          _uniformBuffer->updateColor4("diffuseRightColor",
                                       _diffuseFresnelParameters->rightColor,
                                       _diffuseFresnelParameters->bias, "");
        }

        if (_opacityFresnelParameters
            && _opacityFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4(
            "opacityParts",
            Color3(_opacityFresnelParameters->leftColor.toLuminance(),
                   _opacityFresnelParameters->rightColor.toLuminance(),
                   _opacityFresnelParameters->bias),
            _opacityFresnelParameters->power, "");
        }

        if (_reflectionFresnelParameters
            && _reflectionFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("reflectionLeftColor",
                                       _reflectionFresnelParameters->leftColor,
                                       _reflectionFresnelParameters->power, "");
          _uniformBuffer->updateColor4("reflectionRightColor",
                                       _reflectionFresnelParameters->rightColor,
                                       _reflectionFresnelParameters->bias, "");
        }

        if (_refractionFresnelParameters
            && _refractionFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("refractionLeftColor",
                                       _refractionFresnelParameters->leftColor,
                                       _refractionFresnelParameters->power, "");
          _uniformBuffer->updateColor4("refractionRightColor",
                                       _refractionFresnelParameters->rightColor,
                                       _refractionFresnelParameters->bias, "");
        }

        if (_emissiveFresnelParameters
            && _emissiveFresnelParameters->isEnabled()) {
          _uniformBuffer->updateColor4("emissiveLeftColor",
                                       _emissiveFresnelParameters->leftColor,
                                       _emissiveFresnelParameters->power, "");
          _uniformBuffer->updateColor4("emissiveRightColor",
                                       _emissiveFresnelParameters->rightColor,
                                       _emissiveFresnelParameters->bias, "");
        }
      }

      // Textures
      if (scene->texturesEnabled()) {
        if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
          _uniformBuffer->updateFloat2("vDiffuseInfos",
                                       _diffuseTexture->coordinatesIndex,
                                       _diffuseTexture->level);
          _uniformBuffer->updateMatrix("diffuseMatrix",
                                       *_diffuseTexture->getTextureMatrix());
        }

        if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
          _uniformBuffer->updateFloat2("vAmbientInfos",
                                       _ambientTexture->coordinatesIndex,
                                       _ambientTexture->level);
          _uniformBuffer->updateMatrix("ambientMatrix",
                                       *_ambientTexture->getTextureMatrix());
        }

        if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
          _uniformBuffer->updateFloat2("vOpacityInfos",
                                       _opacityTexture->coordinatesIndex,
                                       _opacityTexture->level);
          _uniformBuffer->updateMatrix("opacityMatrix",
                                       *_opacityTexture->getTextureMatrix());
        }

        if (_reflectionTexture
            && StandardMaterial::ReflectionTextureEnabled()) {
          _uniformBuffer->updateFloat2("vReflectionInfos",
                                       _reflectionTexture->level, _roughness);
          _uniformBuffer->updateMatrix(
            "reflectionMatrix",
            *_reflectionTexture->getReflectionTextureMatrix());
        }

        if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
          _uniformBuffer->updateFloat2("vEmissiveInfos",
                                       _emissiveTexture->coordinatesIndex,
                                       _emissiveTexture->level);
          _uniformBuffer->updateMatrix("emissiveMatrix",
                                       *_emissiveTexture->getTextureMatrix());
        }

        if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
          _uniformBuffer->updateFloat2("vLightmapInfos",
                                       _lightmapTexture->coordinatesIndex,
                                       _lightmapTexture->level);
          _uniformBuffer->updateMatrix("lightmapMatrix",
                                       *_lightmapTexture->getTextureMatrix());
        }

        if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
          _uniformBuffer->updateFloat2("vSpecularInfos",
                                       _specularTexture->coordinatesIndex,
                                       _specularTexture->level);
          _uniformBuffer->updateMatrix("specularMatrix",
                                       *_specularTexture->getTextureMatrix());
        }

        if (_bumpTexture && scene->getEngine()->getCaps().standardDerivatives
            && StandardMaterial::BumpTextureEnabled()) {
          _uniformBuffer->updateFloat3(
            "vBumpInfos", static_cast<float>(_bumpTexture->coordinatesIndex),
            1.f / _bumpTexture->level, parallaxScaleBias, "");
          _uniformBuffer->updateMatrix("bumpMatrix",
                                       *_bumpTexture->getTextureMatrix());
        }

        if (_refractionTexture
            && StandardMaterial::RefractionTextureEnabled()) {
          float depth = 1.f;
          if (!_refractionTexture->isCube) {
            _uniformBuffer->updateMatrix(
              "refractionMatrix",
              *_refractionTexture->getReflectionTextureMatrix());
            auto refractionTextureTmp
              = static_cast<RefractionTexture*>(_refractionTexture);
            if (refractionTextureTmp) {
              depth = refractionTextureTmp->depth;
            }
          }
          _uniformBuffer->updateFloat4(
            "vRefractionInfos", _refractionTexture->level, indexOfRefraction,
            depth, invertRefractionY ? -1.f : 1.f, "");
        }
      }

      // Point size
      if (pointsCloud()) {
        _uniformBuffer->updateFloat("pointSize", pointSize);
      }

      if (defines.SPECULARTERM) {
        _uniformBuffer->updateColor4("vSpecularColor", specularColor,
                                     specularPower, "");
      }
      _uniformBuffer->updateColor3("vEmissiveColor", emissiveColor, "");
      // Diffuse
      _uniformBuffer->updateColor4("vDiffuseColor", diffuseColor,
                                   alpha * mesh->visibility, "");
    }

    // Textures
    if (scene->texturesEnabled()) {
      if (_diffuseTexture && StandardMaterial::DiffuseTextureEnabled()) {
        effect->setTexture("diffuseSampler", _diffuseTexture);
      }

      if (_ambientTexture && StandardMaterial::AmbientTextureEnabled()) {
        effect->setTexture("ambientSampler", _ambientTexture);
      }

      if (_opacityTexture && StandardMaterial::OpacityTextureEnabled()) {
        effect->setTexture("opacitySampler", _opacityTexture);
      }

      if (_reflectionTexture && StandardMaterial::ReflectionTextureEnabled()) {
        if (_reflectionTexture->isCube) {
          effect->setTexture("reflectionCubeSampler", _reflectionTexture);
        }
        else {
          effect->setTexture("reflection2DSampler", _reflectionTexture);
        }
      }

      if (_emissiveTexture && StandardMaterial::EmissiveTextureEnabled()) {
        effect->setTexture("emissiveSampler", _emissiveTexture);
      }

      if (_lightmapTexture && StandardMaterial::LightmapTextureEnabled()) {
        effect->setTexture("lightmapSampler", _lightmapTexture);
      }

      if (_specularTexture && StandardMaterial::SpecularTextureEnabled()) {
        effect->setTexture("specularSampler", _specularTexture);
      }

      if (_bumpTexture && scene->getEngine()->getCaps().standardDerivatives
          && StandardMaterial::BumpTextureEnabled()) {
        effect->setTexture("bumpSampler", _bumpTexture);
      }

      if (_refractionTexture && StandardMaterial::RefractionTextureEnabled()) {
        if (_refractionTexture->isCube) {
          effect->setTexture("refractionCubeSampler", _refractionTexture);
        }
        else {
          effect->setTexture("refraction2DSampler", _refractionTexture);
        }
      }

      if (_cameraColorGradingTexture
          && StandardMaterial::ColorGradingTextureEnabled()) {
        ColorGradingTexture::Bind(_cameraColorGradingTexture, effect);
      }
    }

    // Clip plane
    MaterialHelper::BindClipPlane(effect, scene);

    // Colors
    scene->ambientColor.multiplyToRef(ambientColor, _globalAmbientColor);

    effect->setVector3("vEyePosition", scene->_mirroredCameraPosition ?
                                         *scene->_mirroredCameraPosition :
                                         scene->activeCamera->position);
    effect->setColor3("vAmbientColor", _globalAmbientColor);
  }

  if (_mustRebind(scene, effect) || !isFrozen()) {
    // Lights
    if (scene->lightsEnabled() && !_disableLighting) {
      MaterialHelper::BindLights(scene, mesh, effect, defines,
                                 _maxSimultaneousLights, SMD::SPECULARTERM);
    }

    // View
    if ((scene->fogEnabled() && mesh->applyFog()
         && (scene->fogMode() != Scene::FOGMODE_NONE))
        || _reflectionTexture || _refractionTexture) {
      bindView(effect);
    }

    // Fog
    MaterialHelper::BindFogParameters(scene, mesh, effect);

    // Morph targets
    if (defines.NUM_MORPH_INFLUENCERS) {
      MaterialHelper::BindMorphTargetParameters(mesh, effect);
    }

    // Log. depth
    MaterialHelper::BindLogDepth(defines, effect, scene, SMD::LOGARITHMICDEPTH);

    // Color Curves
    if (_cameraColorCurves) {
      ColorCurves::Bind(*_cameraColorCurves, effect);
    }
  }

  _uniformBuffer->update();
  _afterBind(mesh, _activeEffect);
}

std::vector<IAnimatable*> StandardMaterial::getAnimatables()
{
  std::vector<IAnimatable*> results;

  if (_diffuseTexture && _diffuseTexture->animations.size() > 0) {
    results.emplace_back(_diffuseTexture);
  }

  if (_ambientTexture && _ambientTexture->animations.size() > 0) {
    results.emplace_back(_ambientTexture);
  }

  if (_opacityTexture && _opacityTexture->animations.size() > 0) {
    results.emplace_back(_opacityTexture);
  }

  if (_reflectionTexture && _reflectionTexture->animations.size() > 0) {
    results.emplace_back(_reflectionTexture);
  }

  if (_emissiveTexture && _emissiveTexture->animations.size() > 0) {
    results.emplace_back(_emissiveTexture);
  }

  if (_specularTexture && _specularTexture->animations.size() > 0) {
    results.emplace_back(_specularTexture);
  }

  if (_bumpTexture && _bumpTexture->animations.size() > 0) {
    results.emplace_back(_bumpTexture);
  }

  if (_lightmapTexture && _lightmapTexture->animations.size() > 0) {
    results.emplace_back(_lightmapTexture);
  }

  if (_refractionTexture && _refractionTexture->animations.size() > 0) {
    results.emplace_back(_refractionTexture);
  }

  if (_cameraColorGradingTexture
      && _cameraColorGradingTexture->animations.size() > 0) {
    results.emplace_back(_cameraColorGradingTexture);
  }

  return results;
}

void StandardMaterial::dispose(bool forceDisposeEffect,
                               bool forceDisposeTextures)
{
  if (forceDisposeTextures) {
    if (_diffuseTexture) {
      _diffuseTexture->dispose();
    }

    if (_ambientTexture) {
      _ambientTexture->dispose();
    }

    if (_opacityTexture) {
      _opacityTexture->dispose();
    }

    if (_reflectionTexture) {
      _reflectionTexture->dispose();
    }

    if (_emissiveTexture) {
      _emissiveTexture->dispose();
    }

    if (_specularTexture) {
      _specularTexture->dispose();
    }

    if (_bumpTexture) {
      _bumpTexture->dispose();
    }

    if (_lightmapTexture) {
      _lightmapTexture->dispose();
    }

    if (_refractionTexture) {
      _refractionTexture->dispose();
    }

    if (_cameraColorGradingTexture) {
      _cameraColorGradingTexture->dispose();
    }
  }

  Material::dispose(forceDisposeEffect, forceDisposeTextures);
}

Material* StandardMaterial::clone(const std::string& _name,
                                  bool /*cloneChildren*/) const
{
  auto standardMaterial  = StandardMaterial::New(*this);
  standardMaterial->name = _name;
  standardMaterial->id   = _name;
  return standardMaterial;
}

Json::object StandardMaterial::serialize() const
{
  return Json::object();
}

BaseTexture* StandardMaterial::emissiveTexture() const
{
  return _emissiveTexture;
}

bool StandardMaterial::useAlphaFromDiffuseTexture() const
{
  return _useAlphaFromDiffuseTexture;
}

void StandardMaterial::setUseAlphaFromDiffuseTexture(bool value)
{
  if (_useAlphaFromDiffuseTexture == value) {
    return;
  }
  _useAlphaFromDiffuseTexture = value;
}

bool StandardMaterial::useEmissiveAsIllumination() const
{
  return _useEmissiveAsIllumination;
}

void StandardMaterial::setUseEmissiveAsIllumination(bool value)
{
  if (_useEmissiveAsIllumination == value) {
    return;
  }
  _useEmissiveAsIllumination = value;
}

bool StandardMaterial::linkEmissiveWithDiffuse() const
{
  return _linkEmissiveWithDiffuse;
}

void StandardMaterial::setLinkEmissiveWithDiffuse(bool value)
{
  if (_linkEmissiveWithDiffuse == value) {
    return;
  }
  _linkEmissiveWithDiffuse = value;
}

bool StandardMaterial::useReflectionFresnelFromSpecular() const
{
  return _useReflectionFresnelFromSpecular;
}

void StandardMaterial::setUseReflectionFresnelFromSpecular(bool value)
{
  if (_useReflectionFresnelFromSpecular == value) {
    return;
  }
  _useReflectionFresnelFromSpecular = value;
}

bool StandardMaterial::useSpecularOverAlpha() const
{
  return _useSpecularOverAlpha;
}

void StandardMaterial::setUseSpecularOverAlpha(bool value)
{
  if (_useSpecularOverAlpha == value) {
    return;
  }
  _useSpecularOverAlpha = value;
}

bool StandardMaterial::useReflectionOverAlpha() const
{
  return _useReflectionOverAlpha;
}

void StandardMaterial::setUseReflectionOverAlpha(bool value)
{
  if (_useReflectionOverAlpha == value) {
    return;
  }
  _useReflectionOverAlpha = value;
}

bool StandardMaterial::disableLighting() const
{
  return _disableLighting;
}

void StandardMaterial::setDisableLighting(bool value)
{
  if (_disableLighting == value) {
    return;
  }
  _disableLighting = value;
}

bool StandardMaterial::useParallax() const
{
  return _useParallax;
}

void StandardMaterial::setUseParallax(bool value)
{
  if (_useParallax == value) {
    return;
  }
  _useParallax = value;
}

bool StandardMaterial::useParallaxOcclusion() const
{
  return _useParallaxOcclusion;
}

void StandardMaterial::setUseParallaxOcclusion(bool value)
{
  if (_useParallaxOcclusion == value) {
    return;
  }
  _useParallaxOcclusion = value;
}

float StandardMaterial::roughness() const
{
  return _roughness;
}

bool StandardMaterial::useLightmapAsShadowmap() const
{
  return _useLightmapAsShadowmap;
}

void StandardMaterial::setUseLightmapAsShadowmap(bool value)
{
  if (_useLightmapAsShadowmap == value) {
    return;
  }
  _useLightmapAsShadowmap = value;
}

bool StandardMaterial::useGlossinessFromSpecularMapAlpha() const
{
  return _useGlossinessFromSpecularMapAlpha;
}

void StandardMaterial::setUseGlossinessFromSpecularMapAlpha(bool value)
{
  if (_useGlossinessFromSpecularMapAlpha == value) {
    return;
  }
  _useGlossinessFromSpecularMapAlpha = value;
}

unsigned int StandardMaterial::maxSimultaneousLights() const
{
  return _maxSimultaneousLights;
}

void StandardMaterial::setMaxSimultaneousLights(unsigned int value)
{
  if (_maxSimultaneousLights == value) {
    return;
  }
  _maxSimultaneousLights = value;
}

bool StandardMaterial::invertNormalMapX() const
{
  return _invertNormalMapX;
}

void StandardMaterial::setInvertNormalMapX(bool value)
{
  if (_invertNormalMapX == value) {
    return;
  }
  _invertNormalMapX = value;
}

bool StandardMaterial::invertNormalMapY() const
{
  return _invertNormalMapY;
}

void StandardMaterial::setInvertNormalMapY(bool value)
{
  if (_invertNormalMapY == value) {
    return;
  }
  _invertNormalMapY = value;
}

void StandardMaterial::setRoughness(float value)
{
  if (stl_util::almost_equal(_roughness, value)) {
    return;
  }
  _roughness = value;
}

StandardMaterial* StandardMaterial::Parse(const Json::value& source,
                                          Scene* scene,
                                          const std::string& rootUrl)
{
  return SerializationHelper::Parse(
    StandardMaterial::New(Json::GetString(source, "name"), scene), source,
    scene, rootUrl);
}

bool StandardMaterial::DiffuseTextureEnabled()
{
  return StandardMaterial::_DiffuseTextureEnabled;
}

void StandardMaterial::SetDiffuseTextureEnabled(bool value)
{
  if (StandardMaterial::_DiffuseTextureEnabled == value) {
    return;
  }

  StandardMaterial::_DiffuseTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::AmbientTextureEnabled()
{
  return StandardMaterial::_AmbientTextureEnabled;
}

void StandardMaterial::SetAmbientTextureEnabled(bool value)
{
  if (StandardMaterial::_AmbientTextureEnabled == value) {
    return;
  }

  StandardMaterial::_AmbientTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::OpacityTextureEnabled()
{
  return StandardMaterial::_OpacityTextureEnabled;
}

void StandardMaterial::SetOpacityTextureEnabled(bool value)
{
  if (StandardMaterial::_OpacityTextureEnabled == value) {
    return;
  }

  StandardMaterial::_OpacityTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::ReflectionTextureEnabled()
{
  return StandardMaterial::_ReflectionTextureEnabled;
}

void StandardMaterial::SetReflectionTextureEnabled(bool value)
{
  if (StandardMaterial::_ReflectionTextureEnabled == value) {
    return;
  }

  StandardMaterial::_ReflectionTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::EmissiveTextureEnabled()
{
  return StandardMaterial::_EmissiveTextureEnabled;
}

void StandardMaterial::SetEmissiveTextureEnabled(bool value)
{
  if (StandardMaterial::_EmissiveTextureEnabled == value) {
    return;
  }

  StandardMaterial::_EmissiveTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::SpecularTextureEnabled()
{
  return StandardMaterial::_SpecularTextureEnabled;
}

void StandardMaterial::SetSpecularTextureEnabled(bool value)
{
  if (StandardMaterial::_SpecularTextureEnabled == value) {
    return;
  }

  StandardMaterial::_SpecularTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::BumpTextureEnabled()
{
  return StandardMaterial::_BumpTextureEnabled;
}

void StandardMaterial::SetBumpTextureEnabled(bool value)
{
  if (StandardMaterial::_BumpTextureEnabled == value) {
    return;
  }

  StandardMaterial::_BumpTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::LightmapTextureEnabled()
{
  return StandardMaterial::_LightmapTextureEnabled;
}

void StandardMaterial::SetLightmapTextureEnabled(bool value)
{
  if (StandardMaterial::_LightmapTextureEnabled == value) {
    return;
  }

  StandardMaterial::_LightmapTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::RefractionTextureEnabled()
{
  return StandardMaterial::_RefractionTextureEnabled;
}

void StandardMaterial::SetRefractionTextureEnabled(bool value)
{
  if (StandardMaterial::_RefractionTextureEnabled == value) {
    return;
  }

  StandardMaterial::_RefractionTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::ColorGradingTextureEnabled()
{
  return StandardMaterial::_ColorGradingTextureEnabled;
}

void StandardMaterial::SetColorGradingTextureEnabled(bool value)
{
  if (StandardMaterial::_ColorGradingTextureEnabled == value) {
    return;
  }

  StandardMaterial::_ColorGradingTextureEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::TextureDirtyFlag);
}

bool StandardMaterial::FresnelEnabled()
{
  return StandardMaterial::_FresnelEnabled;
}

void StandardMaterial::SetFresnelEnabled(bool value)
{
  if (StandardMaterial::_FresnelEnabled == value) {
    return;
  }

  StandardMaterial::_FresnelEnabled = value;
  Engine::MarkAllMaterialsAsDirty(Material::FresnelDirtyFlag);
}

} // end of namespace BABYLON
