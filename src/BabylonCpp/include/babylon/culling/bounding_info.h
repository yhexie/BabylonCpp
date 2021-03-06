#ifndef BABYLON_CULLING_BOUNDING_INFO_H
#define BABYLON_CULLING_BOUNDING_INFO_H

#include <babylon/babylon_global.h>
#include <babylon/core/structs.h>
#include <babylon/culling/bounding_box.h>
#include <babylon/culling/bounding_sphere.h>
#include <babylon/culling/icullable.h>

namespace BABYLON {

class BABYLON_SHARED_EXPORT BoundingInfo : public ICullable {

public:
  BoundingInfo(const Vector3& minimum, const Vector3& maximum);
  BoundingInfo(const BoundingInfo& boundingInfo);
  virtual ~BoundingInfo();

  /** Methods **/
  bool isLocked() const;
  void setIsLocked(bool value);
  void update(const Matrix& world);
  bool isInFrustum(const std::array<Plane, 6>& frustumPlanes) override;
  bool isCompletelyInFrustum(
    const std::array<Plane, 6>& frustumPlanes) const override;
  bool _checkCollision(const Collider& collider) const;
  bool intersectsPoint(const Vector3& point);
  bool intersects(const BoundingInfo& boundingInfo, bool precise);

private:
  Extents computeBoxExtents(const Vector3& axis, const BoundingBox& box) const;
  bool extentsOverlap(float min0, float max0, float min1, float max1) const;
  bool axisOverlap(const Vector3& axis, const BoundingBox& box0,
                   const BoundingBox& box1) const;

public:
  Vector3 minimum;
  Vector3 maximum;
  BoundingBox boundingBox;
  BoundingSphere boundingSphere;

private:
  bool _isLocked;

}; // end of class BoundingInfo

} // end of namespace BABYLON

#endif // end of BABYLON_CULLING_BOUNDING_INFO_H
