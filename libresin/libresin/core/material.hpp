#ifndef RESIN_MATERIAL_HPP
#define RESIN_MATERIAL_HPP

#include <glm/glm.hpp>

namespace resin {

struct Material {
  explicit Material(const glm::vec3& color, float _ambientFactor = 0.5F, float _diffuseFactor = 0.5F,
                    float _specularFactor = 0.5F, float _specularExponent = 50.F)
      : albedo(color),
        ambientFactor(_ambientFactor),
        diffuseFactor(_diffuseFactor),
        specularFactor(_specularFactor),
        specularExponent(_specularExponent) {}

  glm::vec3 albedo;

  float ambientFactor;
  float diffuseFactor;
  float specularFactor;
  float specularExponent;
};

}  // namespace resin

#endif  // RESIN_MATERIAL_HPP
