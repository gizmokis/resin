#ifndef RESIN_COMPONENTS_HPP
#define RESIN_COMPONENTS_HPP

#include <imgui/imgui.h>

#include <libresin/core/transform.hpp>

namespace ImGui {  // NOLINT

namespace resin {

bool TransformEdit(::resin::Transform* transform);

}

}  // namespace ImGui

#endif  // RESIN_COMPONENTS_HPP