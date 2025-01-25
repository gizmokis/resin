#include <imgui/imgui.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/utils/json.hpp>
#include <resin/dialog/file_dialog.hpp>
#include <resin/imgui/main_menu_bar.hpp>

namespace ImGui {
namespace resin {

static const std::array<::resin::FileDialog::FilterItem, 1> kPrefabFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin prefab", "amber")};

static const std::array<::resin::FileDialog::FilterItem, 1> kResinFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin project", "resin")};

void MainMenuBar(std::unique_ptr<::resin::Scene>& scene) {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::MenuItem("New")) {
    }
    if (ImGui::MenuItem("Open")) {
      ::resin::FileDialog::instance().open_file(
          [&scene](const std::filesystem::path& path) {
            std::string json_content;
            std::ifstream file(path);
            if (!file.is_open()) {
              ::resin::Logger::err("Could not open a file with path {}", path.string());
              return;
            }

            std::ostringstream ss;
            ss << file.rdbuf();
            json_content = ss.str();

            scene = ::resin::json::deserialize_scene(json_content);
            ::resin::Logger::info("Loaded scene from {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kResinFiltersArray));
    }
    if (ImGui::MenuItem("Save As...")) {
      ::resin::FileDialog::instance().save_file(
          [&scene](const std::filesystem::path& path) {
            std::ofstream file(path);
            if (!file.is_open()) {
              ::resin::Logger::warn("Could not save to path {}", path.string());
              ::resin::log_throw(::resin::FileStreamNotAvailableException(path.string()));
              return;
            }
            scene->tree().root().rename(std::format("{}", path.stem().string()));
            file << ::resin::json::serialize_scene(*scene);
            ::resin::Logger::info("Saved prefab to {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kResinFiltersArray), "Scene.resin");
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
            scene->tree().root().rename(std::format("{}", path.stem().string()));
            file << ::resin::json::serialize_prefab(scene->tree(), scene->tree().root().node_id());
            ::resin::Logger::info("Saved prefab to {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray), "Scene Prefab.amber");
    }

    ImGui::EndMainMenuBar();
  }
}

}  // namespace resin

}  // namespace ImGui
