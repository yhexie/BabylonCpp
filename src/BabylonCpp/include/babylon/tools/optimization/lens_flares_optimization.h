#ifndef BABYLON_TOOLS_OPTIMIZATION_LENS_FLARES_OPTIMIZATION_H
#define BABYLON_TOOLS_OPTIMIZATION_LENS_FLARES_OPTIMIZATION_H

#include <babylon/babylon_global.h>
#include <babylon/tools/optimization/scene_optimization.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT LensFlaresOptimization : public SceneOptimization {

public:
  LensFlaresOptimization(int priority = 0);
  ~LensFlaresOptimization();

  bool apply(Scene* scene) override;

}; // end of class LensFlaresOptimization

} // end of namespace BABYLON

#endif // end of BABYLON_TOOLS_OPTIMIZATION_LENS_FLARES_OPTIMIZATION_H