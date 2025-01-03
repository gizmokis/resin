#include <imgui/imgui.h>

#include <cstddef>
#include <resin/imgui/viewport.hpp>

namespace ImGui {

namespace resin {

bool Viewport(::resin::Framebuffer& framebuffer, bool& resized) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  if (!ImGui::Begin("Viewport")) {
    ImGui::PopStyleVar();
    return false;
  }

  ImGui::PopStyleVar();
  ImVec2 view = ImGui::GetContentRegionAvail();
  auto width  = static_cast<size_t>(view.x);
  auto height = static_cast<size_t>(view.y);

  if (width != framebuffer.width() || height != framebuffer.height()) {
    if (width == 0 || height == 0) {
      return false;
    }

    framebuffer.resize(width, height);
    resized = true;
  }

  return true;
}

}  // namespace resin

}  // namespace ImGui