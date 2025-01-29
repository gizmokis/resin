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

///
/// Returns true when OK button is clicked.
///
bool MessageOkCancelModal(std::string_view modal_name, std::string_view msg, std::string_view ok_button_label = "OK",
                          std::string_view cancel_button_label = "Cancel");

}  // namespace resin

}  // namespace ImGui

#endif
