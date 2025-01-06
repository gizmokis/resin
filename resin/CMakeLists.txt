cmake_minimum_required(VERSION 3.20)

project(resin CXX)
message(STATUS "Configuring " ${PROJECT_NAME})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/resin/*.cpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC glfw libresin glm::glm imgui glad nfd)
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

# Copy assets folder to build directory
add_custom_target(copy_assets
  COMMAND ${CMAKE_COMMAND} -DASSETS_SOURCE_DIR=${PROJECT_SOURCE_DIR}/assets
                           -DASSETS_DEST_DIR=$<TARGET_FILE_DIR:${PROJECT_NAME}>
                           -P ${PROJECT_SOURCE_DIR}/../cmake/copy_assets.cmake
)
add_dependencies(${PROJECT_NAME} copy_assets)

if(BUILD_TESTING)
  enable_testing()

  file(GLOB_RECURSE TESTS_SOURCES 
      "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
  )

  add_executable(
    "${PROJECT_NAME}_tests"
    ${TESTS_SOURCES} 
  )
  target_link_libraries(
    "${PROJECT_NAME}_tests"
    GTest::gtest_main
  )

  include(CTest)
  include(GoogleTest)
  gtest_discover_tests("${PROJECT_NAME}_tests")
endif()
