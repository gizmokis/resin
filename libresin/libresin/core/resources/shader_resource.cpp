#include "libresin/core/resources/shader_resource.hpp"

#include <cctype>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <string_view>
#include <vector>

#include "libresin/utils/logger.hpp"
namespace resin {

static constexpr const char* kShaderLoadingFailedMsg    = "Could not load shader resource with path \"{}\": {}";
static constexpr const char* kShaderProcessingFailedMsg = "Shader preprocessing failed (line {}): {}";

static constexpr const char* kIncludeMacro = "#include";
static constexpr const char* kExtDefiMacro = "#external_definition";

static std::string naive_load(const std::filesystem::path& path) {
  std::ifstream file_stream(path);
  if (!file_stream.is_open()) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File stream cannot be opened.");
  }
  std::stringstream buffer;
  buffer << file_stream.rdbuf();
  return buffer.str();
}

std::optional<ShaderResource> ShaderResource::from_path(const std::filesystem::path& path) {
  namespace fs = std::filesystem;

  if (!fs::exists(path)) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File does not exist.");
    return std::nullopt;
  }

  if (fs::is_regular_file(path) && !fs::is_symlink(path)) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File must be a regular file or symlink.");
    return std::nullopt;
  }

  auto sh_type = extension_to_shader_type(path.extension().string());
  if (!sh_type.has_value()) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File extension \"{}\" is not suported.", path.extension());
    return std::nullopt;
  }

  std::string raw_content = naive_load(path);

  return create(std::move(raw_content), sh_type.value());
}

static std::optional<std::string> process_include(std::string_view arg, size_t curr_line_num) {
  if (!arg.starts_with("\"") || !arg.ends_with("\"")) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument should begin and end with `\"`", kExtDefiMacro);
    return std::nullopt;
  }

  return std::string(arg.substr(1, arg.size() - 2));
}

static std::optional<std::string> process_external_defi(std::string_view arg, size_t curr_line_num) {
  if (!std::all_of(arg.begin(), arg.end(), [](const char c) { return std::isalnum(c); })) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument \"{}\" contains non-alphanumeric characters.", kExtDefiMacro, arg);
    return std::nullopt;
  }

  return std::string(arg);
}

std::optional<ShaderResource> ShaderResource::create(std::string&& content, ShaderType type) {
  size_t line_number = 1;
  auto delims        = [](auto x, auto y) { return not(x == ' ' or x == '\n' or y == ' ' or y == '\n'); };

  // clang-format off
  auto words_with_line_nums = content | std::views::chunk_by(delims) 
                                      | std::views::transform([&line_number](auto&& r) {
                                          auto word = std::string_view(r);
                                          if (word == "\n") {
                                            line_number++;
                                          }
                                          return std::make_pair(word, line_number);
                                        })
                                      | std::views::filter([](auto chunk) {
                                          return std::string_view(chunk.first) != " " 
                                              && std::string_view(chunk.first) != "\n";
                                        });
  // clang-format on

  auto it  = words_with_line_nums.begin();
  auto end = words_with_line_nums.end();

  std::vector<std::filesystem::path> paths;
  std::vector<std::string> defi_names;

  while (it != end) {
    const auto& [word, line] = *it;

    if (word != kIncludeMacro && word != kExtDefiMacro) {
      ++it;
      continue;
    }

    ++it;

    if (it == end) {
      resin::Logger::err(kShaderProcessingFailedMsg, line, "{} macro argument is absent.", word);
      return std::nullopt;
    }

    const auto& [arg, arg_line] = *it;
    if (line != arg_line) {
      resin::Logger::err(kShaderProcessingFailedMsg, line, "{} macro argument is absent.", word);
      return std::nullopt;
    }

    if (word == kIncludeMacro) {
      auto rel_path = process_include(arg, arg_line);

      if (!rel_path.has_value()) {
        return std::nullopt;
      }
      paths.emplace_back(rel_path.value());

    } else {
      auto name = process_external_defi(arg, arg_line);

      if (!name.has_value()) {
        return std::nullopt;
      }
      defi_names.emplace_back(name.value());
    }

    ++it;
  }

  return ShaderResource(std::move(content), type, std::move(paths), std::move(defi_names));
}

ShaderResource::ShaderResource(std::string&& content, ShaderType type,
                               std::vector<std::filesystem::path>&& deps_rel_paths,
                               std::vector<std::string>&& ext_defi_names)
    : deps_rel_paths_(std::move(deps_rel_paths)),
      ext_defi_names_(std::move(ext_defi_names)),
      content_(std::move(content)),
      type_(type) {}

std::optional<ShaderResource> ShaderResourceManager::load_res(const std::filesystem::path& path) {
  return ShaderResource::from_path(path);
}

}  // namespace resin
