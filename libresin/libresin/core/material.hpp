#ifndef RESIN_MATERIAL_HPP
#define RESIN_MATERIAL_HPP

#include <glm/glm.hpp>

namespace resin {

struct Material {
  explicit Material(const glm::vec3& color, float ambientFactor = 0.5F, float diffuseFactor = 0.5F,
                    float specularFactor = 0.5F, float specularExponent = 50.F)
      : albedo(color),
        ambientFactor(ambientFactor),
        diffuseFactor(diffuseFactor),
        specularFactor(specularFactor),
        specularExponent(specularExponent) {}

  glm::vec3 albedo;

  float ambientFactor;
  float diffuseFactor;
  float specularFactor;
  float specularExponent;
};

}  // namespace resin

#endif  // RESIN_MATERIAL_HPP