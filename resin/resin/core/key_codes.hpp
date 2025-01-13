#ifndef RESIN_KEY_CODES_HPP
#define RESIN_KEY_CODES_HPP

#include <GLFW/glfw3.h>

#include <cstdint>
#include <libresin/utils/enum_mapper.hpp>

namespace resin {

namespace key {

enum class Code : uint8_t {
  // Letters
  A,
  B,
  C,
  D,
  E,
  F,
  G,
  H,
  I,
  J,
  K,
  L,
  M,
  N,
  O,
  P,
  Q,
  R,
  S,
  T,
  U,
  V,
  W,
  X,
  Y,
  Z,

  // Numbers
  Num0,
  Num1,
  Num2,
  Num3,
  Num4,
  Num5,
  Num6,
  Num7,
  Num8,
  Num9,

  // Function s
  F1,
  F2,
  F3,
  F4,
  F5,
  F6,
  F7,
  F8,
  F9,
  F10,
  F11,
  F12,

  // Arrow s
  ArrowUp,
  ArrowDown,
  ArrowLeft,
  ArrowRight,

  // Special s
  Escape,
  Enter,
  Backspace,
  Tab,
  Space,
  LeftShift,
  RightShift,
  LeftControl,
  RightControl,
  LeftAlt,
  RightAlt,

  // Additional s
  Insert,
  Delete,
  Home,
  End,
  PageUp,
  PageDown,

  // Punctuation and symbols
  Apostrophe,
  Comma,
  Minus,
  Period,
  Slash,
  Semicolon,
  Equal,
  LeftBracket,
  RightBracket,
  Backslash,
  GraveAccent,

  _Count,  // NOLINT
};

constexpr StringEnumMapper<Code> kKeyCodeNames({
    // Letters
    {Code::A, "A"},
    {Code::B, "B"},
    {Code::C, "C"},
    {Code::D, "D"},
    {Code::E, "E"},
    {Code::F, "F"},
    {Code::G, "G"},
    {Code::H, "H"},
    {Code::I, "I"},
    {Code::J, "J"},
    {Code::K, "K"},
    {Code::L, "L"},
    {Code::M, "M"},
    {Code::N, "N"},
    {Code::O, "O"},
    {Code::P, "P"},
    {Code::Q, "Q"},
    {Code::R, "R"},
    {Code::S, "S"},
    {Code::T, "T"},
    {Code::U, "U"},
    {Code::V, "V"},
    {Code::W, "W"},
    {Code::X, "X"},
    {Code::Y, "Y"},
    {Code::Z, "Z"},

    // Numbers
    {Code::Num0, "Num0"},
    {Code::Num1, "Num1"},
    {Code::Num2, "Num2"},
    {Code::Num3, "Num3"},
    {Code::Num4, "Num4"},
    {Code::Num5, "Num5"},
    {Code::Num6, "Num6"},
    {Code::Num7, "Num7"},
    {Code::Num8, "Num8"},
    {Code::Num9, "Num9"},

    // Function s
    {Code::F1, "F1"},
    {Code::F2, "F2"},
    {Code::F3, "F3"},
    {Code::F4, "F4"},
    {Code::F5, "F5"},
    {Code::F6, "F6"},
    {Code::F7, "F7"},
    {Code::F8, "F8"},
    {Code::F9, "F9"},
    {Code::F10, "F10"},
    {Code::F11, "F11"},
    {Code::F12, "F12"},

    // Arrow s
    {Code::ArrowUp, "ArrowUp"},
    {Code::ArrowDown, "ArrowDown"},
    {Code::ArrowLeft, "ArrowLeft"},
    {Code::ArrowRight, "ArrowRight"},

    // Special s
    {Code::Escape, "Escape"},
    {Code::Enter, "Enter"},
    {Code::Backspace, "Backspace"},
    {Code::Tab, "Tab"},
    {Code::Space, "Space"},
    {Code::LeftShift, "LeftShift"},
    {Code::RightShift, "RightShift"},
    {Code::LeftControl, "LeftControl"},
    {Code::RightControl, "RightControl"},
    {Code::LeftAlt, "LeftAlt"},
    {Code::RightAlt, "RightAlt"},

    // Additional s
    {Code::Insert, "Insert"},
    {Code::Delete, "Delete"},
    {Code::Home, "Home"},
    {Code::End, "End"},
    {Code::PageUp, "PageUp"},
    {Code::PageDown, "PageDown"},

    // Punctuation and symbols
    {Code::Apostrophe, "Apostrophe"},
    {Code::Comma, "Comma"},
    {Code::Minus, "Minus"},
    {Code::Period, "Period"},
    {Code::Slash, "Slash"},
    {Code::Semicolon, "Semicolon"},
    {Code::Equal, "Equal"},
    {Code::LeftBracket, "LeftBracket"},
    {Code::RightBracket, "RightBracket"},
    {Code::Backslash, "Backslash"},
    {Code::GraveAccent, "GraveAccent"},
});

constexpr EnumMapper<Code, int> kCodeGLFWMapping({
    // Letters
    {Code::A, GLFW_KEY_A},
    {Code::B, GLFW_KEY_B},
    {Code::C, GLFW_KEY_C},
    {Code::D, GLFW_KEY_D},
    {Code::E, GLFW_KEY_E},
    {Code::F, GLFW_KEY_F},
    {Code::G, GLFW_KEY_G},
    {Code::H, GLFW_KEY_H},
    {Code::I, GLFW_KEY_I},
    {Code::J, GLFW_KEY_J},
    {Code::K, GLFW_KEY_K},
    {Code::L, GLFW_KEY_L},
    {Code::M, GLFW_KEY_M},
    {Code::N, GLFW_KEY_N},
    {Code::O, GLFW_KEY_O},
    {Code::P, GLFW_KEY_P},
    {Code::Q, GLFW_KEY_Q},
    {Code::R, GLFW_KEY_R},
    {Code::S, GLFW_KEY_S},
    {Code::T, GLFW_KEY_T},
    {Code::U, GLFW_KEY_U},
    {Code::V, GLFW_KEY_V},
    {Code::W, GLFW_KEY_W},
    {Code::X, GLFW_KEY_X},
    {Code::Y, GLFW_KEY_Y},
    {Code::Z, GLFW_KEY_Z},

    // Numbers
    {Code::Num0, GLFW_KEY_0},
    {Code::Num1, GLFW_KEY_1},
    {Code::Num2, GLFW_KEY_2},
    {Code::Num3, GLFW_KEY_3},
    {Code::Num4, GLFW_KEY_4},
    {Code::Num5, GLFW_KEY_5},
    {Code::Num6, GLFW_KEY_6},
    {Code::Num7, GLFW_KEY_7},
    {Code::Num8, GLFW_KEY_8},
    {Code::Num9, GLFW_KEY_9},

    // Function s
    {Code::F1, GLFW_KEY_F1},
    {Code::F2, GLFW_KEY_F2},
    {Code::F3, GLFW_KEY_F3},
    {Code::F4, GLFW_KEY_F4},
    {Code::F5, GLFW_KEY_F5},
    {Code::F6, GLFW_KEY_F6},
    {Code::F7, GLFW_KEY_F7},
    {Code::F8, GLFW_KEY_F8},
    {Code::F9, GLFW_KEY_F9},
    {Code::F10, GLFW_KEY_F10},
    {Code::F11, GLFW_KEY_F11},
    {Code::F12, GLFW_KEY_F12},

    // Arrow s
    {Code::ArrowUp, GLFW_KEY_UP},
    {Code::ArrowDown, GLFW_KEY_DOWN},
    {Code::ArrowLeft, GLFW_KEY_LEFT},
    {Code::ArrowRight, GLFW_KEY_RIGHT},

    // Special s
    {Code::Escape, GLFW_KEY_ESCAPE},
    {Code::Enter, GLFW_KEY_ENTER},
    {Code::Backspace, GLFW_KEY_BACKSPACE},
    {Code::Tab, GLFW_KEY_TAB},
    {Code::Space, GLFW_KEY_SPACE},
    {Code::LeftShift, GLFW_KEY_LEFT_SHIFT},
    {Code::RightShift, GLFW_KEY_RIGHT_SHIFT},
    {Code::LeftControl, GLFW_KEY_LEFT_CONTROL},
    {Code::RightControl, GLFW_KEY_RIGHT_CONTROL},
    {Code::LeftAlt, GLFW_KEY_LEFT_ALT},
    {Code::RightAlt, GLFW_KEY_RIGHT_ALT},

    // Additional s
    {Code::Insert, GLFW_KEY_INSERT},
    {Code::Delete, GLFW_KEY_DELETE},
    {Code::Home, GLFW_KEY_HOME},
    {Code::End, GLFW_KEY_END},
    {Code::PageUp, GLFW_KEY_PAGE_UP},
    {Code::PageDown, GLFW_KEY_PAGE_DOWN},

    // Punctuation and symbols
    {Code::Apostrophe, GLFW_KEY_APOSTROPHE},
    {Code::Comma, GLFW_KEY_COMMA},
    {Code::Minus, GLFW_KEY_MINUS},
    {Code::Period, GLFW_KEY_PERIOD},
    {Code::Slash, GLFW_KEY_SLASH},
    {Code::Semicolon, GLFW_KEY_SEMICOLON},
    {Code::Equal, GLFW_KEY_EQUAL},
    {Code::LeftBracket, GLFW_KEY_LEFT_BRACKET},
    {Code::RightBracket, GLFW_KEY_RIGHT_BRACKET},
    {Code::Backslash, GLFW_KEY_BACKSLASH},
    {Code::GraveAccent, GLFW_KEY_GRAVE_ACCENT},
});

// TODO(SDF-134)
inline std::optional<Code> GLFWKeyToCode(int key) {
  switch (key) {
    // Letters
    case GLFW_KEY_A:
      return Code::A;
    case GLFW_KEY_B:
      return Code::B;
    case GLFW_KEY_C:
      return Code::C;
    case GLFW_KEY_D:
      return Code::D;
    case GLFW_KEY_E:
      return Code::E;
    case GLFW_KEY_F:
      return Code::F;
    case GLFW_KEY_G:
      return Code::G;
    case GLFW_KEY_H:
      return Code::H;
    case GLFW_KEY_I:
      return Code::I;
    case GLFW_KEY_J:
      return Code::J;
    case GLFW_KEY_K:
      return Code::K;
    case GLFW_KEY_L:
      return Code::L;
    case GLFW_KEY_M:
      return Code::M;
    case GLFW_KEY_N:
      return Code::N;
    case GLFW_KEY_O:
      return Code::O;
    case GLFW_KEY_P:
      return Code::P;
    case GLFW_KEY_Q:
      return Code::Q;
    case GLFW_KEY_R:
      return Code::R;
    case GLFW_KEY_S:
      return Code::S;
    case GLFW_KEY_T:
      return Code::T;
    case GLFW_KEY_U:
      return Code::U;
    case GLFW_KEY_V:
      return Code::V;
    case GLFW_KEY_W:
      return Code::W;
    case GLFW_KEY_X:
      return Code::X;
    case GLFW_KEY_Y:
      return Code::Y;
    case GLFW_KEY_Z:
      return Code::Z;

    // Numbers
    case GLFW_KEY_0:
      return Code::Num0;
    case GLFW_KEY_1:
      return Code::Num1;
    case GLFW_KEY_2:
      return Code::Num2;
    case GLFW_KEY_3:
      return Code::Num3;
    case GLFW_KEY_4:
      return Code::Num4;
    case GLFW_KEY_5:
      return Code::Num5;
    case GLFW_KEY_6:
      return Code::Num6;
    case GLFW_KEY_7:
      return Code::Num7;
    case GLFW_KEY_8:
      return Code::Num8;
    case GLFW_KEY_9:
      return Code::Num9;

    // Function s
    case GLFW_KEY_F1:
      return Code::F1;
    case GLFW_KEY_F2:
      return Code::F2;
    case GLFW_KEY_F3:
      return Code::F3;
    case GLFW_KEY_F4:
      return Code::F4;
    case GLFW_KEY_F5:
      return Code::F5;
    case GLFW_KEY_F6:
      return Code::F6;
    case GLFW_KEY_F7:
      return Code::F7;
    case GLFW_KEY_F8:
      return Code::F8;
    case GLFW_KEY_F9:
      return Code::F9;
    case GLFW_KEY_F10:
      return Code::F10;
    case GLFW_KEY_F11:
      return Code::F11;
    case GLFW_KEY_F12:
      return Code::F12;

    // Arrow s
    case GLFW_KEY_UP:
      return Code::ArrowUp;
    case GLFW_KEY_DOWN:
      return Code::ArrowDown;
    case GLFW_KEY_LEFT:
      return Code::ArrowLeft;
    case GLFW_KEY_RIGHT:
      return Code::ArrowRight;

    // Special s
    case GLFW_KEY_ESCAPE:
      return Code::Escape;
    case GLFW_KEY_ENTER:
      return Code::Enter;
    case GLFW_KEY_BACKSPACE:
      return Code::Backspace;
    case GLFW_KEY_TAB:
      return Code::Tab;
    case GLFW_KEY_SPACE:
      return Code::Space;
    case GLFW_KEY_LEFT_SHIFT:
      return Code::LeftShift;
    case GLFW_KEY_RIGHT_SHIFT:
      return Code::RightShift;
    case GLFW_KEY_LEFT_CONTROL:
      return Code::LeftControl;
    case GLFW_KEY_RIGHT_CONTROL:
      return Code::RightControl;
    case GLFW_KEY_LEFT_ALT:
      return Code::LeftAlt;
    case GLFW_KEY_RIGHT_ALT:
      return Code::RightAlt;

    // Additional s
    case GLFW_KEY_INSERT:
      return Code::Insert;
    case GLFW_KEY_DELETE:
      return Code::Delete;
    case GLFW_KEY_HOME:
      return Code::Home;
    case GLFW_KEY_END:
      return Code::End;
    case GLFW_KEY_PAGE_UP:
      return Code::PageUp;
    case GLFW_KEY_PAGE_DOWN:
      return Code::PageDown;

    // Punctuation and symbols
    case GLFW_KEY_APOSTROPHE:
      return Code::Apostrophe;
    case GLFW_KEY_COMMA:
      return Code::Comma;
    case GLFW_KEY_MINUS:
      return Code::Minus;
    case GLFW_KEY_PERIOD:
      return Code::Period;
    case GLFW_KEY_SLASH:
      return Code::Slash;
    case GLFW_KEY_SEMICOLON:
      return Code::Semicolon;
    case GLFW_KEY_EQUAL:
      return Code::Equal;
    case GLFW_KEY_LEFT_BRACKET:
      return Code::LeftBracket;
    case GLFW_KEY_RIGHT_BRACKET:
      return Code::RightBracket;
    case GLFW_KEY_BACKSLASH:
      return Code::Backslash;
    case GLFW_KEY_GRAVE_ACCENT:
      return Code::GraveAccent;

    default:
      return std::nullopt;
  }
}

}  // namespace key

}  // namespace resin

#endif  // RESIN_MOUSE_CODES_HPP
