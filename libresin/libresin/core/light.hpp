#ifndef RESIN_LIGHT_HPP
#define RESIN_LIGHT_HPP

#include <libresin/core/transform.hpp>

namespace resin {

struct BaseLight {
  explicit BaseLight(const glm::vec3& _color) : color(_color) {}

  Transform transform;
  glm::vec3 color;
};

struct DirectionalLight : public BaseLight {
  DirectionalLight(const glm::vec3& _color, float _ambient_impact)
      : BaseLight(_color), ambient_impact(_ambient_impact) {}

  float ambient_impact;
};

struct PointLight : public BaseLight {
  struct Attenuation {
    Attenuation(float _constant, float _linear, float _quadratic)
        : constant(_constant), linear(_linear), quadratic(_quadratic) {}

    float constant, linear, quadratic;
  };

  PointLight(const glm::vec3& _color, const glm::vec3& _pos, const Attenuation& _attenuation)
      : BaseLight(_color), attenuation(_attenuation) {
    transform.set_local_pos(_pos);
  }

  Attenuation attenuation;
};

}  // namespace resin

#endif  // RESIN_LIGHT_HPP
