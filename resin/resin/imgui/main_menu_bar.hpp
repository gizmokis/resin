#ifndef IMGUI_RESIN_MAIN_MENU_BAR_HPP
#define IMGUI_RESIN_MAIN_MENU_BAR_HPP
#include <libresin/core/sdf_tree/sdf_tree.hpp>

namespace resin {
class Scene;
}

namespace ImGui {
namespace resin {

void MainMenuBar(::resin::Scene& scene);

}  // namespace resin

}  // namespace ImGui

#endif
