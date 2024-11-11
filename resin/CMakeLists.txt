cmake_minimum_required(VERSION 3.20)

project(resin CXX)
message(STATUS "Configuring " ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME} resin/main.cpp 
               resin/event/event.hpp resin/event/window_events.hpp
               resin/core/graphics_context.hpp resin/core/graphics_context.cpp
               resin/core/window.hpp resin/core/window.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw libresin glm::glm imgui glad)
target_include_directories(${PROJECT_NAME}
                           PUBLIC . "${CMAKE_BINARY_DIR}/generated")

# GLFW_INCLUDE_NONE makes the GLFW header not include any OpenGL or OpenGL ES
# API header. This is useful in combination with an extension loading library
# (GLAD in our case).
target_compile_definitions(${PROJECT_NAME} PUBLIC GLFW_INCLUDE_NONE)

# Set compile options and properties of the target
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})

# This is required when resin is linked with shared libraries
target_link_options(${PROJECT_NAME} PRIVATE ${PROJ_EXE_LINKER_FLAGS})

if(BUILD_TESTING)
  enable_testing()

  add_executable(
    "${PROJECT_NAME}_tests"
    tests/example_test.cpp
  )
  target_link_libraries(
    "${PROJECT_NAME}_tests"
    GTest::gtest_main
  )

  include(CTest)
  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_tests")
endif()
