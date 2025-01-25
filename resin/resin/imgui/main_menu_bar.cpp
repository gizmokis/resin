#include <imgui/imgui.h>

#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/primitive_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/json.hpp>
#include <resin/dialog/file_dialog.hpp>
#include <resin/imgui/main_menu_bar.hpp>

#include "resin/imgui/modals.hpp"

namespace ImGui {
namespace resin {

static const std::array<::resin::FileDialog::FilterItem, 1> kPrefabFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin prefab", "amber")};

static const std::array<::resin::FileDialog::FilterItem, 1> kResinFiltersArray = {
    ::resin::FileDialog::FilterItem("Resin project", "resin")};

void MainMenuBar(LazyMaterialImageFramebuffers& material_img_fbs, ::resin::Scene& scene) {
  bool open_new_modal   = false;
  bool open_scene_modal = false;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::MenuItem("New")) {
      open_new_modal = true;
    }
    if (ImGui::MenuItem("Open")) {
      open_scene_modal = true;
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
            scene.tree().root().rename(std::format("{}", path.stem().string()));
            file << ::resin::json::serialize_scene(scene);
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
            scene.tree().root().rename(std::format("{}", path.stem().string()));
            file << ::resin::json::serialize_prefab(scene.tree(), scene.tree().root().node_id());
            ::resin::Logger::info("Saved prefab to {}", path.string());
          },
          std::span<const ::resin::FileDialog::FilterItem>(kPrefabFiltersArray), "Scene Prefab.amber");
    }

    ImGui::EndMainMenuBar();
  }

  if (open_new_modal) {
    OpenModal("New Scene");
  }

  if (MessageOkCancelModal("New Scene",
                           "Are you sure you want to create a new scene? Any unsaved progress will be lost.", "Create",
                           "Cancel")) {
    scene.set_default();
    material_img_fbs.reset();
  }

  if (open_scene_modal) {
    OpenModal("Open Scene");
  }

  if (MessageOkCancelModal("Open Scene", "Are you sure you want to open a scene? Any unsaved progress will be lost.",
                           "Open", "Cancel")) {
    ::resin::FileDialog::instance().open_file(
        [&scene, &material_img_fbs](const std::filesystem::path& path) {
          std::string json_content;
          std::ifstream file(path);
          if (!file.is_open()) {
            ::resin::Logger::err("Could not open a file with path {}", path.string());
            return;
          }

          std::ostringstream ss;
          ss << file.rdbuf();
          json_content = ss.str();

          ::resin::json::deserialize_scene(scene, json_content);
          ::resin::Logger::info("Loaded scene from {}", path.string());
          material_img_fbs.reset();
        },
        std::span<const ::resin::FileDialog::FilterItem>(kResinFiltersArray));
  }
}

}  // namespace resin

}  // namespace ImGui
