#include <babylon/cameras/arc_rotate_camera.h>

#include <babylon/babylon_stl_util.h>
#include <babylon/collisions/icollision_coordinator.h>
#include <babylon/core/string.h>
#include <babylon/engine/engine.h>
#include <babylon/interfaces/icanvas.h>
#include <babylon/math/math_tools.h>
#include <babylon/mesh/mesh.h>

namespace BABYLON {

ArcRotateCamera::ArcRotateCamera(const std::string& iName, float iAlpha,
                                 float iBeta, float iRadius, Scene* scene)
    : ArcRotateCamera(iName, iAlpha, iBeta, iRadius, Vector3::Zero(), scene)
{
}

ArcRotateCamera::ArcRotateCamera(const std::string& iName, float iAlpha,
                                 float iBeta, float iRadius,
                                 const Vector3& iTarget, Scene* scene)
    : TargetCamera{iName, Vector3::Zero(), scene}
    , alpha{iAlpha}
    , beta{iBeta}
    , radius{iRadius}
    , inertialAlphaOffset{0.f}
    , inertialBetaOffset{0.f}
    , inertialRadiusOffset{0.f}
    , lowerAlphaLimit{0.f}
    , upperAlphaLimit{0.f}
    , lowerBetaLimit{0.01f}
    , upperBetaLimit{Math::PI}
    , lowerRadiusLimit{0.f}
    , upperRadiusLimit{0.f}
    , inertialPanningX{0.f}
    , inertialPanningY{0.f}
    , panningInertia{0.9f}
    , zoomOnFactor{1.f}
    , targetScreenOffset{Vector2::Zero()}
    , allowUpsideDown{true}
    , panningAxis{std::make_unique<Vector3>(1.f, 1.f, 0.f)}
    , checkCollisions{false}
    , collisionRadius{std::make_unique<Vector3>(0.5f, 0.5f, 0.5f)}
    , _targetHost{nullptr}
    , _collider{nullptr}
    , _previousPosition{Vector3::Zero()}
    , _collisionVelocity{Vector3::Zero()}
    , _newPosition{Vector3::Zero()}
    , _collisionTriggered{false}
{
  setTarget(iTarget);
  getViewMatrix();
  inputs = std::make_unique<ArcRotateCameraInputsManager>(this);
  inputs->addKeyboard().addMouseWheel().addPointers();
}

ArcRotateCamera::~ArcRotateCamera()
{
}

IReflect::Type ArcRotateCamera::type() const
{
  return IReflect::Type::ARCROTATECAMERA;
}

void ArcRotateCamera::_initCache()
{
  TargetCamera::_initCache();

  _cache._target = Vector3(std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max(),
                           std::numeric_limits<float>::max());
  _cache.alpha              = 0.f;
  _cache.beta               = 0.f;
  _cache.radius             = 0.f;
  _cache.targetScreenOffset = Vector2::Zero();
}

void ArcRotateCamera::_updateCache(bool ignoreParentClass)
{
  if (!ignoreParentClass) {
    TargetCamera::_updateCache(ignoreParentClass);
  }

  _cache._target.copyFrom(_getTargetPosition());
  _cache.alpha  = alpha;
  _cache.beta   = beta;
  _cache.radius = radius;
  _cache.targetScreenOffset.copyFrom(targetScreenOffset);
}

Vector3 ArcRotateCamera::_getTargetPosition()
{
  if (_targetHost && _targetHost->getAbsolutePosition()) {
    auto pos = _targetHost->getAbsolutePosition();
    if (_targetBoundingCenter) {
      pos->addToRef(*_targetBoundingCenter, _target);
    }
    else {
      _target.copyFrom(*pos);
    }
  }

  return _target;
}

bool ArcRotateCamera::_isSynchronizedViewMatrix()
{
  if (!TargetCamera::_isSynchronizedViewMatrix()) {
    return false;
  }

  return _cache._target.equals(_target)
         && stl_util::almost_equal(_cache.alpha, alpha)
         && stl_util::almost_equal(_cache.beta, beta)
         && stl_util::almost_equal(_cache.radius, radius)
         && _cache.targetScreenOffset.equals(targetScreenOffset);
}

void ArcRotateCamera::attachControl(ICanvas* canvas, bool noPreventDefault,
                                    bool useCtrlForPanning,
                                    MouseButtonType panningMouseButton)
{
  _useCtrlForPanning  = useCtrlForPanning;
  _panningMouseButton = panningMouseButton;

  inputs->attachElement(canvas, noPreventDefault);

  _reset = [&]() {
    inertialAlphaOffset  = 0.f;
    inertialBetaOffset   = 0.f;
    inertialRadiusOffset = 0.f;
  };
}

void ArcRotateCamera::detachControl(ICanvas* canvas)
{
  inputs->detachElement(canvas);

  if (_reset) {
    _reset();
  }
}

void ArcRotateCamera::_checkInputs()
{
  // if (async) collision inspection was triggered, don't update the camera's
  // position - until the collision callback was called.
  if (_collisionTriggered) {
    return;
  }

  inputs->checkInputs();

  // Inertia
  if (!stl_util::almost_equal(inertialAlphaOffset, 0.f)
      || !stl_util::almost_equal(inertialBetaOffset, 0.f)
      || !stl_util::almost_equal(inertialRadiusOffset, 0.f)) {

    if (getScene()->useRightHandedSystem()) {
      alpha -= beta <= 0.f ? -inertialAlphaOffset : inertialAlphaOffset;
    }
    else {
      alpha += beta <= 0.f ? -inertialAlphaOffset : inertialAlphaOffset;
    }

    beta += inertialBetaOffset;

    radius -= inertialRadiusOffset;
    inertialAlphaOffset *= inertia;
    inertialBetaOffset *= inertia;
    inertialRadiusOffset *= inertia;
    if (std::abs(inertialAlphaOffset) < speed * MathTools::Epsilon) {
      inertialAlphaOffset = 0.f;
    }
    if (std::abs(inertialBetaOffset) < speed * MathTools::Epsilon) {
      inertialBetaOffset = 0.f;
    }
    if (std::abs(inertialRadiusOffset) < speed * MathTools::Epsilon) {
      inertialRadiusOffset = 0.f;
    }
  }

  // Panning inertia
  if (!stl_util::almost_equal(inertialPanningX, 0.f)
      || !stl_util::almost_equal(inertialPanningY, 0.f)) {
    if (!_localDirection) {
      _localDirection       = std::make_unique<Vector3>(Vector3::Zero());
      _transformedDirection = Vector3::Zero();
    }

    _localDirection->copyFromFloats(inertialPanningX, inertialPanningY,
                                    inertialPanningY);
    _localDirection->multiplyInPlace(*panningAxis);
    _viewMatrix.invertToRef(_cameraTransformMatrix);
    Vector3::TransformNormalToRef(*_localDirection, _cameraTransformMatrix,
                                  _transformedDirection);
    // Eliminate y if map panning is enabled (panningAxis == 1,0,1)
    if (!stl_util::almost_equal(panningAxis->y, 0.f)) {
      _transformedDirection.y = 0;
    }

    if (!_targetHost) {
      _target.addInPlace(_transformedDirection);
    }

    inertialPanningX *= panningInertia;
    inertialPanningY *= panningInertia;

    if (std::abs(inertialPanningX) < speed * MathTools::Epsilon) {
      inertialPanningX = 0.f;
    }
    if (std::abs(inertialPanningY) < speed * MathTools::Epsilon) {
      inertialPanningY = 0.f;
    }
  }

  // Limits
  _checkLimits();

  TargetCamera::_checkInputs();
}

void ArcRotateCamera::_checkLimits()
{
  if (lowerBetaLimit == 0.f) {
    if (allowUpsideDown && beta > Math::PI) {
      beta = beta - (2 * Math::PI);
    }
  }
  else {
    if (beta < lowerBetaLimit) {
      beta = lowerBetaLimit;
    }
  }

  if (upperBetaLimit == 0.f) {
    if (allowUpsideDown && beta < -Math::PI) {
      beta = beta + (2 * Math::PI);
    }
  }
  else {
    if (beta > upperBetaLimit) {
      beta = upperBetaLimit;
    }
  }

  if ((!stl_util::almost_equal(lowerAlphaLimit, 0.f))
      && alpha < lowerAlphaLimit) {
    alpha = lowerAlphaLimit;
  }
  if ((!stl_util::almost_equal(upperAlphaLimit, 0.f))
      && alpha > upperAlphaLimit) {
    alpha = upperAlphaLimit;
  }

  if ((!stl_util::almost_equal(lowerRadiusLimit, 0.f))
      && radius < lowerRadiusLimit) {
    radius = lowerRadiusLimit;
  }
  if ((!stl_util::almost_equal(upperRadiusLimit, 0.f))
      && radius > upperRadiusLimit) {
    radius = upperRadiusLimit;
  }
}

void ArcRotateCamera::rebuildAnglesAndRadius()
{
  auto radiusv3 = position.subtract(_getTargetPosition());
  radius        = radiusv3.length();

  // Alpha
  alpha = std::acos(radiusv3.x / std::sqrt(std::pow(radiusv3.x, 2.f)
                                           + std::pow(radiusv3.z, 2.f)));

  if (radiusv3.z < 0.f) {
    alpha = Math::PI2 - alpha;
  }

  // Beta
  beta = std::acos(radiusv3.y / radius);

  _checkLimits();
}

void ArcRotateCamera::setPosition(const Vector3& iPosition)
{
  if (position.equals(iPosition)) {
    return;
  }

  position.copyFrom(iPosition);

  rebuildAnglesAndRadius();
}

Vector3& ArcRotateCamera::target()
{
  return _target;
}

const Vector3& ArcRotateCamera::target() const
{
  return _target;
}

void ArcRotateCamera::setTarget(AbstractMesh* iTarget, bool toBoundingCenter,
                                bool /*allowSamePosition*/)
{
  if (iTarget->getBoundingInfo()) {
    if (toBoundingCenter) {
      _targetBoundingCenter
        = iTarget->getBoundingInfo()->boundingBox.centerWorld.clone();
    }
    else {
      _targetBoundingCenter.reset(nullptr);
    }
    _targetHost = iTarget;
    _target     = _getTargetPosition();
  }

  rebuildAnglesAndRadius();
}

void ArcRotateCamera::setTarget(const Vector3& iTarget,
                                bool /*toBoundingCenter*/,
                                bool allowSamePosition)
{
  Vector3 newTarget{iTarget};
  if (!allowSamePosition && _getTargetPosition().equals(newTarget)) {
    return;
  }
  _target = newTarget;
  _targetBoundingCenter.reset(nullptr);

  rebuildAnglesAndRadius();
}

Matrix ArcRotateCamera::_getViewMatrix()
{
  // Compute
  const float cosa = std::cos(alpha);
  const float sina = std::sin(alpha);
  const float cosb = std::cos(beta);
  float sinb       = std::sin(beta);

  if (stl_util::almost_equal(sinb, 0.f)) {
    sinb = 0.0001f;
  }

  auto _target = _getTargetPosition();
  _target.addToRef(
    Vector3(radius * cosa * sinb, radius * cosb, radius * sina * sinb),
    _newPosition);
  if (getScene()->collisionsEnabled && checkCollisions) {
    if (!_collider) {
      _collider = std::make_unique<Collider>();
    }
    _collider->radius = *collisionRadius;
    _newPosition.subtractToRef(position, _collisionVelocity);
    _collisionTriggered = true;
    getScene()->collisionCoordinator->getNewPosition(
      position, _collisionVelocity, _collider.get(), 3, nullptr,
      [&](int collisionId, Vector3& newPosition, AbstractMesh* collidedMesh) {
        _onCollisionPositionChange(collisionId, newPosition, collidedMesh);
      },
      uniqueId);
  }
  else {
    position.copyFrom(_newPosition);

    auto up = upVector;
    if (allowUpsideDown && sinb < 0.f) {
      up = up.negate();
    }

    if (getScene()->useRightHandedSystem()) {
      Matrix::LookAtRHToRef(position, _target, up, _viewMatrix);
    }
    else {
      Matrix::LookAtLHToRef(position, _target, up, _viewMatrix);
    }
    _viewMatrix.m[12] += targetScreenOffset.x;
    _viewMatrix.m[13] += targetScreenOffset.y;
  }
  _currentTarget = _target;

  return _viewMatrix;
}

void ArcRotateCamera::_onCollisionPositionChange(int /*collisionId*/,
                                                 Vector3& newPosition,
                                                 AbstractMesh* collidedMesh)
{
  if (getScene()->workerCollisions() && checkCollisions) {
    newPosition.multiplyInPlace(_collider->radius);
  }

  if (!collidedMesh) {
    _previousPosition.copyFrom(position);
  }
  else {
    setPosition(newPosition);

    if (onCollide) {
      onCollide(collidedMesh);
    }
  }

  // Recompute because of constraints
  const float cosa = std::cos(alpha);
  const float sina = std::sin(alpha);
  const float cosb = std::cos(beta);
  float sinb       = std::sin(beta);

  if (stl_util::almost_equal(sinb, 0.f)) {
    sinb = 0.0001f;
  }

  auto _target = _getTargetPosition();
  _target.addToRef(
    Vector3(radius * cosa * sinb, radius * cosb, radius * sina * sinb),
    _newPosition);
  position.copyFrom(_newPosition);

  auto up = upVector;
  if (allowUpsideDown && beta < 0.f) {
    up = up.negate();
  }

  Matrix::LookAtLHToRef(position, _target, up, _viewMatrix);
  _viewMatrix.m[12] += targetScreenOffset.x;
  _viewMatrix.m[13] += targetScreenOffset.y;

  _collisionTriggered = false;
}

void ArcRotateCamera::zoomOn(const std::vector<AbstractMesh*> meshes,
                             bool doNotUpdateMaxZ)
{
  auto _meshes = meshes.empty() ? getScene()->getMeshes() : meshes;

  auto minMaxVector = Mesh::GetMinMax(_meshes);
  auto distance     = Vector3::Distance(minMaxVector.min, minMaxVector.max);

  radius = distance * zoomOnFactor;

  focusOn({minMaxVector.min, minMaxVector.max, distance}, doNotUpdateMaxZ);
}

void ArcRotateCamera::focusOn(
  const MinMaxDistance& meshesOrMinMaxVectorAndDistance, bool doNotUpdateMaxZ)
{
  _target = Mesh::Center(meshesOrMinMaxVectorAndDistance);

  if (!doNotUpdateMaxZ) {
    maxZ = meshesOrMinMaxVectorAndDistance.distance * 2.f;
  }
}

Camera* ArcRotateCamera::createRigCamera(const std::string& iName,
                                         int cameraIndex)
{
  float alphaShift = 0.f;

  switch (cameraRigMode) {
    case Camera::RIG_MODE_STEREOSCOPIC_ANAGLYPH:
    case Camera::RIG_MODE_STEREOSCOPIC_SIDEBYSIDE_PARALLEL:
    case Camera::RIG_MODE_STEREOSCOPIC_OVERUNDER:
    case Camera::RIG_MODE_VR:
      alphaShift
        = _cameraRigParams.stereoHalfAngle * (cameraIndex == 0 ? 1 : -1);
      break;
    case Camera::RIG_MODE_STEREOSCOPIC_SIDEBYSIDE_CROSSEYED:
      alphaShift
        = _cameraRigParams.stereoHalfAngle * (cameraIndex == 0 ? -1 : 1);
      break;
    default:
      break;
  }

  auto rigCam = ArcRotateCamera::New(iName, alpha + alphaShift, beta, radius,
                                     _target, getScene());
  return rigCam;
}

void ArcRotateCamera::_updateRigCameras()
{
  auto camLeft  = dynamic_cast<ArcRotateCamera*>(_rigCameras[0]);
  auto camRight = dynamic_cast<ArcRotateCamera*>(_rigCameras[1]);

  camLeft->beta = camRight->beta = beta;
  camLeft->radius = camRight->radius = radius;

  switch (cameraRigMode) {
    case Camera::RIG_MODE_STEREOSCOPIC_ANAGLYPH:
    case Camera::RIG_MODE_STEREOSCOPIC_SIDEBYSIDE_PARALLEL:
    case Camera::RIG_MODE_STEREOSCOPIC_OVERUNDER:
    case Camera::RIG_MODE_VR:
      camLeft->alpha  = alpha - _cameraRigParams.stereoHalfAngle;
      camRight->alpha = alpha + _cameraRigParams.stereoHalfAngle;
      break;
    case Camera::RIG_MODE_STEREOSCOPIC_SIDEBYSIDE_CROSSEYED:
      camLeft->alpha  = alpha + _cameraRigParams.stereoHalfAngle;
      camRight->alpha = alpha - _cameraRigParams.stereoHalfAngle;
      break;
    default:
      break;
  }
  TargetCamera::_updateRigCameras();
}

void ArcRotateCamera::dispose(bool doNotRecurse)
{
  inputs->clear();
  TargetCamera::dispose(doNotRecurse);
}

const char* ArcRotateCamera::getClassName() const
{
  return "ArcRotateCamera";
}

Json::object ArcRotateCamera::serialize() const
{
  return Json::object();
}

} // end of namespace BABYLON
