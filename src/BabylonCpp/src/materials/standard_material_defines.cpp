#include <babylon/materials/standard_material_defines.h>

namespace BABYLON {

StandardMaterialDefines::StandardMaterialDefines() : MaterialDefines{}
{
  _keys = {"DIFFUSE",
           "AMBIENT",
           "OPACITY",
           "OPACITYRGB",
           "REFLECTION",
           "EMISSIVE",
           "SPECULAR",
           "BUMP",
           "PARALLAX",
           "PARALLAXOCCLUSION",
           "SPECULAROVERALPHA",
           "CLIPPLANE",
           "ALPHATEST",
           "ALPHAFROMDIFFUSE",
           "POINTSIZE",
           "FOG",
           "SPECULARTERM",
           "DIFFUSEFRESNEL",
           "OPACITYFRESNEL",
           "REFLECTIONFRESNEL",
           "REFRACTIONFRESNEL",
           "EMISSIVEFRESNEL",
           "FRESNEL",
           "NORMAL",
           "UV1",
           "UV2",
           "VERTEXCOLOR",
           "VERTEXALPHA",
           "INSTANCES",
           "GLOSSINESS",
           "ROUGHNESS",
           "EMISSIVEASILLUMINATION",
           "LINKEMISSIVEWITHDIFFUSE",
           "REFLECTIONFRESNELFROMSPECULAR",
           "LIGHTMAP",
           "USELIGHTMAPASSHADOWMAP",
           "REFLECTIONMAP_3D",
           "REFLECTIONMAP_SPHERICAL",
           "REFLECTIONMAP_PLANAR",
           "REFLECTIONMAP_CUBIC",
           "REFLECTIONMAP_PROJECTION",
           "REFLECTIONMAP_SKYBOX",
           "REFLECTIONMAP_EXPLICIT",
           "REFLECTIONMAP_EQUIRECTANGULAR",
           "REFLECTIONMAP_EQUIRECTANGULAR_FIXED",
           "REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED",
           "INVERTCUBICMAP",
           "LOGARITHMICDEPTH",
           "REFRACTION",
           "REFRACTIONMAP_3D",
           "REFLECTIONOVERALPHA",
           "INVERTNORMALMAPX",
           "INVERTNORMALMAPY",
           "TWOSIDEDLIGHTING",
           "SHADOWFLOAT",
           "CAMERACOLORGRADING",
           "CAMERACOLORCURVES",
           "MORPHTARGETS",
           "MORPHTARGETS_NORMAL",
           "MORPHTARGETS_TANGENT"};
  rebuild();
}

StandardMaterialDefines::~StandardMaterialDefines()
{
}

void StandardMaterialDefines::setReflectionMode(unsigned int modeToEnable)
{
  static const std::array<unsigned int, 9> modes{{
    REFLECTIONMAP_CUBIC, REFLECTIONMAP_EXPLICIT, REFLECTIONMAP_PLANAR,
    REFLECTIONMAP_PROJECTION, REFLECTIONMAP_SKYBOX, REFLECTIONMAP_SPHERICAL,
    REFLECTIONMAP_EQUIRECTANGULAR, REFLECTIONMAP_EQUIRECTANGULAR_FIXED,
    REFLECTIONMAP_MIRROREDEQUIRECTANGULAR_FIXED,
  }};

  for (auto& mode : modes) {
    defines[mode] = (mode == modeToEnable);
  }
}

} // end of namespace BABYLON
