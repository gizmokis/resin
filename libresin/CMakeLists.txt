cmake_minimum_required(VERSION 3.20)

project(libresin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Configuring " ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} libresin/resin.cpp libresin/resin.hpp libresin/utils/logger.cpp libresin/utils/logger.hpp)

# Prevent CMake from adding `lib` before `libresin`
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# The `-fmacro-prefix-map` flag is required for GCC and CLANG to ensure that `__FILE__` and `std::source_location` use 
# relative paths instead of absolute ones.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  list(APPEND PROJ_CXX_FLAGS "-fmacro-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}/=")
endif()

# Set compile options and properties of the target
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})

# This is required when libresin is built as a shared library and is linked with shared libraries
target_link_options(${PROJECT_NAME} PRIVATE ${PROJ_SHARED_LINKER_FLAGS})

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

  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_tests")
endif()
