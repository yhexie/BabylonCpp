#ifndef BABYLON_COLLISIONS_COLLISION_CACHE_H
#define BABYLON_COLLISIONS_COLLISION_CACHE_H

#include <babylon/babylon_global.h>
#include <babylon/collisions/serialized_geometry.h>
#include <babylon/collisions/serialized_mesh.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT CollisionCache {

public:
  CollisionCache();
  ~CollisionCache();

  std::unordered_map<unsigned int, SerializedMesh>& getMeshes();
  std::unordered_map<std::string, SerializedGeometry>& getGeometries();
  bool containsMesh(unsigned int id) const;
  SerializedMesh& getMesh(unsigned int id);
  void addMesh(const SerializedMesh& mesh);
  void removeMesh(unsigned int uniqueId);
  bool containsGeometry(const std::string& id) const;
  SerializedGeometry& getGeometry(const std::string& id);
  void addGeometry(const SerializedGeometry& geometry);
  void removeGeometry(const std::string& id);

private:
  std::unordered_map<unsigned int, SerializedMesh> _meshes;
  std::unordered_map<std::string, SerializedGeometry> _geometries;

}; // end of class CollisionCache

} // end of namespace BABYLON

#endif // end of BABYLON_COLLISIONS_COLLISION_CACHE_H
