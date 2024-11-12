cmake_minimum_required(VERSION 3.20)

project(libresin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Configuring " ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} 
        libresin/core/transform.hpp libresin/core/transform.cpp
        libresin/utils/logger.cpp libresin/utils/logger.hpp)

# Prevent CMake from adding `lib` before `libresin`
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME}
                           PUBLIC . "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# Set compile options and properties of the target
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})

# This is required when libresin is built as a shared library and is linked with shared libraries
target_link_options(${PROJECT_NAME} PRIVATE ${PROJ_SHARED_LINKER_FLAGS})

if(BUILD_TESTING)
  enable_testing()

  add_executable(
    "${PROJECT_NAME}_tests"
    tests/example_test.cpp
    tests/core/transform_test.cpp
  )
  target_link_libraries(
    "${PROJECT_NAME}_tests"
    ${PROJECT_NAME}
    GTest::gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_tests")
endif()
