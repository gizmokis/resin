#include <imgui/imgui.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/utils/json.hpp>
#include <resin/dialog/file_dialog.hpp>
#include <resin/imgui/main_menu_bar.hpp>

namespace ImGui {
namespace resin {

static const std::array<::resin::FileDialog::FilterItem, 1> kPrefabFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin prefab", "rsnpfb")};

static const std::array<::resin::FileDialog::FilterItem, 1> kResinFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin project", "resin")};

void MainMenuBar(::resin::Scene& scene) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("New")) {
      }
      if (ImGui::MenuItem("Open")) {
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Save As...")) {
        ::resin::FileDialog::instance().save_file(
            [&scene](const std::filesystem::path& path) {
              std::ofstream file(path);
              if (!file.is_open()) {
                ::resin::Logger::warn("Could not save to path {}", path.string());
                ::resin::log_throw(::resin::FileStreamNotAvailableException(path.string()));
                return;
              }
              scene.tree().root().rename(std::format("{}", path.stem().string()));
              file << ::resin::json::serialize_scene(scene);
              ::resin::Logger::info("Saved prefab to {}", path.string());
            },
            std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray), "Scene.resin");
      }
      if (ImGui::MenuItem("Export as prefab...")) {
        ::resin::FileDialog::instance().save_file(
            [&scene](const std::filesystem::path& path) {
              std::ofstream file(path);
              if (!file.is_open()) {
                ::resin::Logger::warn("Could not save to path {}", path.string());
                ::resin::log_throw(::resin::FileStreamNotAvailableException(path.string()));
                return;
              }
              scene.tree().root().rename(std::format("{}", path.stem().string()));
              file << ::resin::json::serialize_prefab(scene.tree(), scene.tree().root().node_id());
              ::resin::Logger::info("Saved prefab to {}", path.string());
            },
            std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray), "Scene Prefab.rsnpfb");
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }
}

}  // namespace resin

}  // namespace ImGui
