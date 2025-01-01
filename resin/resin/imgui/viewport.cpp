#include <imgui/imgui.h>

#include <cstddef>
#include <resin/imgui/viewport.hpp>

namespace ImGui {

namespace resin {

bool handle_resize(::resin::Framebuffer& framebuffer, bool& resized) {
  ImVec2 view = ImGui::GetContentRegionAvail();
  auto width  = static_cast<size_t>(view.x);
  auto height = static_cast<size_t>(view.y);

  if (width != framebuffer.width() || height != framebuffer.height()) {
    resized = true;

    if (width == 0 || height == 0) {
      return false;
    }

    framebuffer.resize(width, height);

    return true;
  }

  return true;
}

bool Viewport(::resin::Framebuffer& framebuffer) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::Begin("Viewport");

  bool resized = false;
  if (!handle_resize(framebuffer, resized)) {
    ImGui::End();
    ImGui::PopStyleVar();
    return resized;
  }

  ImGui::Image((ImTextureID)(intptr_t)framebuffer.color_texture(),  // NOLINT
               ImVec2(static_cast<float>(framebuffer.width()), static_cast<float>(framebuffer.height())), ImVec2(0, 1),
               ImVec2(1, 0));

  ImGui::End();
  ImGui::PopStyleVar();
  return resized;
}

}  // namespace resin

}  // namespace ImGui