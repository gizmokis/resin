#ifndef RESIN_MOUSE_CODES_HPP
#define RESIN_MOUSE_CODES_HPP

#include <GLFW/glfw3.h>

#include <cstdint>
#include <libresin/utils/enum_mapper.hpp>

namespace resin {

namespace mouse {

enum class Code : uint8_t {
  MouseButtonLeft,
  MouseButtonRight,
  MouseButtonMiddle,
  MouseButton4,
  MouseButton5,
  MouseButton6,
  MouseButton7,
  MouseButton8,

  _Count,  // NOLINT

  MouseButton1 = MouseButtonLeft,
  MouseButton2 = MouseButtonRight,
  MouseButton3 = MouseButtonMiddle
};

constexpr StringEnumMapper<Code> kMouseCodeNames({
    {Code::MouseButtonLeft, "MouseButtonLeft"},      //
    {Code::MouseButtonRight, "MouseButtonRight"},    //
    {Code::MouseButtonMiddle, "MouseButtonMiddle"},  //
    {Code::MouseButton4, "MouseButton4"},            //
    {Code::MouseButton5, "MouseButton5"},            //
    {Code::MouseButton6, "MouseButton6"},            //
    {Code::MouseButton7, "MouseButton7"},            //
    {Code::MouseButton8, "MouseButton8"}             //
});

constexpr EnumMapper<Code, int> kMouseCodeGLFWMapping({
    {Code::MouseButtonLeft, GLFW_MOUSE_BUTTON_1},    //
    {Code::MouseButtonRight, GLFW_MOUSE_BUTTON_2},   //
    {Code::MouseButtonMiddle, GLFW_MOUSE_BUTTON_3},  //
    {Code::MouseButton4, GLFW_MOUSE_BUTTON_4},       //
    {Code::MouseButton5, GLFW_MOUSE_BUTTON_5},       //
    {Code::MouseButton6, GLFW_MOUSE_BUTTON_6},       //
    {Code::MouseButton7, GLFW_MOUSE_BUTTON_7},       //
    {Code::MouseButton8, GLFW_MOUSE_BUTTON_8}        //
});

enum class CursorMode : uint8_t {
  Normal,
  Hidden,
  Disabled,
  Captured,
  _Count  // NOLINT
};

constexpr StringEnumMapper<CursorMode> kMouseCursorModeNames({
    {CursorMode::Normal, "Normal"},      //
    {CursorMode::Hidden, "Hidden"},      //
    {CursorMode::Captured, "Captured"},  //
    {CursorMode::Disabled, "Disabled"},  //
});

constexpr EnumMapper<CursorMode, int> kMouseCursorModeGLFWMapping({
    {CursorMode::Normal, GLFW_CURSOR_NORMAL},      //
    {CursorMode::Hidden, GLFW_CURSOR_HIDDEN},      //
    {CursorMode::Captured, GLFW_CURSOR_CAPTURED},  //
    {CursorMode::Disabled, GLFW_CURSOR_DISABLED},  //
});

}  // namespace mouse

}  // namespace resin

#endif  // RESIN_MOUSE_CODES_HPP
