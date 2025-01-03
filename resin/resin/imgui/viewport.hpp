#ifndef RESIN_VIEWPORT_HPP
#define RESIN_VIEWPORT_HPP

#include <imgui/imgui.h>

#include <libresin/core/framebuffer.hpp>

namespace ImGui {  // NOLINT

namespace resin {

bool Viewport(::resin::Framebuffer& framebuffer, bool& resized);

}  // namespace resin

}  // namespace ImGui

#endif  // RESIN_TRANSFORM_EDIT_HPP