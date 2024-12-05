#ifndef RESIN_LIGHT_HPP
#define RESIN_LIGHT_HPP

#include <libresin/core/transform.hpp>

namespace resin {

struct BaseLight {
  explicit BaseLight(const glm::vec3& color) : color(color) {}

  Transform transform;
  glm::vec3 color;
};

struct DirectionalLight : public BaseLight {
  DirectionalLight(const glm::vec3& color, float ambient_impact) : BaseLight(color), ambient_impact(ambient_impact) {}

  float ambient_impact;
};

struct PointLight : public BaseLight {
  struct Attenuation {
    Attenuation(float constant, float linear, float quadratic)
        : constant(constant), linear(linear), quadratic(quadratic) {}

    float constant, linear, quadratic;
  };

  PointLight(const glm::vec3& color, const glm::vec3& pos, const Attenuation& attenuation)
      : BaseLight(color), attenuation(attenuation) {
    transform.set_local_pos(pos);
  }

  Attenuation attenuation;
};

}  // namespace resin

#endif  // RESIN_LIGHT_HPP