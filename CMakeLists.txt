cmake_minimum_required(VERSION 3.20)

find_package(Git)
include(cmake/find_repo_version.cmake)
find_repo_version(VERSION IS_STABLE)

project(
  resin
  DESCRIPTION "SDF Modeler"
  LANGUAGES CXX C
  VERSION ${VERSION})

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GENERATE_CLANGD_FILE
       "Generate the .clangd file for the host operating system" ON)
option(FIND_GLM "Try to find glm on the host operating system" OFF)
option(FIND_GLFW "Try to find glfw on the host operating system" OFF)
option(
  USE_IMPLICIT_INCLUDE_DIRECTORIES
  "Add the implicit include directories to standard include directories.
   May be helpful (especially for nix users) for compile_commands.json generation,
   when the clangd struggles to find the standard libraries."
  OFF)

# Print version
set(VERSION_SUFFIX "")
if(IS_STABLE EQUAL 0)
  set(VERSION_SUFFIX "(unstable)")
endif()
message(STATUS "Version: ${CMAKE_PROJECT_VERSION}${VERSION_SUFFIX}")

# Generate the version header
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.hpp.in
               ${CMAKE_BINARY_DIR}/generated/version/version.hpp)

# Provide standard include directories for explicit compile_commands.json
# generation
if(USE_IMPLICIT_INCLUDE_DIRECTORIES)
  set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES
      ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
  message("-- Using the
following implicit include directories to standard include directories: ")
  message(${CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES})
endif()

if(GENERATE_CLANGD_FILE)
  include(cmake/generate_clangd_file.cmake)
  generate_clangd_file("${PROJ_CXX_FLAGS}")
endif()

# Set the binaries and libraries output directory
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Multi-config generators (e.g. Visual Studio) when CMAKE_CONFIGURATION_TYPES is
# set to "Relase", by default put the output files in the
# ${CMAKE_(ARCHIVE|LIBRARY|RUNTIME)_OUTPUT_DIRECTORY}/Release directory in order
# to avoid collisions with the debug binaries. In our setup there is no need for
# this, since the debug binary is put in the `build/debug/bin`.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Prepare dependencies
set(BUILD_GLM ON)
if(FIND_GLM)
  find_package(GLM 1.0.1)
  if(GLM_FOUND)
    set(BUILD_GLM OFF)
  endif()
endif()

set(BUILD_GLFW ON)
if(FIND_GLFW)
  find_package(GLFW 3.4)
  if(GLFW_FOUND)
    set(BUILD_GLFW OFF)
  endif()
endif()

add_subdirectory(cmake/dependencies)

add_subdirectory(resin)
add_subdirectory(libresin)
