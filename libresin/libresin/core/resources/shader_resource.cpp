#include <array>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/utils/logger.hpp>
#include <optional>
#include <ranges>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace resin {

static constexpr const char* kShaderLoadingFailedMsg    = "Could not load shader resource with path \"{}\": {}";
static constexpr const char* kShaderProcessingFailedMsg = "Shader resource preprocessing failed (line {}): {}";

static constexpr const char* kIncludeMacro = "#include";
static constexpr const char* kExtDefiMacro = "#external_definition";

static constexpr std::array<const char*, 2> kAllMacros = {
    kIncludeMacro,
    kExtDefiMacro,
};

static constexpr bool is_macro(std::string_view word) {
  return std::ranges::find(kAllMacros, word) != kAllMacros.end();
}

ShaderResource::ShaderResource(std::filesystem::path path, std::string&& content, ShaderType type,
                               std::unordered_set<std::string>&& ext_defi_names)
    : path_(std::move(path)),
      ext_defi_names_(std::move(ext_defi_names)),
      raw_content_(std::move(content)),
      type_(type),
      is_dirty_(false) {}

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
  for (const auto& ext_defi : ext_defi_contents_) {
    glsl_.append(std::format("#define {} {}\n", ext_defi.first, ext_defi.second));
  }
  glsl_.append(raw_content_);

  is_dirty_ = false;
  return glsl_;
}

static std::optional<ShaderType> get_sh_type(const std::filesystem::path& path) {
  auto sh_type = extension_to_shader_type(path.extension().string());
  if (!sh_type.has_value()) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File extension \"{}\" is not suported.", path.extension());
    return std::nullopt;
  }

  return sh_type;
}

static std::optional<std::string> load_content(const std::filesystem::path& path) {
  namespace fs = std::filesystem;

  if (!fs::exists(path)) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File does not exist.");
    return std::nullopt;
  }

  if (fs::is_regular_file(path) && !fs::is_symlink(path)) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File must be a regular file or symlink.");
    return std::nullopt;
  }

  std::ifstream file_stream(path.string());
  if (!file_stream.is_open()) {
    resin::Logger::err(kShaderLoadingFailedMsg, path, "File stream cannot be opened.");
  }

  std::stringstream buffer;
  buffer << file_stream.rdbuf();
  return buffer.str();
}

static std::optional<std::filesystem::path> process_include_macro(std::string_view arg, int64_t curr_line_num) {
  if (!arg.starts_with("\"") || !arg.ends_with("\"") || arg.size() != 2) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument should begin and end with `\"`", kIncludeMacro);
    return std::nullopt;
  }

  auto arg_val = std::string_view{arg.substr(1, arg.size() - 2)};
  auto path    = std::filesystem::path{arg_val};
  if (path.empty()) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num, "The {} macro argument cannot be empty.",
                       kIncludeMacro, arg_val);
    return std::nullopt;
  }

  if (path.is_absolute()) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument \"{}\" cannot be an absolute path.", kIncludeMacro, arg_val);
    return std::nullopt;
  }
  auto dep_ext = resin::extension_to_shader_type(path.extension().string());

  if (dep_ext.has_value() && dep_ext.value() == ShaderType::Library) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument \"{}\" must a library shader (.glsl extension).", kIncludeMacro, arg_val);
    return std::nullopt;
  }

  return path;
}

static std::optional<std::string> process_ext_defi_macro(std::string_view arg, int64_t curr_line_num) {
  if (!std::all_of(arg.begin(), arg.end(), [](const char c) { return std::isalnum(c); })) {
    resin::Logger::err(kShaderProcessingFailedMsg, curr_line_num,
                       "The {} macro argument \"{}\" contains non-alphanumeric characters.", kExtDefiMacro, arg);
    return std::nullopt;
  }

  return std::string(arg);
}

std::optional<ShaderResource> ShaderResourceManager::load_res(const std::filesystem::path& path) {
  auto content = load_content(path);
  if (!content.has_value()) {
    return std::nullopt;
  }

  auto sh_type = get_sh_type(path);
  if (!sh_type.has_value()) {
    return std::nullopt;
  }

  auto lines = content.value() | std::views::split('\n')
               | std::views::transform([](auto&& r) { return std::string_view(r); }) | std::views::enumerate;

  std::unordered_set<std::string> defi_names;

  std::string preprocessed_content;
  for (auto const [line, line_str] : lines) {
    auto words = line_str | std::views::split(' ') | std::views::transform([](auto&& r) { return std::string_view(r); })
                 | std::views::filter([](auto chunk) { return !chunk.empty(); });

    auto it  = words.begin();
    auto end = words.end();

    if (it == end) {
      preprocessed_content.append(line_str);
      continue;
    }

    auto macro = std::string_view{*it};
    if (!is_macro(macro)) {
      preprocessed_content.append(line_str);
      continue;
    }

    ++it;
    if (it == end) {
      resin::Logger::err(kShaderProcessingFailedMsg, line, "{} macro argument is absent.", macro);
      return std::nullopt;
    }
    auto arg = std::string_view{*it};

    ++it;
    if (it != end) {
      resin::Logger::err(kShaderProcessingFailedMsg, line, "{} macro expects only one argument", macro);
      return std::nullopt;
    }

    if (macro == kIncludeMacro) {
      auto rel_path = process_include_macro(arg, line);

      if (!rel_path.has_value()) {
        return std::nullopt;
      }

      auto abs_path = path / rel_path.value();
      if (std::ranges::find(visited_paths_, abs_path) != visited_paths_.end()) {
        resin::Logger::err(kShaderProcessingFailedMsg, line, "Detected dependency cycle: {} -> {}", visited_paths_,
                           abs_path);
        return std::nullopt;
      }

      visited_paths_.push_back(abs_path);
      auto res = get_res(abs_path);
      if (!res.has_value()) {
        return std::nullopt;
      }
      visited_paths_.pop_back();

      preprocessed_content.append(res.value()->get_raw());
      defi_names.insert(res.value()->get_ext_defi_names().begin(), res.value()->get_ext_defi_names().end());
    } else {
      auto name = process_ext_defi_macro(arg, line);

      if (!name.has_value()) {
        return std::nullopt;
      }
      defi_names.emplace(name.value());
    }
  }

  return ShaderResource(std::move(path), std::move(preprocessed_content), sh_type.value(), std::move(defi_names));
}

}  // namespace resin
