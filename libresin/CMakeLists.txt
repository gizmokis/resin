cmake_minimum_required(VERSION 3.20)

project(libresin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Configuring " ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} 
      libresin/core/transform.cpp
      libresin/utils/logger.cpp 
      libresin/core/resources/shader_resource.cpp
)

# Prevent CMake from adding `lib` before `libresin`
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME}
                           PUBLIC . "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm glad)

# Set compile options and properties of the target
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})

# This is required when libresin is built as a shared library and is linked with shared libraries
target_link_options(${PROJECT_NAME} PRIVATE ${PROJ_SHARED_LINKER_FLAGS})

target_compile_definitions(${PROJECT_NAME} PUBLIC GLM_ENABLE_EXPERIMENTAL)

if(BUILD_TESTING)
  # Generate the tests constants 
  set(TESTS_DATA_PATH "${CMAKE_CURRENT_SOURCE_DIR}/tests/data")
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/test_consts.hpp.in"
               "${CMAKE_BINARY_DIR}/generated/tests/${PROJECT_NAME}/test_consts.hpp")

  enable_testing()

  add_executable(
    "${PROJECT_NAME}_tests"
    tests/core/transform_test.cpp
    tests/core/resources/shader_resource_test.cpp
  )

  target_include_directories("${PROJECT_NAME}_tests" PUBLIC  
    "${CMAKE_BINARY_DIR}/generated/tests/${PROJECT_NAME}/test_consts.hpp"
  )

  target_link_libraries(
    "${PROJECT_NAME}_tests"
    ${PROJECT_NAME}
    GTest::gtest_main
  )

  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_tests")
endif()
