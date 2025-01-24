#ifndef IMGUI_RESIN_MODALS_HPP
#define IMGUI_RESIN_MODALS_HPP
#include <string_view>

namespace ImGui {

namespace resin {

void OpenModal(std::string_view modal_name);

///
/// Returns true when closed and name is changed.
///
bool RenameModal(std::string_view modal_name, std::string& name_holder);

}  // namespace resin

}  // namespace ImGui

#endif
