#include <cctype>
#include <filesystem>
#include <fstream>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/helpers/logger_helpers.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_set>

namespace resin {
ShaderResource::ShaderResource(std::string&& content, ShaderType type, std::unordered_set<std::string>&& ext_defi_names)
    : ext_defi_names_(std::move(ext_defi_names)), raw_content_(std::move(content)), type_(type), is_dirty_(true) {}

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
  glsl_.append(shader_macros::kSupportedShaderVersion);
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
    helpers::log_file_err(path, "File extension \"{}\" is not suported.", file_ext);

    throw FileExtensionNotSupportedException();
  }

  return sh_type.value();
}

static std::string load_content(const std::filesystem::path& path) {
  namespace fs = std::filesystem;

  if (!fs::exists(path)) {
    helpers::log_file_err(path, "File does not exist.");
    throw FileDoesNotExistException();
  }

  if (!fs::is_regular_file(path) && !fs::is_symlink(path)) {
    helpers::log_file_err(path, "File must be a regular file or symlink.");
    throw InvalidFileTypeException();
  }

  std::ifstream file_stream(path.string());
  if (!file_stream.is_open()) {
    helpers::log_file_err(path, "File stream cannot be opened.");
    throw FileStreamNotAvailableException();
  }

  std::stringstream buffer;
  buffer << file_stream.rdbuf();
  return buffer.str();
}

static std::optional<std::filesystem::path> process_include_macro(std::string_view arg, int64_t curr_line_num) {
  if (!arg.starts_with("\"") || !arg.ends_with("\"") || arg.size() < 2) {
    helpers::log_sh_err(curr_line_num, "The include macro argument should begin and end with `\"`.");
    return std::nullopt;
  }

  auto arg_val = std::string_view{arg.substr(1, arg.size() - 2)};
  auto path    = std::filesystem::path{arg_val};
  if (path.empty()) {
    helpers::log_sh_err(curr_line_num, "The include macro argument cannot be empty.");
    return std::nullopt;
  }

  if (path.is_absolute()) {
    helpers::log_sh_err(curr_line_num, "The include macro argument cannot be an absolute path.");
    return std::nullopt;
  }
  auto dep_ext = resin::extension_to_shader_type(path.extension().string());

  if (!dep_ext.has_value() || dep_ext.value() != ShaderType::Library) {
    helpers::log_sh_err(curr_line_num, "The include macro argument must a library shader (.glsl extension).");
    return std::nullopt;
  }

  return path;
}

static std::optional<std::string> process_ext_defi_macro(std::string_view arg, int64_t curr_line_num) {
  if (!std::all_of(arg.begin(), arg.end(), [](const char c) { return std::isalnum(c) != 0 || c == '_'; })) {
    helpers::log_sh_err(curr_line_num, "The external definition macro argument contains non-alphanumeric characters.");
    return std::nullopt;
  }

  return std::string(arg);
}

ShaderResource ShaderResourceManager::load_res(const std::filesystem::path& path) {
  auto content = load_content(path);
  auto sh_type = get_sh_type(path);

  auto lines = content | std::views::split('\n') | std::views::transform([](auto&& r) { return std::string_view(r); })
               | std::views::enumerate;

  std::unordered_set<std::string> defi_names;

  std::string preprocessed_content;

  for (auto const [line, line_str] : lines) {
    auto words = line_str | std::views::split(' ') | std::views::transform([](auto&& r) { return std::string_view(r); })
                 | std::views::filter([](auto chunk) { return !chunk.empty(); });

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
    if (it == end) {
      helpers::log_sh_err(line, "Macro {} requires an argument.", macro);
      clear_and_throw<ShaderInvalidMacroArgumentsException>();
    }
    auto arg = std::string_view{*it};

    ++it;
    if (it != end) {
      helpers::log_sh_err(line, "Macro {} expects exactly one argument.", macro);
      clear_and_throw<ShaderInvalidMacroArgumentsException>();
    }

    if (macro == shader_macros::kIncludeMacro) {
      auto rel_path = process_include_macro(arg, line);

      if (!rel_path.has_value()) {
        clear_and_throw<ShaderInvalidMacroArgumentsException>();
      }

      auto abs_path = path.parent_path() / rel_path.value();
      if (std::ranges::find(visited_paths_, abs_path) != visited_paths_.end()) {
        helpers::log_sh_err(line, "Detected dependency cycle starting and ending in {}.", abs_path.string());
        clear_and_throw<ShaderDependencyCycleException>();
      }

      visited_paths_.push_back(abs_path);
      auto res = get_res(abs_path);
      visited_paths_.pop_back();

      preprocessed_content.append(res->get_raw());
      defi_names.insert(res->get_ext_defi_names().begin(), res->get_ext_defi_names().end());
    } else {
      auto name = process_ext_defi_macro(arg, line);

      if (!name.has_value()) {
        clear_and_throw<ShaderInvalidMacroArgumentsException>();
      }
      defi_names.emplace(name.value());
    }
    // all version macros are skipped
    // TODO(migoox): add multiple glsl versions support?
  }

  return ShaderResource(std::move(preprocessed_content), sh_type, std::move(defi_names));
}

}  // namespace resin