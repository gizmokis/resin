#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <resin/imgui/material_edit.hpp>

namespace ImGui {  // NOLINT

namespace resin {

bool MaterialEdit(::resin::MaterialSDFTreeComponent& mat) {
  ImGui::ColorEdit3("Color", glm::value_ptr(mat.material.albedo));
  ImGui::DragFloat("Ambient", &mat.material.ambientFactor, 0.01F, 0.0F, 1.0F, "%.2f");
  ImGui::DragFloat("Diffuse", &mat.material.diffuseFactor, 0.01F, 0.0F, 1.0F, "%.2f");
  ImGui::DragFloat("Specular", &mat.material.specularFactor, 0.01F, 0.0F, 1.0F, "%.2f");
  ImGui::DragFloat("Exponent", &mat.material.specularExponent, 0.1F, 0.0F, 100.0F, "%.1f");

  return true;
}

}  // namespace resin

}  // namespace ImGui
