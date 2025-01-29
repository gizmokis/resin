#ifndef IMGUI_RESIN_MAIN_MENU_BAR_HPP
#define IMGUI_RESIN_MAIN_MENU_BAR_HPP
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <resin/imgui/material.hpp>

namespace resin {
class Scene;
}

namespace ImGui {
namespace resin {

void MainMenuBar(LazyMaterialImageFramebuffers& material_img_fbs, ::resin::Scene& scene);

}  // namespace resin

}  // namespace ImGui

#endif
