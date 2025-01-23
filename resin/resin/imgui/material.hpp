#ifndef IMGUI_RESIN_MATERIAL_HPP
#define IMGUI_RESIN_MATERIAL_HPP

#include <imgui/imgui.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/shader.hpp>
#include <resin/imgui/types.hpp>

namespace ImGui {

namespace resin {

using MaterialPreviewImageFramebuffersMap =
    std::unordered_map<::resin::IdView<::resin::MaterialId>, std::unique_ptr<LazyImageFramebuffer>,
                       ::resin::IdViewHash<::resin::MaterialId>, std::equal_to<>>;

struct LazyMaterialImageFramebuffers {
  explicit LazyMaterialImageFramebuffers(size_t node_material_preview_img_size, size_t main_material_img_size,
                                         size_t _material_preview_img_size)
      : node_material_preview_fb(LazyImageFramebuffer(
            ::resin::ImageFramebuffer(node_material_preview_img_size, node_material_preview_img_size))),
        main_material_fb(::resin::ImageFramebuffer(main_material_img_size, main_material_img_size)),
        material_preview_img_size(_material_preview_img_size) {}

  LazyImageFramebuffer node_material_preview_fb;
  std::optional<::resin::IdView<::resin::MaterialId>> node_material_preview_id;
  LazyImageFramebuffer main_material_fb;
  std::optional<::resin::IdView<::resin::MaterialId>> main_material_id;
  MaterialPreviewImageFramebuffersMap material_preview_fbs_map;
  size_t material_preview_img_size;
};

bool MaterialsList(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                   LazyMaterialImageFramebuffers& material_img_fbs, const ::resin::SDFTree& sdf_tree);

bool MaterialsListEdit(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                       LazyMaterialImageFramebuffers& material_img_fbs, ::resin::SDFTree& sdf_tree);

bool MaterialEdit(std::optional<::resin::IdView<::resin::MaterialId>>& selected,
                  LazyMaterialImageFramebuffers& material_img_fbs, ::resin::SDFTree& sdf_tree);

}  // namespace resin

}  // namespace ImGui

#endif
