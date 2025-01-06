#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree_registry.hpp>

namespace resin {
MaterialSDFTreeComponent::MaterialSDFTreeComponent(SDFTreeRegistry& tree)
    : material(glm::vec3(1.F)),
      mat_id_(tree.materials_registry),
      name_(std::format("Material {}", tree.material_index++)) {}

MaterialSDFTreeComponent::MaterialSDFTreeComponent(SDFTreeRegistry& tree, Material mat)
    : material(mat), mat_id_(tree.materials_registry), name_(std::format("Material {}", tree.material_index++)) {}

}  // namespace resin
