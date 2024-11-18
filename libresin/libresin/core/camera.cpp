#include "camera.hpp"

namespace resin {

// Abstract
Camera::Camera(float width, float height, float nearPlane, float farPlane, bool isOrthographic)
    : width(width), height(height), nearPlane(nearPlane), farPlane(farPlane), isOrthographic(isOrthographic) {}

Camera::~Camera() {}
glm::mat4 Camera::viewMatrix() const {
  return transform.world_to_local_matrix();
}
glm::mat4 Camera::inverseViewMatrix() const {
  return transform.local_to_world_matrix();
}

// Perspective
PerspectiveCamera::PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane)
    : Camera(0.0f, 0.0f, nearPlane, farPlane, false), fov(fov), aspectRatio(aspectRatio) {
  updateDimensions();
}

void PerspectiveCamera::updateDimensions() {
  height = 2.0f * nearPlane * glm::tan(glm::radians(fov) * 0.5f);
  width  = height * aspectRatio;
}

// Orthographic
OrthographicCamera::OrthographicCamera(float width, float height, float nearPlane, float farPlane)
    : Camera(width, height, nearPlane, farPlane, true) {}

} // resin