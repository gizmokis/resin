#include <cctype>
#include <filesystem>
#include <fstream>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <libresin/utils/string_views.hpp>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace resin {
ShaderResource::ShaderResource(std::string&& content, ShaderType type, std::unordered_set<std::string>&& ext_defi_names,
                               std::optional<std::string>&& version)
    : ext_defi_names_(std::move(ext_defi_names)),
      version_(version),
      raw_content_(std::move(content)),
      type_(type),
      is_dirty_(true) {}

const std::unordered_set<std::string>& ShaderResource::get_ext_defi_names() const { return ext_defi_names_; }

void ShaderResource::set_ext_defi(std::string_view ext_defi_name, std::string&& defi_content) {
  auto it = std::ranges::find(ext_defi_names_, ext_defi_name);
  if (it == ext_defi_names_.end()) {
    resin::Logger::warn("Shader resource could not find external definition named \"{}\"", ext_defi_name);
    return;
  }

  ext_defi_contents_[*it] = std::move(defi_content);
  is_dirty_               = true;
}

bool ShaderResource::is_glsl_ready() const { return ext_defi_contents_.size() == ext_defi_names_.size(); }

const std::string& ShaderResource::get_raw() const { return raw_content_; }

const std::string& ShaderResource::get_glsl() const {
  if (!is_dirty_) {
    return glsl_;
  }

  glsl_.clear();
  if (version_ != std::nullopt) {
    glsl_.append(version_.value());
    glsl_.append("\n");
  }
  for (const auto& ext_defi : ext_defi_contents_) {
    glsl_.append(std::format("#define {} {}\n", ext_defi.first, ext_defi.second));
  }
  glsl_.append(raw_content_);

  is_dirty_ = false;
  return glsl_;
}

static ShaderType get_sh_type(const std::filesystem::path& path) {
  auto file_ext = path.extension().string();
  auto sh_type  = extension_to_shader_type(file_ext);
  if (!sh_type.has_value()) {
    log_throw(FileExtensionNotSupportedException(path.string(), std::string(file_ext)));
  }

  return sh_type.value();
}

static std::string load_content(const std::filesystem::path& path) {
  namespace fs = std::filesystem;

  if (!fs::exists(path)) {
    log_throw(FileDoesNotExistException(path.string()));
  }

  if (!fs::is_regular_file(path) && !fs::is_symlink(path)) {
    log_throw(InvalidFileTypeException(path.string(), "Expected regular or symlink file."));
  }

  std::ifstream file_stream(path.string());
  if (!file_stream.is_open()) {
    log_throw(FileStreamNotAvailableException(path.string()));
  }

  std::stringstream buffer;
  buffer << file_stream.rdbuf();
  return buffer.str();
}

void ShaderResourceManager::process_include_macro(const std::filesystem::path& sh_path, WordsStringViewIterator& it,
                                                  const WordsStringViewIterator& end, size_t curr_line,
                                                  std::string& content, std::unordered_set<std::string>& defi_names) {
  if (it == end) {
    clear_log_throw(ShaderMacroInvalidArgumentsCountException(
        sh_path.string(), std::string(shader_macros::kExtDefiMacro), 0, 1, curr_line));
  }
  auto arg = std::string_view{*it};

  ++it;
  if (it != end) {
    clear_log_throw(ShaderMacroInvalidArgumentsCountException(
        sh_path.string(), std::string(shader_macros::kVersionMacro), 1, 2, curr_line));
  }

  if (!arg.starts_with("\"") || !arg.ends_with("\"") || arg.size() < 2) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path.string(), "The include macro argument should begin and end with `\"`.", curr_line));
  }

  auto arg_val  = std::string_view{arg.substr(1, arg.size() - 2)};
  auto rel_path = std::filesystem::path{arg_val};
  if (rel_path.empty()) {
    clear_log_throw(ShaderInvalidMacroArgumentException(sh_path.string(), "The include macro argument cannot be empty.",
                                                        curr_line));
  }

  if (rel_path.is_absolute()) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path.string(), "The include macro argument cannot be an absolute path.", curr_line));
  }

  auto dep_ext = resin::extension_to_shader_type(rel_path.extension().string());
  if (!dep_ext.has_value() || dep_ext.value() != ShaderType::Library) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path.string(), "The include macro argument must be a library shader (.glsl extension).", curr_line));
  }

  auto abs_path = sh_path.parent_path() / rel_path;
  if (abs_path == sh_path || std::ranges::find(visited_paths_, abs_path) != visited_paths_.end()) {
    clear_log_throw(ShaderIncludeMacroDependencyCycleException(sh_path.string(), curr_line));
  }

  visited_paths_.push_back(abs_path);
  auto res = this->get_res(abs_path);
  visited_paths_.pop_back();

  content.append(res->get_raw());
  defi_names.insert(res->get_ext_defi_names().begin(), res->get_ext_defi_names().end());
}

void ShaderResourceManager::process_ext_defi_macro(const std::filesystem::path& sh_path, WordsStringViewIterator& it,
                                                   const WordsStringViewIterator& end, size_t curr_line,
                                                   std::unordered_set<std::string>& defi_names) {
  if (it == end) {
    clear_log_throw(ShaderMacroInvalidArgumentsCountException(
        sh_path.string(), std::string(shader_macros::kExtDefiMacro), 0, 1, curr_line));
  }
  auto arg = std::string_view{*it};

  ++it;
  if (it != end) {
    clear_log_throw(ShaderMacroInvalidArgumentsCountException(
        sh_path.string(), std::string(shader_macros::kVersionMacro), 1, 2, curr_line));
  }

  if (!std::all_of(arg.begin(), arg.end(), [](const char c) { return std::isalnum(c) != 0 || c == '_'; })) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path.string(), "The external definition macro argument contains non-alphanumeric characters.", curr_line));
  }

  defi_names.emplace(arg);
}

std::optional<std::string> ShaderResourceManager::process_version_macro(const std::filesystem::path& sh_path,
                                                                        ShaderType sh_type, WordsStringViewIterator& it,
                                                                        const WordsStringViewIterator& end,
                                                                        size_t curr_line) {
  if (sh_type == ShaderType::Library) {
    resin::Logger::warn("Ignoring version macro in .glsl shader.");
    return std::nullopt;
  }

  if (it == end) {
    clear_log_throw(ShaderMacroInvalidArgumentsCountException(
        sh_path.string(), std::string(shader_macros::kVersionMacro), 0, 1, curr_line));
  }
  auto arg1 = std::string_view{*it};

  ++it;
  if (it != end) {
    auto arg2 = std::string_view{*it};

    ++it;
    if (it != end) {
      clear_log_throw(ShaderMacroInvalidArgumentsCountException(
          sh_path.string(), std::string(shader_macros::kVersionMacro), 2, 3, curr_line));
    }

    return std::format("#version {} {}", arg1, arg2);
  }

  return std::format("#version {}", arg1);
}

ShaderResource ShaderResourceManager::load_res(const std::filesystem::path& path) {
  auto sh_type = get_sh_type(path);
  auto content = load_content(path);

  auto lines = make_lines_view(content) | std::views::enumerate;

  std::unordered_set<std::string> defi_names;
  std::string preprocessed_content;
  std::optional<std::string> version;

  for (auto const [l, line_str] : lines) {
    auto line  = static_cast<size_t>(l);
    auto words = make_words_view(line_str);

    auto it  = words.begin();
    auto end = words.end();

    if (it == end) {
      if (line_str != "") {
        preprocessed_content.append(line_str);
        preprocessed_content.append("\n");
      }
      continue;
    }

    auto macro = std::string_view{*it};
    if (!shader_macros::is_macro(macro)) {
      preprocessed_content.append(line_str);
      preprocessed_content.append("\n");
      continue;
    }

    ++it;
    if (macro == shader_macros::kIncludeMacro) {
      process_include_macro(path, it, end, line, preprocessed_content, defi_names);
    } else if (macro == shader_macros::kVersionMacro) {
      if (version != std::nullopt) {
        continue;
      }
      version = process_version_macro(path, sh_type, it, end, line);
    } else {
      process_ext_defi_macro(path, it, end, line, defi_names);
    }
  }

  if (sh_type != ShaderType::Library && !version.has_value()) {
    clear_log_throw(ShaderAbsentVersionException(path.string()));
  }

  return ShaderResource(std::move(preprocessed_content), sh_type, std::move(defi_names), std::move(version));
}

}  // namespace resin
