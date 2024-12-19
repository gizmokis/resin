#ifndef RESIN_MATERIAL_HPP
#define RESIN_MATERIAL_HPP

#include <glm/glm.hpp>
#include <libresin/core/id_registry.hpp>

namespace resin {

struct Material;
using MaterialId = Id<Material>;

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

struct SDFTreeRegistry;

class MaterialSDFTreeComponent {
 public:
  MaterialSDFTreeComponent() = delete;

  explicit MaterialSDFTreeComponent(SDFTreeRegistry& tree);
  MaterialSDFTreeComponent(SDFTreeRegistry& tree, Material mat);

  MaterialSDFTreeComponent(const MaterialSDFTreeComponent&)            = delete;
  MaterialSDFTreeComponent& operator=(const MaterialSDFTreeComponent&) = delete;

  MaterialSDFTreeComponent(MaterialSDFTreeComponent&&)                  = default;
  MaterialSDFTreeComponent& operator=(MaterialSDFTreeComponent&& other) = default;

  inline IdView<MaterialId> mat_id() const { return mat_id_; }
  inline void rename(std::string&& new_name) { name_ = std::move(new_name); }
  inline std::string_view name() { return name_; }

 public:
  Material material;

 private:
  MaterialId mat_id_;
  std::string name_;
};

}  // namespace resin

#endif  // RESIN_MATERIAL_HPP
