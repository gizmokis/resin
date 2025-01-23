#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree_registry.hpp>

namespace resin {
MaterialSDFTreeComponent::MaterialSDFTreeComponent(SDFTreeRegistry& tree)
    : material(glm::vec3(1.F)),
      mat_id_(tree.materials_registry),
      name_(std::format("Material {}", tree.material_index++)),
      tree_registry_(tree) {
  mark_dirty();
}

MaterialSDFTreeComponent::MaterialSDFTreeComponent(SDFTreeRegistry& tree, Material mat)
    : material(mat),
      mat_id_(tree.materials_registry),
      name_(std::format("Material {}", tree.material_index++)),
      tree_registry_(tree) {
  mark_dirty();
}

void MaterialSDFTreeComponent::mark_dirty() { tree_registry_.dirty_materials.emplace(material_id()); }

bool MaterialSDFTreeComponent::is_dirty() const {
  return tree_registry_.dirty_materials.find(material_id()) != tree_registry_.dirty_materials.end();
}

}  // namespace resin
