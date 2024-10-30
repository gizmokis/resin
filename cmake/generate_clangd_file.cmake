function(generate_clangd_file COMPILER_FLAGS)
  if(CMAKE_HOST_UNIX)
    set(CLANGD_COMPILER_STANDARD "-std=c++23")
  elseif(CMAKE_HOST_WIN32)
    set(CLANGD_COMPILER_STANDARD "/clang:-std=c++23")
  endif()

  file(WRITE
    ${CMAKE_CURRENT_SOURCE_DIR}/.clangd
    "CompileFlags:\n"
    "  CompilationDatabase: build/debug/\n"
    "  Add:\n"
    "    - "
    ${CLANGD_COMPILER_STANDARD}
    "\n"
    "    - "
    ${COMPILER_FLAGS})
endfunction()
