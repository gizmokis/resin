#ifndef RESIN_TESTS_FILES_HELPER
#define RESIN_TESTS_FILES_HELPER
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

::testing::AssertionResult IsFileContentEqual(const char* expected_expr, const char* actual_expr,
                                              const std::filesystem::path& path, const std::string& content) {
  namespace fs = std::filesystem;

  if (!fs::exists(path)) {
    return ::testing::AssertionFailure() << "File does not exist: " << expected_expr << "\n";
  }

  if (!fs::is_regular_file(path) && !fs::is_symlink(path)) {
    return ::testing::AssertionFailure() << "Path is not a regular file or a symlink file: " << expected_expr;
  }

  std::ifstream file(path, std::ios::in | std::ios::binary);
  if (!file.is_open()) {
    return ::testing::AssertionFailure() << "Could not open file: " << expected_expr << "\n";
  }

  std::string file_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  file_content = std::regex_replace(file_content, std::regex("\r\n"), "\n");

  if (file_content != content) {
    return ::testing::AssertionFailure() << "File content does not match.\n";
  }

  return ::testing::AssertionSuccess();
}

#define EXPECT_FILE_CONTENT_EQ(expected, actual) EXPECT_PRED_FORMAT2(IsFileContentEqual, expected, actual)

#endif
