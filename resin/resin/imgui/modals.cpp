#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_stdlib.h>

#include <libresin/utils/string_views.hpp>
#include <resin/imgui/modals.hpp>
#include <string>

namespace ImGui {

namespace resin {
static bool modal_start = false;  // NOLINT

void OpenModal(std::string_view modal_name) {
  ImGui::OpenPopup(::resin::c_str(modal_name));
  modal_start = true;
}

bool RenameModal(std::string_view modal_name, std::string& name_holder) {
  static bool init_selection = false;
  bool result                = false;

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0F, 10.0F});
  if (ImGui::BeginPopupModal(::resin::c_str(modal_name), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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

bool MessageOkCancelModal(std::string_view modal_name, std::string_view msg, std::string_view ok_button_label,
                          std::string_view cancel_button_label) {
  bool result = false;

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {10.0F, 10.0F});
  if (ImGui::BeginPopupModal(::resin::c_str(modal_name), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("%s", ::resin::c_str(msg));
    if (ImGui::Button(::resin::c_str(ok_button_label), ImVec2(120, 0))) {
      result = true;
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button(::resin::c_str(cancel_button_label), ImVec2(120, 0))) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
  return result;
}

}  // namespace resin

}  // namespace ImGui
