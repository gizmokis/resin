cmake_minimum_required(VERSION 3.20)

project(libresin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Configuring " ${PROJECT_NAME})

message(STATUS "Injecting JSON schemas")
include(${PROJECT_SOURCE_DIR}/../cmake/inject_defs_into_schema.cmake)
inject_defs_into_schema(PREFAB_JSON_SCHEMA ${PROJECT_SOURCE_DIR}/../schemas/definitions.schema.json ${PROJECT_SOURCE_DIR}/../schemas/prefab.schema.json)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/json_schemas.hpp.in
               ${CMAKE_BINARY_DIR}/generated/json_schemas/json_schemas.hpp)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/libresin/*.cpp"
)

add_library(${PROJECT_NAME} ${SOURCES})

# Prevent CMake from adding `lib` before `libresin`
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME}
                           PUBLIC . "${CMAKE_BINARY_DIR}/generated")
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm glad nlohmann_json assimp)

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

  file(GLOB_RECURSE TESTS_SOURCES 
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
  )

  add_executable(
    "${PROJECT_NAME}_tests"
    ${TESTS_SOURCES}
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
