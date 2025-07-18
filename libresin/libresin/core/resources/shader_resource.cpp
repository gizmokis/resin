#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/resources/shader_type.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <libresin/utils/string_views.hpp>
#include <optional>
#include <ostream>
#include <ranges>
#include <regex>
#include <string_view>
#include <unordered_set>
#include <variant>

namespace resin {

int ShaderResourceManager::shader_name_id_ = 1;

ShaderResource::ShaderResource(std::string&& raw_glsl, std::string&& intermediate_glsl, std::string&& name,
                               ShaderType&& type, std::unordered_set<std::string>&& ext_defi_names,
                               std::optional<std::string>&& version)
    : ext_defi_names_(std::move(ext_defi_names)),
      version_(std::move(version)),
      name_(std::move(name)),
      type_(std::move(type)),
      raw_glsl_(std::move(raw_glsl)),
      intermediate_glsl_(std::move(intermediate_glsl)),
      is_dirty_(true) {}

const std::unordered_set<std::string>& ShaderResource::get_external_definition_names() const { return ext_defi_names_; }

bool ShaderResource::inject_external_definition(std::string_view ext_defi_name, std::string&& defi_content) {
  static const auto kSingleLineCommentRegExp = std::regex(std::string(shader_macros::kSingleLineCommentRegExpStr));

  auto it = std::ranges::find(ext_defi_names_, ext_defi_name);
  if (it == ext_defi_names_.end()) {
    resin::Logger::warn("Shader resource could not find external definition named \"{}\"", ext_defi_name);
    return false;
  }

  // External definitions rely on #define macro, single-line comments are problematic
  std::regex_replace(defi_content, kSingleLineCommentRegExp, " ");

  // Older GLSL compilers may not support line continuation characters in #define macros, so for maximum compatibility
  // they should be avoided
  // (https://stackoverflow.com/questions/16426105/does-opengl-and-opengl-es-support-preprocessor-line-continuation-characters)
  // For that reason the newline symbols are replaced with space characters
  std::ranges::replace_if(defi_content, [](auto&& c) { return c == '\n' or c == '\r'; }, ' ');

  ext_defi_contents_[*it] = std::move(defi_content);
  is_dirty_               = true;
  return true;
}

bool ShaderResource::is_glsl_ready() const { return ext_defi_contents_.size() == ext_defi_names_.size(); }

std::optional<std::string_view> ShaderResource::glsl() const {
  if (!is_glsl_ready()) {
    return std::nullopt;
  }

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
  glsl_.append(intermediate_glsl_);

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

  std::ifstream file_stream(path);
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
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kExtDefiMacro), 0, 1, curr_line));
  }
  auto arg = std::string_view{*it};

  ++it;
  if (it != end) {
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kVersionMacro), 1, 2, curr_line));
  }

  if (!arg.starts_with("\"") || !arg.ends_with("\"") || arg.size() < 2) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path, "The include macro argument should begin and end with `\"`.", curr_line));
  }

  auto arg_val  = std::string_view{arg.substr(1, arg.size() - 2)};
  auto rel_path = std::filesystem::path{arg_val};
  if (rel_path.empty()) {
    clear_log_throw(
        ShaderInvalidMacroArgumentException(sh_path, "The include macro argument cannot be empty.", curr_line));
  }

  if (rel_path.is_absolute()) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path, "The include macro argument cannot be an absolute path.", curr_line));
  }

  auto dep_ext = resin::extension_to_shader_type(rel_path.extension().string());
  if (!dep_ext.has_value() || !std::holds_alternative<LibraryShaderType>(*dep_ext)) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path, "The include macro argument must be a library shader (.glsl extension).", curr_line));
  }

  auto abs_path = std::filesystem::is_directory(sh_path) ? sh_path / rel_path : sh_path.parent_path() / rel_path;
  if (abs_path == sh_path || std::ranges::find(visited_paths_, abs_path) != visited_paths_.end()) {
    clear_log_throw(ShaderIncludeMacroDependencyCycleException(sh_path, curr_line));
  }

  visited_paths_.push_back(abs_path);
  auto res = get_res_ptr(abs_path);
  visited_paths_.pop_back();

  content.append(res->intermediate_glsl());
  defi_names.insert(res->get_external_definition_names().begin(), res->get_external_definition_names().end());
}

void ShaderResourceManager::process_ext_defi_macro(const std::optional<std::filesystem::path>& sh_path,
                                                   WordsStringViewIterator& it, const WordsStringViewIterator& end,
                                                   size_t curr_line, std::unordered_set<std::string>& defi_names) {
  if (it == end) {
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kExtDefiMacro), 0, 1, curr_line));
  }
  auto arg = std::string_view{*it};

  ++it;
  if (it != end) {
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kVersionMacro), 1, 2, curr_line));
  }

  if (!std::ranges::all_of(arg, [](const char c) { return std::isalnum(c) != 0 || c == '_'; })) {
    clear_log_throw(ShaderInvalidMacroArgumentException(
        sh_path, "The external definition macro argument contains non-alphanumeric characters.", curr_line));
  }

  defi_names.emplace(arg);
}

const std::shared_ptr<const ShaderResource>& ShaderResourceManager::get_res_ptr(const std::filesystem::path& path) {
  auto elem = cache_.find(path);
  if (elem != cache_.end()) {
    Logger::info("Cache hit for path \"{}\".", path.string());
    return elem->second;
  }

  cache_[path] = std::make_shared<const ShaderResource>(parse_res(path));

  Logger::info("Loaded and cached resource with path \"{}\".", path.string());

  return cache_[path];
}

std::optional<std::string> ShaderResourceManager::process_version_macro(
    const std::optional<std::filesystem::path>& sh_path, const ShaderType& sh_type, WordsStringViewIterator& it,
    const WordsStringViewIterator& end, size_t curr_line) {
  if (std::holds_alternative<LibraryShaderType>(sh_type)) {
    resin::Logger::warn("Ignoring version macro in .glsl shader.");
    return std::nullopt;
  }

  if (it == end) {
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kVersionMacro), 0, 1, curr_line));
  }
  auto arg1 = std::string_view{*it};

  ++it;
  if (it != end) {
    auto arg2 = std::string_view{*it};

    ++it;
    if (it != end) {
      clear_log_throw(ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kVersionMacro), 2,
                                                                3, curr_line));
    }

    return std::format("{} {} {}", shader_macros::kVersionMacro, arg1, arg2);
  }

  return std::format("{} {}", shader_macros::kVersionMacro, arg1);
}

std::optional<std::string> ShaderResourceManager::process_name_macro(
    const std::optional<std::filesystem::path>& sh_path, const ShaderType& sh_type, WordsStringViewIterator& it,
    const WordsStringViewIterator& end, size_t curr_line) {
  if (!std::holds_alternative<SDFShaderType>(sh_type)) {
    resin::Logger::warn("Ignoring name macro in non .sdf shader.");
    return std::nullopt;
  }

  if (it == end) {
    clear_log_throw(
        ShaderMacroInvalidArgumentsCountException(sh_path, std::string(shader_macros::kVersionMacro), 0, 1, curr_line));
  }

  auto arg1 = std::string_view{*it};
  return std::format("{}", arg1);
}

void ShaderResourceManager::process_sdf_shader(ShaderType& sh_type, std::string& preprocessed_content,
                                               const std::string& name) {
  if (!std::holds_alternative<SDFShaderType>(sh_type)) {
    return;
  }

  auto sh_content = preprocessed_content;

  static auto sdf_signature_pattern = std::regex(
      R"(float[\s\r\n]*sdf[\s\r\n]*\([\s\r\n]*vec3[\s\r\n]*\w+[\s\r\n]*(\)|(,[\s\r\n]*float[\s\r\n]*\w+[\s\r\n]*){1,3}\))+[\s\r\n]*)");
  static auto sdf_argument_pattern = std::regex(R"(,[\s\r\n]*float[\s\r\n]*(\w+))");
  static auto sdf_name_pattern     = std::regex(R"(float[\s\r\n]*(sdf)[\s\r\n]*\()");
  static auto single_line_comment  = std::regex(std::string(shader_macros::kSingleLineCommentRegExpStr));

  // Single-line comments are problematic when injecting them as external definitions.
  std::regex_replace(sh_content, single_line_comment, " ");

  // Find the expected SDF signature
  auto iter      = std::sregex_iterator(sh_content.begin(), sh_content.end(), sdf_signature_pattern);
  const auto end = std::sregex_iterator();

  if (iter == end) {
    log_throw(SDFShaderInvalidFunctionSignature(std::string(name)));
  }
  auto sdf_signature_match = *iter;
  if (++iter != end) {
    log_throw(SDFShaderInvalidFunctionSignature(std::string(name)));
  }

  auto func_body_it = sh_content.begin() + sdf_signature_match.position() + sdf_signature_match.length();

  // Find the opening bracket
  while (func_body_it != sh_content.end() && *func_body_it != '{') {
    ++func_body_it;
  }
  auto open_bracket_it = func_body_it;

  if (func_body_it == sh_content.end()) {
    log_throw(SDFShaderNoFunctionBodyFound(std::string(name)));
  }

  // Find the closing bracket
  auto counter = 1;
  while (func_body_it != sh_content.end()) {
    if (*func_body_it == '{') {
      ++counter;
    }
    if (*func_body_it == '}') {
      --counter;
    }
    if (counter < 0) {
      // Closing bracket before opening bracket
      log_throw(SDFShaderNoFunctionBodyFound(std::string(name)));
    }
    if (counter == 0) {
      break;
    }
    ++func_body_it;
  }
  auto close_bracket_it = func_body_it;

  // Parse the SDF arguments
  iter = std::sregex_iterator(sh_content.begin() + sdf_signature_match.position(),
                              sh_content.begin() + sdf_signature_match.position() + sdf_signature_match.length(),
                              sdf_argument_pattern);

  auto sdf_args_count = 0U;

  StaticVector<std::string, sdf_shader_consts::kSDFMaxParamCount> sdf_args;
  for (; iter != end; ++iter, ++sdf_args_count) {
    sdf_args.emplace(iter->str(1));
  }

  // Find the arguments start
  iter = std::sregex_iterator(sh_content.begin() + sdf_signature_match.position(),
                              sh_content.begin() + sdf_signature_match.position() + sdf_signature_match.length(),
                              sdf_name_pattern);
  if (iter == end) {
    log_throw(SDFShaderNoFunctionBodyFound(std::string(name)));
  }
  auto args_start = static_cast<size_t>(sdf_signature_match.position() + iter->position() + iter->length());

  // Create the final sdf function
  auto glsl_sdf_name = std::format("{}_Id{}_SDF", name, shader_name_id_++);
  auto body_end      = static_cast<size_t>(std::distance(sh_content.begin(), close_bracket_it));
  auto sdf_func      = std::format("float {}({}", glsl_sdf_name, sh_content.substr(args_start, body_end));

  // Remove newlines
  auto r = std::ranges::remove_if(sdf_func, [](auto&& c) { return c == '\n' or c == '\r'; });
  sdf_func.erase(r.begin(), r.end());

  // Set the shader type properties
  sh_type = SDFShaderType{
      .glsl_sdf_name = std::move(glsl_sdf_name),
      .args          = std::move(sdf_args)  //
  };
  preprocessed_content = sdf_func;
}

ShaderResource ShaderResourceManager::parse_res(const std::filesystem::path& path) {
  auto sh_type = get_sh_type(path);
  auto content = load_content(path);

  return parse_res(content, std::move(sh_type), path);
}

ShaderResource ShaderResourceManager::parse_res(std::string_view shader_content, ShaderType&& shader_type,
                                                std::optional<std::filesystem::path> path) {
  auto lines = make_lines_view(shader_content) | std::views::enumerate;

  std::unordered_set<std::string> defi_names;
  std::string preprocessed_content;
  std::optional<std::string> version;
  std::string name;

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

    if (!std::visit([macro](const auto& t) { return t.is_macro_supported(macro); }, shader_type)) {
      clear_log_throw(UnsupportedShaderMacroException(shader_type, macro));
    }

    ++it;
    if (macro == shader_macros::kIncludeMacro) {
      if (!path) {
        clear_log_throw(ShaderIncludeMacroWithNoDirectoryException());
      }

      process_include_macro(*path, it, end, line, preprocessed_content, defi_names);
    } else if (macro == shader_macros::kVersionMacro) {
      if (version != std::nullopt) {
        continue;
      }
      version = process_version_macro(path, shader_type, it, end, line);
    } else if (macro == shader_macros::kNameMacro) {
      if (auto name_opt = process_name_macro(path, shader_type, it, end, line)) {
        name = *name_opt;
      }
    } else {
      process_ext_defi_macro(path, it, end, line, defi_names);
    }
  }

  if (!std::holds_alternative<LibraryShaderType>(shader_type) && !std::holds_alternative<SDFShaderType>(shader_type) &&
      !version.has_value()) {
    clear_log_throw(ShaderAbsentVersionException(path));
  }

  if (std::holds_alternative<SDFShaderType>(shader_type) && name.empty()) {
    clear_log_throw(ShaderAbsentNameException(path));
  }

  if (std::holds_alternative<SDFShaderType>(shader_type)) {
    process_sdf_shader(shader_type, preprocessed_content, name);
  }

  if (name.empty()) {
    name = std::format("UnnamedShader{}", shader_name_id_++);
  }

  return ShaderResource(std::string(shader_content), std::move(preprocessed_content), std::move(name),
                        std::move(shader_type), std::move(defi_names), std::move(version));
}

}  // namespace resin
