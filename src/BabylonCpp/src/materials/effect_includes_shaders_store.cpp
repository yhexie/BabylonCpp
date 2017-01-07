﻿#include <babylon/materials/effect_includes_shaders_store.h>

#include <babylon/shaders/shadersinclude/bones_declaration_fx.h>
#include <babylon/shaders/shadersinclude/bones_vertex_fx.h>
#include <babylon/shaders/shadersinclude/bump_fragment_fx.h>
#include <babylon/shaders/shadersinclude/bump_fragment_functions_fx.h>
#include <babylon/shaders/shadersinclude/clip_plane_fragment_fx.h>
#include <babylon/shaders/shadersinclude/clip_plane_fragment_declaration_fx.h>
#include <babylon/shaders/shadersinclude/clip_plane_vertex_fx.h>
#include <babylon/shaders/shadersinclude/clip_plane_vertex_declaration_fx.h>
#include <babylon/shaders/shadersinclude/color_curves_fx.h>
#include <babylon/shaders/shadersinclude/color_curves_definition_fx.h>
#include <babylon/shaders/shadersinclude/color_grading_fx.h>
#include <babylon/shaders/shadersinclude/color_grading_definition_fx.h>
#include <babylon/shaders/shadersinclude/fog_fragment_fx.h>
#include <babylon/shaders/shadersinclude/fog_fragment_declaration_fx.h>
#include <babylon/shaders/shadersinclude/fog_vertex_fx.h>
#include <babylon/shaders/shadersinclude/fog_vertex_declaration_fx.h>
#include <babylon/shaders/shadersinclude/fresnel_function_fx.h>
#include <babylon/shaders/shadersinclude/harmonics_functions_fx.h>
#include <babylon/shaders/shadersinclude/helper_functions_fx.h>
#include <babylon/shaders/shadersinclude/instances_declaration_fx.h>
#include <babylon/shaders/shadersinclude/instances_vertex_fx.h>
#include <babylon/shaders/shadersinclude/light_fragment_fx.h>
#include <babylon/shaders/shadersinclude/light_fragment_declaration_fx.h>
#include <babylon/shaders/shadersinclude/lights_fragment_functions_fx.h>
#include <babylon/shaders/shadersinclude/log_depth_declaration_fx.h>
#include <babylon/shaders/shadersinclude/log_depth_fragment_fx.h>
#include <babylon/shaders/shadersinclude/log_depth_vertex_fx.h>
#include <babylon/shaders/shadersinclude/pbr_functions_fx.h>
#include <babylon/shaders/shadersinclude/pbr_light_functions_fx.h>
#include <babylon/shaders/shadersinclude/pbr_light_functions_call_fx.h>
#include <babylon/shaders/shadersinclude/pbr_shadow_functions_fx.h>
#include <babylon/shaders/shadersinclude/point_cloud_vertex_fx.h>
#include <babylon/shaders/shadersinclude/point_cloud_vertex_declaration_fx.h>
#include <babylon/shaders/shadersinclude/reflection_function_fx.h>
#include <babylon/shaders/shadersinclude/shadows_fragment_functions_fx.h>
#include <babylon/shaders/shadersinclude/shadows_vertex_fx.h>
#include <babylon/shaders/shadersinclude/shadows_vertex_declaration_fx.h>

namespace BABYLON {

std::unordered_map<std::string, const char*> EffectIncludesShadersStore::Shaders  
 = {{"bonesDeclaration", bonesDeclaration},
   {"bonesVertex", bonesVertex},
   {"bumpFragment", bumpFragment},
   {"bumpFragmentFunctions", bumpFragmentFunctions},
   {"clipPlaneFragment", clipPlaneFragment},
   {"clipPlaneFragmentDeclaration", clipPlaneFragmentDeclaration},
   {"clipPlaneVertex", clipPlaneVertex},
   {"clipPlaneVertexDeclaration", clipPlaneVertexDeclaration},
   {"colorCurves", colorCurves},
   {"colorCurvesDefinition", colorCurvesDefinition},
   {"colorGrading", colorGrading},
   {"colorGradingDefinition", colorGradingDefinition},
   {"fogFragment", fogFragment},
   {"fogFragmentDeclaration", fogFragmentDeclaration},
   {"fogVertex", fogVertex},
   {"fogVertexDeclaration", fogVertexDeclaration},
   {"fresnelFunction", fresnelFunction},
   {"harmonicsFunctions", harmonicsFunctions},
   {"helperFunctions", helperFunctions},
   {"instancesDeclaration", instancesDeclaration},
   {"instancesVertex", instancesVertex},
   {"lightFragment", lightFragment},
   {"lightFragmentDeclaration", lightFragmentDeclaration},
   {"lightsFragmentFunctions", lightsFragmentFunctions},
   {"logDepthDeclaration", logDepthDeclaration},
   {"logDepthFragment", logDepthFragment},
   {"logDepthVertex", logDepthVertex},
   {"pbrFunctions", pbrFunctions},
   {"pbrLightFunctions", pbrLightFunctions},
   {"pbrLightFunctionsCall", pbrLightFunctionsCall},
   {"pbrShadowFunctions", pbrShadowFunctions},
   {"pointCloudVertex", pointCloudVertex},
   {"pointCloudVertexDeclaration", pointCloudVertexDeclaration},
   {"reflectionFunction", reflectionFunction},
   {"shadowsFragmentFunctions", shadowsFragmentFunctions},
   {"shadowsVertex", shadowsVertex},
   {"shadowsVertexDeclaration", shadowsVertexDeclaration}
};

} // end of namespace BABYLON
