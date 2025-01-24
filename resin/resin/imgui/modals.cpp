#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <resin/imgui/modals.hpp>
#include <string>

namespace ImGui {

namespace resin {
static bool modal_start = false;  // NOLINT

void OpenModal(std::string_view modal_name) {
  ImGui::OpenPopup(modal_name.data());
  modal_start = true;
}

bool RenameModal(std::string_view modal_name, std::string& name_holder) {
  static bool init_selection = false;
  bool result                = false;

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0F, 10.0F});
  if (ImGui::BeginPopupModal(modal_name.data(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    if (modal_start) {
      ImGui::SetKeyboardFocusHere();
      init_selection = true;
      modal_start    = false;
    }

    ImGui::PushItemWidth(250.0F);
    if (ImGui::InputText("##Name", &name_holder) || ImGui::IsKeyDown(ImGuiKey_LeftArrow) ||
        ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
      init_selection = false;
    }

    if (ImGui::IsItemDeactivatedAfterEdit() && ImGui::IsKeyDown(ImGuiKey_Enter)) {
      if (ImGui::IsKeyDown(ImGuiKey_Enter)) {
        if (!name_holder.empty()) {
          ImGui::CloseCurrentPopup();
          result = true;
        }
      }
    }

    ImGui::PopItemWidth();
    if (init_selection) {
      if (ImGuiInputTextState * state{ImGui::GetInputTextState(ImGui::GetItemID())}) {
        state->ReloadUserBufAndSelectAll();
      }
    }

    if (name_holder.empty()) {
      ImGui::BeginDisabled();
    }

    ImGui::SetItemDefaultFocus();
    if (ImGui::Button("Rename", ImVec2(120, 0))) {
      if (!name_holder.empty()) {
        ImGui::CloseCurrentPopup();
        result = true;
      }
    }

    if (name_holder.empty()) {
      ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
  return result;
}

}  // namespace resin

}  // namespace ImGui
