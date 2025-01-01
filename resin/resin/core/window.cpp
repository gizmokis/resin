#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_opengl3_loader.h>

#include <libresin/utils/logger.hpp>
#include <resin/core/graphics_context.hpp>
#include <resin/core/window.hpp>
#include <resin/event/event.hpp>
#include <resin/event/window_events.hpp>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

namespace resin {

uint8_t Window::glfw_window_count_ = 0;

static void error_callback(int error_code, const char* description) {
  Logger::err("GLFW Error #{0}: {1}", error_code, description);
}

void Window::api_init() {
  const int status = glfwInit();
  if (!status) {
    throw std::runtime_error("GLFW init failed!");
  }

  glfwSetErrorCallback(error_callback);

  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  imgui_set_style();

  Logger::debug("Api init");
}

void Window::api_terminate() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();

  glfwTerminate();

  Logger::debug("Api shutdown");
}

Window::Window(WindowProperties properties) : properties_(std::move(properties)) {
  if (glfw_window_count_ == 0) {
    api_init();
  }

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  // TODO(SDF-72): proper window creation for fullscreen
  window_ptr_ = glfwCreateWindow(static_cast<int>(properties_.width), static_cast<int>(properties_.height),
                                 properties_.title.c_str(), nullptr, nullptr);
  if (window_ptr_ == nullptr) {  // MAYBE: semantic exception class (i.e. window_creation_error)?
    throw std::runtime_error("Unable to create window");
  }
  ++glfw_window_count_;

  context_ = std::make_unique<GraphicsContext>(window_ptr_);
  context_->init();

  glfwSetWindowUserPointer(window_ptr_, &properties_);

  if (properties_.x && properties_.y) {
    glfwSetWindowPos(window_ptr_, *properties_.x, *properties_.y);
  } else {
    int x, y;  // NOLINT
    glfwGetWindowPos(window_ptr_, &x, &y);
    properties_.x = x;
    properties_.y = y;
  }

  glfwSwapInterval(properties_.vsync ? 1 : 0);

  if (properties_.eventDispatcher.has_value()) {
    set_glfw_callbacks();
  } else {
    Logger::warn("Window {} created without event dispatcher!", properties_.title);
  }

  imgui_setup();

  Logger::info("Created window {} ({} x {})", properties_.title, properties_.width, properties_.height);
}

void Window::set_glfw_callbacks() const {
  glfwSetWindowCloseCallback(window_ptr_, [](GLFWwindow* window) {
    const WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));

    WindowCloseEvent window_close_event;
    properties.eventDispatcher->get().dispatch(window_close_event);
  });

  glfwSetWindowSizeCallback(window_ptr_, [](GLFWwindow* window, int width, int height) {
    WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));
    properties.width             = static_cast<unsigned int>(width);
    properties.height            = static_cast<unsigned int>(height);

    WindowResizeEvent window_resize_event(properties.width, properties.height);
    properties.eventDispatcher->get().dispatch(window_resize_event);
  });

  glfwSetKeyCallback(window_ptr_, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    const WindowProperties& properties = *static_cast<WindowProperties*>(glfwGetWindowUserPointer(window));

    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
      WindowTestEvent window_test_event;
      properties.eventDispatcher->get().dispatch(window_test_event);
    }
  });
}

Window::~Window() {
  glfwDestroyWindow(window_ptr_);
  --glfw_window_count_;

  if (glfw_window_count_ == 0) {
    api_terminate();
  }
}

void Window::on_update() {
  glfwPollEvents();
  context_->swap_buffers();
}

void Window::set_title(std::string_view title) {
  glfwSetWindowTitle(window_ptr_, title.data());
  properties_.title = title;
}

void Window::set_pos(glm::ivec2 pos) {
  glfwSetWindowPos(window_ptr_, pos.x, pos.y);
  properties_.x = pos.x;
  properties_.y = pos.y;
}

void Window::set_dimensions(glm::uvec2 dimensions) {
  glfwSetWindowSize(window_ptr_, static_cast<int>(dimensions.x), static_cast<int>(dimensions.y));
  properties_.width  = dimensions.x;
  properties_.height = dimensions.y;
}

void Window::set_vsync(bool vsync) {
  glfwSwapInterval(vsync ? 1 : 0);
  properties_.vsync = vsync;
}

void Window::set_fullscreen(bool fullscreen) {
  properties_.fullscreen = fullscreen;

  // TODO(SDF-72): proper handling of properties, we have to remember old x,y,width,height somewhere
  if (fullscreen) {
    GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwSetWindowMonitor(window_ptr_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
  } else {
    glfwSetWindowMonitor(window_ptr_, nullptr, *properties_.x, *properties_.y, static_cast<int>(properties_.width),
                         static_cast<int>(properties_.height), 0);
  }
}

void Window::imgui_setup() const {
  ImGui_ImplGlfw_InitForOpenGL(window_ptr_, true);
  ImGui_ImplOpenGL3_Init("#version 150");
}
#include <string>
#include <locale>
#include <vector>
#include <cstdlib>

std::string wstring_to_string(const std::wstring& wstr) {
    // Set the locale to handle wide characters properly
    std::setlocale(LC_ALL, "");
    
    // Get the required buffer size
    size_t required_size = 0;
    errno_t err = wcstombs_s(&required_size, nullptr, 0, wstr.c_str(), 0);
    if (err != 0) {
        throw std::runtime_error("Error getting required buffer size");
    }
    
    // Create a buffer for the converted string
    std::vector<char> buffer(required_size);
    
    // Perform the actual conversion
    size_t converted_size = 0;
    err = wcstombs_s(&converted_size, buffer.data(), required_size, wstr.c_str(), _TRUNCATE);
    if (err != 0) {
        throw std::runtime_error("Error converting wstring: conversion failed");
    }
    
    // Create string from buffer (subtract 1 to exclude null terminator)
    return std::string(buffer.data(), converted_size - 1);
}

void Window::imgui_set_style() {
  ImGuiIO& io = ImGui::GetIO();

  std::filesystem::path path = std::filesystem::current_path() / "assets" / "fonts" / "OpenSans-Regular.ttf";
  if (std::filesystem::exists(path)) {
#ifdef _WIN32
    std::wstring widePath = path.wstring();
    int size = WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8Path(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, widePath.c_str(), -1, &utf8Path[0], size, nullptr, nullptr);
    ImFont* sans_regular = io.Fonts->AddFontFromFileTTF(utf8Path.c_str(), 18.0f);
#else
    ImFont* sans_regular = io.Fonts->AddFontFromFileTTF(path.string().c_str(), 18.0f);
#endif
  } else {
    Logger::warn("Failed to load font. Using ImGui's default.");
  }

  io.Fonts->Build();

  // Resin Gold
  ImGuiStyle& style = ImGui::GetStyle();

  style.Alpha                     = 1.0f;
  style.DisabledAlpha             = 0.1000000014901161f;
  style.WindowPadding             = ImVec2(8.0f, 8.0f);
  style.WindowRounding            = 5.0f;
  style.WindowBorderSize          = 1.0f;
  style.WindowMinSize             = ImVec2(30.0f, 30.0f);
  style.WindowTitleAlign          = ImVec2(0.5f, 0.5f);
  style.WindowMenuButtonPosition  = ImGuiDir_Right;
  style.ChildRounding             = 5.0f;
  style.ChildBorderSize           = 1.0f;
  style.PopupRounding             = 5.0f;
  style.PopupBorderSize           = 0.0f;
  style.FramePadding              = ImVec2(5.0f, 3.0f);
  style.FrameRounding             = 5.0f;
  style.FrameBorderSize           = 0.0f;
  style.ItemSpacing               = ImVec2(8.0f, 4.0f);
  style.ItemInnerSpacing          = ImVec2(4.0f, 4.0f);
  style.CellPadding               = ImVec2(4.0f, 2.0f);
  style.IndentSpacing             = 5.0f;
  style.ColumnsMinSpacing         = 5.0f;
  style.ScrollbarSize             = 15.0f;
  style.ScrollbarRounding         = 9.0f;
  style.GrabMinSize               = 15.0f;
  style.GrabRounding              = 5.0f;
  style.TabRounding               = 5.0f;
  style.TabBorderSize             = 0.0f;
  style.TabMinWidthForCloseButton = 0.0f;
  style.ColorButtonPosition       = ImGuiDir_Right;
  style.ButtonTextAlign           = ImVec2(0.5f, 0.5f);
  style.SelectableTextAlign       = ImVec2(0.0f, 0.0f);

  // Colors
  constexpr ImVec4 primary =
      ImVec4(0.9803921580314636f, 0.6627451181411743f, 0.07450980693101883f, 0.7450980544090271f);
  constexpr ImVec4 gui_background = ImVec4(0.09803921729326248f, 0.09803921729326248f, 0.09803921729326248f, 1.0f);
  constexpr ImVec4 gui_light_background = ImVec4(0.2588235437870026f, 0.2588235437870026f, 0.2588235437870026f, 1.0f);
  constexpr ImVec4 grey                 = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 1.0f);
  constexpr ImVec4 white                = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  style.Colors[ImGuiCol_Text] = white;

  style.Colors[ImGuiCol_Border]            = primary;
  style.Colors[ImGuiCol_FrameBgActive]     = primary;
  style.Colors[ImGuiCol_TitleBg]           = primary;
  style.Colors[ImGuiCol_TitleBgActive]     = primary;
  style.Colors[ImGuiCol_CheckMark]         = primary;
  style.Colors[ImGuiCol_ButtonActive]      = primary;
  style.Colors[ImGuiCol_SliderGrab]        = primary;
  style.Colors[ImGuiCol_SliderGrabActive]  = primary;
  style.Colors[ImGuiCol_Button]            = primary;
  style.Colors[ImGuiCol_ButtonHovered]     = primary;
  style.Colors[ImGuiCol_HeaderHovered]     = primary;
  style.Colors[ImGuiCol_Separator]         = primary;
  style.Colors[ImGuiCol_SeparatorHovered]  = primary;
  style.Colors[ImGuiCol_SeparatorActive]   = primary;
  style.Colors[ImGuiCol_ResizeGrip]        = primary;
  style.Colors[ImGuiCol_ResizeGripHovered] = primary;
  style.Colors[ImGuiCol_ResizeGripActive]  = primary;
  style.Colors[ImGuiCol_TabHovered]        = primary;
  style.Colors[ImGuiCol_TabActive]         = primary;
  style.Colors[ImGuiCol_PlotLinesHovered]  = primary;
  style.Colors[ImGuiCol_PlotHistogram]     = primary;
  style.Colors[ImGuiCol_TableBorderStrong] = primary;
  style.Colors[ImGuiCol_TextSelectedBg]    = primary;
  style.Colors[ImGuiCol_Header]            = primary;

  style.Colors[ImGuiCol_WindowBg]     = gui_background;
  style.Colors[ImGuiCol_PopupBg]      = gui_background;
  style.Colors[ImGuiCol_Header]       = gui_background;
  style.Colors[ImGuiCol_HeaderActive] = gui_background;

  style.Colors[ImGuiCol_FrameBg]       = grey;
  style.Colors[ImGuiCol_ScrollbarGrab] = grey;

  style.Colors[ImGuiCol_TitleBgCollapsed] = gui_light_background;
  style.Colors[ImGuiCol_PopupBg]          = gui_light_background;

  // Misc
  style.Colors[ImGuiCol_TextDisabled] = ImVec4(1.0f, 1.0f, 1.0f, 0.3605149984359741f);
  style.Colors[ImGuiCol_FrameBgHovered] =
      ImVec4(0.3803921639919281f, 0.4235294163227081f, 0.572549045085907f, 0.5490196347236633f);
  style.Colors[ImGuiCol_MenuBarBg]   = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.1568627506494522f, 0.1568627506494522f, 0.1568627506494522f, 0.0f);
  style.Colors[ImGuiCol_ScrollbarGrabHovered] =
      ImVec4(0.2352941185235977f, 0.2352941185235977f, 0.2352941185235977f, 1.0f);
  style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.294117659330368f, 0.294117659330368f, 0.294117659330368f, 1.0f);
  style.Colors[ImGuiCol_Tab] =
      ImVec4(0.9803921580314636f, 0.6627451181411743f, 0.07450980693101883f, 0.1176470592617989f);
  style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.4509803950786591f, 1.0f, 0.0f);
  style.Colors[ImGuiCol_TabUnfocusedActive] =
      ImVec4(0.1333333402872086f, 0.2588235437870026f, 0.4235294163227081f, 0.0f);
  style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882352977991104f, 0.1882352977991104f, 0.2000000029802322f, 1.0f);
  style.Colors[ImGuiCol_TableBorderLight] =
      ImVec4(0.9803921580314636f, 0.6627451181411743f, 0.07450980693101883f, 0.3921568691730499f);
  style.Colors[ImGuiCol_TableRowBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  style.Colors[ImGuiCol_TableRowBgAlt]  = ImVec4(1.0f, 1.0f, 1.0f, 0.03433477878570557f);
  style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.8999999761581421f);
}

}  // namespace resin
