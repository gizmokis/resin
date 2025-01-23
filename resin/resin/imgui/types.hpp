#ifndef IMGUI_RESIN_TYPES_HPP
#define IMGUI_RESIN_TYPES_HPP

#include <libresin/core/framebuffer.hpp>
#include <libresin/core/id_registry.hpp>
#include <libresin/core/material.hpp>
#include <utility>

namespace ImGui {  // NOLINT

namespace resin {

struct LazyImageFramebuffer {
  explicit LazyImageFramebuffer(::resin::ImageFramebuffer&& _fb) : fb(std::move(_fb)) {}

  inline void mark_dirty() { is_dirty = true; }
  inline void mark_clean() { is_dirty = false; }

  ::resin::ImageFramebuffer fb;
  bool is_dirty{};
};

}  // namespace resin

}  // namespace ImGui

#endif
