message(STATUS "Assets source directory: ${ASSETS_SOURCE_DIR}")
message(STATUS "Assets destination directory: ${ASSETS_DEST_DIR}")

file(COPY ${ASSETS_SOURCE_DIR} DESTINATION ${ASSETS_DEST_DIR})