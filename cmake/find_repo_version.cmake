# This function parses output of `git describe --always` in order to find the
# latest annotated tag and parse the program version. The tag name must follow
# the following pattern: `v?[0-9]+.[0-9]+(.[0-9]+)?`. If the branch is ahead of
# the latest tag, the version is marked as unstable.
#
# Note: Lightweight tags are ignored.
#
# WARNING: `find_package(Git)` call might be necessary before calling this
# function.
function(find_repo_version VERSION IS_STABLE)
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --always
    OUTPUT_VARIABLE REPO_VERSION
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

  set(VERSION
      "0.0.0"
      PARENT_SCOPE)
  set(IS_STABLE
      0
      PARENT_SCOPE)

  if(NOT GIT_RESULT EQUAL 0)
    message(WARNING "Git command failed, unable to find the build version.")
    return()
  endif()

  set(VERSION_PATTERN "^v?([0-9]+\\.[0-9]+(\\.[0-9]+)?)")
  set(VERSION_FULL_PATTERN
      "^v?[0-9]+\\.[0-9]+(\\.[0-9]+)?(-[0-9]+-g[0-9a-f]+)?$")
  set(RESULT_VERSION_PATTERN "^[0-9]+\\.[0-9]+\\.[0-9]+$")

  string(REGEX MATCH ${VERSION_FULL_PATTERN} VERSION_FULL_MATCH ${REPO_VERSION})
  if(NOT VERSION_FULL_MATCH)
    return()
  endif()

  string(REGEX REPLACE "${VERSION_PATTERN}(.*)" "\\1" EXTRACTED_VERSION
                       ${REPO_VERSION})
  string(REGEX MATCH ${RESULT_VERSION_PATTERN}$ RESULT_MATCH
               ${EXTRACTED_VERSION})

  if(RESULT_MATCH)
    set(VERSION
        ${EXTRACTED_VERSION}
        PARENT_SCOPE)
  else()
    set(VERSION
        "${EXTRACTED_VERSION}.0"
        PARENT_SCOPE)
  endif()

  string(REGEX MATCH "${VERSION_PATTERN}$" STABLE_MATCH ${REPO_VERSION})
  if(STABLE_MATCH)
    set(IS_STABLE
        1
        PARENT_SCOPE)
  endif()

endfunction()
