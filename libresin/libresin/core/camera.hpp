#ifndef RESIN_CAMERA_HPP
#define RESIN_CAMERA_HPP
#include "transform.hpp"

namespace resin {

class Camera {
public:
  float width;
  float height;
  float nearPlane;
  float farPlane;
  Transform transform;
  bool isOrthographic;

  Camera(float width, float height, float nearPlane, float farPlane, bool isOrthographic);
  virtual ~Camera();

  virtual glm::mat4 viewMatrix() const;
  virtual glm::mat4 inverseViewMatrix() const;
};

class PerspectiveCamera : public Camera {
public:
  float fov;
  float aspectRatio;

  PerspectiveCamera(float fov, float aspectRatio, float nearPlane, float farPlane);

private:
  void updateDimensions();
};

class OrthographicCamera : public Camera {
public:
  OrthographicCamera(float width, float height, float nearPlane, float farPlane);

};

} // resin

#endif // RESIN_CAMERA_HPP
