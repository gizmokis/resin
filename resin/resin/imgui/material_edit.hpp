#ifndef RESIN_MATERIAL_EDIT_HPP
#define RESIN_MATERIAL_EDIT_HPP

#include <imgui/imgui.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/shader.hpp>

namespace ImGui {

namespace resin {

using MaterialFramebuffers =
    std::unordered_map<::resin::IdView<::resin::MaterialId>, std::unique_ptr<::resin::ImageFramebuffer>,
                       ::resin::IdViewHash<::resin::MaterialId>, std::equal_to<>>;

std::optional<::resin::IdView<::resin::MaterialId>> MaterialsList(
    MaterialFramebuffers& material_view_framebuffers, const ::resin::RenderingShaderProgram& material_image_shader,
    const ::resin::SDFTree& sdf_tree, const std::optional<::resin::IdView<::resin::MaterialId>>& selected_old,
    size_t img_size = 64);

bool MaterialEdit(::resin::ImageFramebuffer& framebuffer, const ::resin::RenderingShaderProgram& material_image_shader,
                  ::resin::MaterialSDFTreeComponent& mat, size_t img_size = 128);

}  // namespace resin

}  // namespace ImGui

#endif
