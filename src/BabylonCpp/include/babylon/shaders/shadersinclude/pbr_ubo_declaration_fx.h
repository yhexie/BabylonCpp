﻿#ifndef BABYLON_SHADERS_SHADERS_INCLUDE_PBR_UBO_DECLARATION_FX_H
#define BABYLON_SHADERS_SHADERS_INCLUDE_PBR_UBO_DECLARATION_FX_H

namespace BABYLON {

extern const char* pbrUboDeclaration;

const char* pbrUboDeclaration
  = "layout(std140, column_major) uniform;\n"
    "\n"
    "uniform Material\n"
    "{\n"
    "  uniform vec2 vAlbedoInfos;\n"
    "  uniform vec3 vAmbientInfos;\n"
    "  uniform vec2 vOpacityInfos;\n"
    "  uniform vec2 vEmissiveInfos;\n"
    "  uniform vec2 vLightmapInfos;\n"
    "  uniform vec3 vReflectivityInfos;\n"
    "  uniform vec2 vMicroSurfaceSamplerInfos;\n"
    "  uniform vec4 vRefractionInfos;\n"
    "  uniform vec2 vReflectionInfos;\n"
    "  uniform vec3 vBumpInfos;\n"
    "  uniform mat4 albedoMatrix;\n"
    "  uniform mat4 ambientMatrix;\n"
    "  uniform mat4 opacityMatrix;\n"
    "  uniform mat4 emissiveMatrix;\n"
    "  uniform mat4 lightmapMatrix;\n"
    "  uniform mat4 reflectivityMatrix;\n"
    "  uniform mat4 microSurfaceSamplerMatrix;\n"
    "  uniform mat4 bumpMatrix;\n"
    "  uniform mat4 refractionMatrix;\n"
    "  uniform mat4 reflectionMatrix;\n"
    "\n"
    "  uniform vec3 vReflectionColor;\n"
    "  uniform vec4 vAlbedoColor;\n"
    "  uniform vec4 vLightingIntensity;\n"
    "\n"
    "  uniform vec2 vMicrosurfaceTextureLods;\n"
    "  uniform vec4 vReflectivityColor;\n"
    "  uniform vec3 vEmissiveColor;\n"
    "  uniform vec4 opacityParts;\n"
    "  uniform vec4 emissiveLeftColor;\n"
    "  uniform vec4 emissiveRightColor;\n"
    "\n"
    "  uniform vec4 vOverloadedIntensity;\n"
    "  uniform vec3 vOverloadedAmbient;\n"
    "  uniform vec3 vOverloadedAlbedo;\n"
    "  uniform vec3 vOverloadedReflectivity;\n"
    "  uniform vec3 vOverloadedEmissive;\n"
    "  uniform vec3 vOverloadedReflection;\n"
    "  uniform vec3 vOverloadedMicroSurface;\n"
    "  uniform vec4 vOverloadedShadowIntensity;\n"
    "\n"
    "  uniform float pointSize;\n"
    "};\n"
    "\n"
    "uniform Scene {\n"
    "  mat4 viewProjection;\n"
    "  mat4 view;\n"
    "};\n";

} // end of namespace BABYLON

#endif // end of BABYLON_SHADERS_SHADERS_INCLUDE_PBR_UBO_DECLARATION_FX_H
