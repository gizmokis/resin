#ifndef RESIN_EXCEPTIONS_HPP
#define RESIN_EXCEPTIONS_HPP
#include <libresin/utils/logger.hpp>
#include <source_location>
#include <stdexcept>
#include <type_traits>

#define EXCEPTION_NAME(TypeName) \
  static constexpr std::string_view name() { return #TypeName; }

namespace resin {

template <typename T>
concept ExceptionConcept = std::is_base_of_v<std::runtime_error, T> && requires {
  { T::name() } -> std::convertible_to<std::string_view>;
};

template <ExceptionConcept Exception>
[[noreturn]] void inline log_throw(Exception&& e, const std::source_location& loc = std::source_location::current()) {
  resin::Logger::get_instance().log(LogLevel::Throw, false, loc, "{}: {}", Exception::name(), e.what());
  throw std::forward<Exception>(e);
}

class ResinException : public std::runtime_error {
 public:
  explicit ResinException(const std::string& arg) : std::runtime_error(arg) {}
  explicit ResinException(const char* arg) : std::runtime_error(arg) {}
};

class FileDoesNotExistException : public std::runtime_error {
 public:
  EXCEPTION_NAME(FileDoesNotExistException)

  explicit FileDoesNotExistException(std::string&& file_path)
      : std::runtime_error(std::format(R"(File "{}" does not exist.)", file_path)), file_path_(std::move(file_path)) {}

  inline const std::string& get_file_path() const { return file_path_; }

 private:
  std::string file_path_;
};

class InvalidFileTypeException : public ResinException {
 public:
  EXCEPTION_NAME(InvalidFileTypeException)

  explicit InvalidFileTypeException(std::string&& file_path, std::string&& msg)
      : ResinException(std::format(R"(File "{}" has invalid type. {})", file_path, msg)),
        file_path_(std::move(file_path)),
        msg_(std::move(msg)) {}

  inline const std::string& get_file_path() const { return file_path_; }
  inline const std::string& get_msg() const { return msg_; }

 private:
  std::string file_path_;
  std::string msg_;
};

class FileStreamNotAvailableException : public ResinException {
 public:
  EXCEPTION_NAME(FileStreamNotAvailableException)

  explicit FileStreamNotAvailableException(std::string&& file_path)
      : ResinException(std::format(R"(File stream is not available for "{}")", file_path)),
        file_path_(std::move(file_path)) {}

  inline const std::string& get_file_path() const { return file_path_; }

 private:
  std::string file_path_;
  std::string error_details_;
};

class FileExtensionNotSupportedException : public ResinException {
 public:
  EXCEPTION_NAME(FileExtensionNotSupportedException)

  explicit FileExtensionNotSupportedException(std::string&& file_path, std::string&& extension)
      : ResinException(std::format(R"(File extension "{}" is not supported for "{}".)", extension, file_path)),
        file_path_(std::move(file_path)),
        extension_(std::move(extension)) {}

  inline const std::string& get_file_path() const { return file_path_; }
  inline const std::string& get_extension() const { return extension_; }

 private:
  std::string file_path_;
  std::string extension_;
};

class ShaderMacroInvalidArgumentsCountException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderMacroInvalidArgumentsCountException)

  explicit ShaderMacroInvalidArgumentsCountException(std::string&& sh_path, std::string&& macro_name,
                                                     size_t expected_args, size_t actual_args, size_t line)
      : ResinException(std::format(
            R"(Shader with path "{}" contains macro "{}" with invalid arguments count at line {}. Expected {}. Actual: {}.)",
            sh_path, macro_name, line, expected_args, actual_args)),
        sh_path_(std::move(sh_path)),
        macro_name_(std::move(macro_name)),
        expected_args_(expected_args),
        actual_args_(actual_args),
        line_(line) {}

  inline const std::string& get_sh_path() const { return sh_path_; }
  inline const std::string& get_macro_name() const { return macro_name_; }
  inline size_t expected_args() const { return expected_args_; }
  inline size_t actual_args() const { return actual_args_; }
  inline size_t line() const { return line_; }

 private:
  std::string sh_path_;
  std::string macro_name_;
  size_t expected_args_;
  size_t actual_args_;
  size_t line_;
};

class ShaderInvalidMacroArgumentException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderInvalidMacroArgumentException)

  explicit ShaderInvalidMacroArgumentException(std::string&& sh_path, std::string&& msg, size_t line)
      : ResinException(std::format(R"(Shader with path "{}" contains macro at line {} with invalid argument. {})",
                                   sh_path, line, msg)),
        sh_path_(std::move(sh_path)),
        msg_(std::move(msg)),
        line_(line) {}

  inline const std::string& get_sh_path() const { return sh_path_; }
  inline const std::string& get_msg() const { return msg_; }
  inline size_t get_line() const { return line_; }

 private:
  std::string sh_path_;
  std::string msg_;
  size_t line_;
};

class ShaderIncludeMacroDependencyCycleException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderIncludeMacroDependencyCycleException)

  explicit ShaderIncludeMacroDependencyCycleException(std::string&& sh_path, size_t line)
      : ResinException(std::format(R"(Detected dependency cycle in shader with path "{}" at line {}.)", sh_path, line)),
        sh_path_(std::move(sh_path)),
        line_(line) {}

  inline const std::string& get_sh_path() const { return sh_path_; }
  inline size_t get_line() const { return line_; }

 private:
  std::string sh_path_;
  size_t line_;
};

class ShaderAbsentVersionException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderAbsentVersionException)

  explicit ShaderAbsentVersionException(std::string&& sh_path)
      : ResinException(std::format(R"(Could not find version macro for a shader with path "{}".)", sh_path)),
        sh_path_(std::move(sh_path)) {}

  inline const std::string& get_sh_path() const { return sh_path_; }

 private:
  std::string sh_path_;
};

class ShaderTypeMismatchException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderTypeMismatchException)

  explicit ShaderTypeMismatchException(std::string_view type, std::string shader_name, std::string_view actual)
      : ResinException(std::format(R"({} "{}" creation failed! Actual type: {})", type, shader_name, actual)),
        shader_type_(type),
        shader_name_(std::move(shader_name)),
        actual_(actual) {}

  inline const std::string& get_shader_type() const { return shader_type_; }
  inline const std::string& get_shader_name() const { return shader_name_; }
  inline const std::string& get_actual() const { return actual_; }

 private:
  std::string shader_type_;
  std::string shader_name_;
  std::string actual_;
};

class ShaderProgramLinkingException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderProgramLinkingException)

  explicit ShaderProgramLinkingException(std::string shader_name, std::string&& reason)
      : ResinException(std::format(R"(Shader program "{}" linking failed! Reason: {})", shader_name, reason)),
        shader_name_(std::move(shader_name)),
        reason_(std::move(reason)) {}

  inline const std::string& get_shader_name() const { return shader_name_; }
  inline const std::string& get_reason() const { return reason_; }

 private:
  std::string shader_name_;
  std::string reason_;
};

class ShaderProgramValidationException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderProgramValidationException)

  explicit ShaderProgramValidationException(std::string shader_name, std::string&& reason)
      : ResinException(std::format(R"(Shader program "{}" validation failed! Reason: {})", shader_name, reason)),
        shader_name_(std::move(shader_name)),
        reason_(std::move(reason)) {}

  inline const std::string& get_shader_name() const { return shader_name_; }
  inline const std::string& get_reason() const { return reason_; }

 private:
  std::string shader_name_;
  std::string reason_;
};

class ShaderCreationException : public ResinException {
 public:
  EXCEPTION_NAME(ShaderCreationException)

  explicit ShaderCreationException(std::string_view type, std::string shader_name, std::string&& reason)
      : ResinException(std::format(R"({} "{}" creation failed! Reason: {})", type, shader_name, reason)),
        shader_type_(type),
        shader_name_(std::move(shader_name)),
        reason_(std::move(reason)) {}

  inline const std::string& get_shader_type() const { return shader_type_; }
  inline const std::string& get_shader_name() const { return shader_name_; }
  inline const std::string& get_reason() const { return reason_; }

 private:
  std::string shader_type_;
  std::string shader_name_;
  std::string reason_;
};

class SDFTreeNodeDoesNotExist : public ResinException {
 public:
  EXCEPTION_NAME(SDFTreeNodeDoesNotExist)

  explicit SDFTreeNodeDoesNotExist(size_t id)
      : ResinException(std::format(R"(SDF Tree node with id {} does not exist)", id)), id_(id) {}

  inline size_t get_id() const { return id_; }

 private:
  size_t id_;
};

class MaterialSDFTreeComponentDoesNotExist : public ResinException {
 public:
  EXCEPTION_NAME(MaterialDoesNotExist)

  explicit MaterialSDFTreeComponentDoesNotExist(size_t id)
      : ResinException(std::format(R"(Material with id {} does not exist)", id)), id_(id) {}

  inline size_t get_id() const { return id_; }

 private:
  size_t id_;
};

class DefaultMaterialDeletionAttempted : public ResinException {
 public:
  EXCEPTION_NAME(DefaultMaterialDeletionAttempted)

  DefaultMaterialDeletionAttempted() : ResinException("Detected an attempt to delete the default material.") {}
};

class ObjectsOverflowException : public ResinException {
 public:
  EXCEPTION_NAME(ObjectsOverflowException)

  explicit ObjectsOverflowException() : ResinException(std::format(R"(No more free IDs available)")) {}
};

class SDFTreeEmptyGroupException : public ResinException {
 public:
  EXCEPTION_NAME(SDFTreeEmptyGroupException)

  explicit SDFTreeEmptyGroupException(size_t group_id)
      : ResinException(std::format(
            R"(Cannot generate shader as there is a group node with id {} that does not have any children)", group_id)),
        group_id_(group_id) {}

  inline size_t get_group_id() const { return group_id_; }

 private:
  size_t group_id_;
};

class SDFTreeRootDeletionError : public ResinException {
 public:
  EXCEPTION_NAME(SDFTreeRootDeletionError)

  explicit SDFTreeRootDeletionError() : ResinException(std::format(R"(SDF Tree root must not be deleted)")) {}
};

class SDFTreeNodeIsNotAChild : public ResinException {
 public:
  EXCEPTION_NAME(SDFTreeNodeIsNotAChild)

  explicit SDFTreeNodeIsNotAChild(size_t id1, size_t id2)
      : ResinException(std::format(R"(SDF Tree node with id {} is not a child of the parent with id {})", id1, id2)),
        id1_(id1),
        id2_(id2) {}

  inline size_t get_id1() const { return id1_; }
  inline size_t get_id2() const { return id2_; }

 private:
  size_t id1_;
  size_t id2_;
};

class SDFTreeReachedDirtyPrimitivesLimit : public ResinException {
 public:
  EXCEPTION_NAME(SDFTreeReachedDirtyPrimitivesLimit)

  explicit SDFTreeReachedDirtyPrimitivesLimit()
      : ResinException(std::format(R"(SDF Tree reached the dirty primitives limit)")) {}
};

class JSONSerializationException : public ResinException {
 public:
  EXCEPTION_NAME(JSONSerializationException)

  explicit JSONSerializationException(std::string_view msg)
      : ResinException(std::format("JSON serialization failed: {}", msg)) {}
};

class JSONDeserializationException : public ResinException {
 public:
  EXCEPTION_NAME(JSONDeserializationException)

  explicit JSONDeserializationException(std::string_view msg)
      : ResinException(std::format("JSON deserialization failed: {}", msg)) {}
};

class InvalidJSONSchemaException : public ResinException {
 public:
  EXCEPTION_NAME(InvalidJSONSchemaException)

  explicit InvalidJSONSchemaException(std::string_view msg)
      : ResinException(std::format("Provided JSON Schema is invalid: {}", msg)) {}
};

class InvalidJSONException : public ResinException {
 public:
  EXCEPTION_NAME(InvalidJSONException)

  explicit InvalidJSONException() : ResinException("Provided JSON is not a valid json") {}
};

class NotImplementedException : public ResinException {
 public:
  EXCEPTION_NAME(NotImplementedException)

  explicit NotImplementedException() : ResinException(std::format(R"(Not implemented yet)")) {}
};

class NonExhaustiveEnumException : public ResinException {
 public:
  EXCEPTION_NAME(NonExhaustiveEnumException)

  explicit NonExhaustiveEnumException() : ResinException(std::format(R"(Enum is not exhaustive.)")) {}
};

}  // namespace resin

#endif  // RESIN_EXCEPTIONS_HPP
