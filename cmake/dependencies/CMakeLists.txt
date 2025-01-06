include(FetchContent)
set(CMAKE_EXPORT_COMPILE_COMMANDS
    OFF
    CACHE BOOL "Do not generate compile commands database for dependencies"
          FORCE)

# ##############################################################################
# GLAD
# ##############################################################################
message(CHECK_START "Fetching GLAD")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

FetchContent_Declare(
  glad
  GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
  GIT_TAG "v2.0.7"
  PATCH_COMMAND git apply --ignore-whitespace
                "${CMAKE_CURRENT_LIST_DIR}/../../patches/glad.patch"
  UPDATE_DISCONNECTED 1)
FetchContent_MakeAvailable(glad)
FetchContent_GetProperties(glad)

add_subdirectory("${glad_SOURCE_DIR}/cmake" glad_cmake)
glad_add_library(
  glad
  STATIC
  REPRODUCIBLE
  EXCLUDE_FROM_ALL
  LOADER
  API
  gl:core=4.6)

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "fetched")

# ##############################################################################
# GLFW
# ##############################################################################
if(BUILD_GLFW)
  message(CHECK_START "Fetching GLFW")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "Do not build GLFW example programs" FORCE)
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "Do not build GLFW test programs" FORCE)
  set(GLFW_BUILD_DOCS
      OFF
      CACHE BOOL "Do not build GLFW documentation" FORCE)
  set(GLFW_INSTALL
      OFF
      CACHE BOOL "Do not install the GLFW" FORCE)

  FetchContent_Declare(
    glfw
    GIT_REPOSITORY "https://github.com/glfw/glfw.git"
    GIT_TAG "3.4"
    PATCH_COMMAND git apply --ignore-whitespace
                  "${CMAKE_CURRENT_LIST_DIR}/../../patches/glfw.patch"
    UPDATE_DISCONNECTED 1)
  FetchContent_MakeAvailable(glfw)

  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "fetched")
endif()

# ##############################################################################
# ImGui (depends on GLFW)
# ##############################################################################
message(CHECK_START "Fetching ImGui")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

FetchContent_Declare(
  imgui
  GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
  GIT_TAG "v1.91.1-docking"
  PATCH_COMMAND git apply --ignore-whitespace
                "${CMAKE_CURRENT_LIST_DIR}/../../patches/imgui.patch"
  UPDATE_DISCONNECTED 1)
FetchContent_MakeAvailable(imgui)

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "fetched")

# ##############################################################################
# GLM
# ##############################################################################
if(BUILD_GLM)
  message(CHECK_START "Fetching GLM")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  FetchContent_Declare(
    glm
    GIT_REPOSITORY "https://github.com/g-truc/glm.git"
    GIT_TAG "1.0.1"
    PATCH_COMMAND git apply --ignore-whitespace
                  "${CMAKE_CURRENT_LIST_DIR}/../../patches/glm.patch"
    UPDATE_DISCONNECTED 1)
  FetchContent_MakeAvailable(glm)

  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "fetched")
endif()

# ##############################################################################
# nlohmann/json
# ##############################################################################
message(CHECK_START "Fetching nlohmann/json")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

FetchContent_Declare(
  nlohmann_json 
  GIT_REPOSITORY "https://github.com/nlohmann/json.git"
  GIT_TAG "v3.11.3"
  UPDATE_DISCONNECTED 1)
FetchContent_MakeAvailable(nlohmann_json)

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "fetched")

# ##############################################################################
# assimp 
# ##############################################################################
if(BUILD_ASSIMP)
  message(CHECK_START "Fetching assimp")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  set(ASSIMP_BUILD_TESTS
      OFF
      CACHE BOOL "Do not build assimp tests" FORCE)

  set(ASSIMP_INSTALL
      OFF
      CACHE BOOL "Do not install assimp" FORCE)

  set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT
      OFF
      CACHE BOOL "Disable all importers" FORCE)

  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT
      OFF
      CACHE BOOL "Disable all importers" FORCE)

  set(ASSIMP_BUILD_GLTF_IMPORTER
      ON
      CACHE BOOL "Enable GLTF importer" FORCE)

  set(ASSIMP_BUILD_GLTF_EXPORTER
      ON
      CACHE BOOL "Disable GLTF exporter" FORCE)

  set(ASSIMP_BUILD_OBJ_EXPORTER
      ON
      CACHE BOOL "Disable OBJ exporter" FORCE)

  set(ASSIMP_BUILD_ASSIMP_VIEW
      OFF
      CACHE BOOL "Do not build the assimp view tool." FORCE)
    
  set(ASSIMP_BUILD_ZLIB
      ON
      CACHE BOOL "Build the zlib" FORCE)
       
  FetchContent_Declare(
  assimp 
  GIT_REPOSITORY "https://github.com/assimp/assimp.git"
  GIT_TAG "v5.4.3"
  UPDATE_DISCONNECTED 1)
  FetchContent_MakeAvailable(assimp)

  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "fetched")
endif()

# ##############################################################################
# nativefiledialog-extended
# ##############################################################################
message(CHECK_START "Fetching nativefiledialog-extended")
list(APPEND CMAKE_MESSAGE_INDENT "  ")

# IMPORTANT: The xdg-desktop-portal allows to display the actual native file dialog
# on linux systems (by default the nfd uses gtk). However there might be some issues 
# on DEs that are not directly supporting the portals. The rule of thumb is that everywhere 
# you can run file dialog from your flatpak app, the portals should work seamlessly.
set(NFD_PORTAL
    ON
    CACHE BOOL "Use portal to use native file dialog instead of the gtk" FORCE)

FetchContent_Declare(
  nfd 
  GIT_REPOSITORY "https://github.com/btzy/nativefiledialog-extended.git"
  GIT_TAG "v1.2.1"
  PATCH_COMMAND git apply --ignore-whitespace
                "${CMAKE_CURRENT_LIST_DIR}/../../patches/nfd.patch"
  UPDATE_DISCONNECTED 1)
FetchContent_MakeAvailable(nfd)

list(POP_BACK CMAKE_MESSAGE_INDENT)
message(CHECK_PASS "fetched")

# ##############################################################################
# GTest
# ##############################################################################
if(BUILD_TESTING)
  message(CHECK_START "Fetching GoogleTest")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY "https://github.com/google/googletest.git"
    GIT_TAG "v1.15.2"
  )

  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "fetched")
endif()

# ##############################################################################
# valijson
# ##############################################################################
if(BUILD_TESTING)
  message(CHECK_START "Fetching valijson")
  list(APPEND CMAKE_MESSAGE_INDENT "  ")

  FetchContent_Declare(
      valijson 
  GIT_REPOSITORY "https://github.com/tristanpenman/valijson.git"
  GIT_TAG "v1.0.3"
  GIT_SUBMODULES ""
  PATCH_COMMAND git apply 
                  "${CMAKE_CURRENT_LIST_DIR}/../../patches/valijson.patch"
  UPDATE_DISCONNECTED 1)
  FetchContent_MakeAvailable(valijson)

  list(POP_BACK CMAKE_MESSAGE_INDENT)
  message(CHECK_PASS "fetched")
endif()
