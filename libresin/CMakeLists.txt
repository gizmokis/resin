cmake_minimum_required(VERSION 3.20)

project(libresin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
message(STATUS "Configuring " ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

add_library(${PROJECT_NAME} libresin/resin.cpp libresin/resin.hpp)

# Prevent CMake from adding `lib` before `libresin`
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)

# Set compile options and properties of the target
target_compile_options(${PROJECT_NAME} PRIVATE ${PROJ_CXX_FLAGS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS
                                                 "${PROJ_SHARED_LINKER_FLAGS}")

if(DEFINED CLANG_TIDY_COMMAND AND NOT CLANG_TIDY_COMMAND STREQUAL "")
  set_target_properties(libresin PROPERTIES CXX_CLANG_TIDY
                                            "${CLANG_TIDY_COMMAND}")
endif()
