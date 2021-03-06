#ifndef BABYLON_POSTPROCESS_VOLUMETRIC_LIGHT_SCATTERING_POST_PROCESS_H
#define BABYLON_POSTPROCESS_VOLUMETRIC_LIGHT_SCATTERING_POST_PROCESS_H

#include <babylon/babylon_global.h>
#include <babylon/math/viewport.h>
#include <babylon/postprocess/post_process.h>

namespace BABYLON {

/**
 * @brief
 *
 * Inspired by http://http.developer.nvidia.com/GPUGems3/gpugems3_ch13.html
 */
class BABYLON_SHARED_EXPORT VolumetricLightScatteringPostProcess
  : public PostProcess {

public:
  /**
   * @constructor
   * @param {string} name - The post-process name
   * @param {any} ratio - The size of the post-process and/or internal pass (0.5
   * means that your postprocess will have a width = canvas.width 0.5 and a
   * height = canvas.height 0.5)
   * @param {BABYLON.Camera} camera - The camera that the post-process will be
   * attached to
   * @param {BABYLON.Mesh} mesh - The mesh used to create the light scattering
   * @param {number} samples - The post-process quality, default 100
   * @param {number} samplingMode - The post-process filtering mode
   * @param {BABYLON.Engine} engine - The babylon engine
   * @param {boolean} reusable - If the post-process is reusable
   * @param {BABYLON.Scene} scene - The constructor needs a scene reference to
   * initialize internal components. If "camera" is null (RenderPipelineà,
   * "scene" must be provided
   */
  VolumetricLightScatteringPostProcess(
    const std::string& name, float ratio, Camera* camera, Mesh* mesh,
    unsigned int samples      = 100,
    unsigned int samplingMode = TextureConstants::BILINEAR_SAMPLINGMODE,
    Engine* engine = nullptr, bool reusable = false, Scene* scene = nullptr);
  ~VolumetricLightScatteringPostProcess();

  bool isReady(SubMesh* subMesh, bool useInstances);

  /**
   * Sets the new light position for light scattering effect
   * @param {BABYLON.Vector3} The new custom light position
   */
  void setCustomMeshPosition(const Vector3& position);

  /**
   * Returns the light position for light scattering effect
   * @return {BABYLON.Vector3} The custom light position
   */
  Vector3& getCustomMeshPosition();

  /**
   * Disposes the internal assets and detaches the post-process from the camera
   */
  void dispose(Camera* camera);

  /**
   * Returns the render target texture used by the post-process
   * @return {BABYLON.RenderTargetTexture} The render target texture used by the
   * post-process
   */
  RenderTargetTexture* getPass();

  //** Static methods **/ /
  /**
   * Creates a default mesh for the Volumeric Light Scattering post-process
   * @param {string} The mesh name
   * @param {BABYLON.Scene} The scene where to create the mesh
   * @return {BABYLON.Mesh} the default mesh
   */
  static Mesh* CreateDefaultMesh(const std::string& name, Scene* scene);

private:
  bool _meshExcluded(AbstractMesh* mesh);
  void _createPass(Scene* scene, float ratio);
  void _updateMeshScreenCoordinates(Scene* scene);

public:
  /**
   * If not undefined, the mesh position is computed from the attached node
   * position
   */
  Vector3* attachedNode;
  /**
   * Custom position of the mesh. Used if "useCustomMeshPosition" is set to
   * "true"
   */
  Vector3 customMeshPosition;
  /**
   * Set if the post-process should use a custom position for the light source
   * (true) or the internal mesh position (false)
   */
  bool useCustomMeshPosition;
  /**
   * If the post-process should inverse the light scattering direction
   */
  bool invert;
  /**
   * The internal mesh used by the post-process
   */
  Mesh* mesh;
  /**
   * Set to true to use the diffuseColor instead of the diffuseTexture
   */
  bool useDiffuseColor;
  /**
   * Array containing the excluded meshes not rendered in the internal pass
   */
  std::vector<AbstractMesh*> excludedMeshes;
  /**
   * Controls the overall intensity of the post-process
   */
  float exposure;
  /**
   * Dissipates each sample's contribution in range [0, 1]
   */
  float decay;
  /**
   * Controls the overall intensity of each sample
   */
  float weight;
  /**
   * Controls the density of each sample
   */
  float density;

private:
  Effect* _volumetricLightScatteringPass;
  RenderTargetTexture* _volumetricLightScatteringRTT;
  Viewport _viewPort;
  Vector2 _screenCoordinates;
  std::string _cachedDefines;
  Vector3 _customMeshPosition;

}; // end of class VolumetricLightScatteringPostProcess

} // end of namespace BABYLON

#endif // end of BABYLON_POSTPROCESS_VOLUMETRIC_LIGHT_SCATTERING_POST_PROCESS_H
