#ifndef RESIN_EXCEPTIONS_HPP
#define RESIN_EXCEPTIONS_HPP
#include <libresin/utils/logger.hpp>
#include <stdexcept>

#define EXCEPTION_NAME(TypeName) \
  static constexpr std::string_view name() { return #TypeName; }

namespace resin {

template <typename T>
concept ExceptionConcept = std::is_base_of_v<std::runtime_error, T> && requires {
  { T::name() } -> std::convertible_to<std::string_view>;
};

class FileDoesNotExistException : public std::runtime_error {
 public:
  EXCEPTION_NAME(FileDoesNotExistException)

  explicit FileDoesNotExistException(std::string file_path)
      : std::runtime_error(std::format(R"(File "{}" does not exist.)", file_path)), file_path_(std::move(file_path)) {}

  inline const std::string& get_file_path() const { return file_path_; }

 private:
  std::string file_path_;
};

class InvalidFileTypeException : public std::runtime_error {
 public:
  EXCEPTION_NAME(InvalidFileTypeException)

  explicit InvalidFileTypeException(std::string file_path, std::string msg)
      : std::runtime_error(std::format(R"(File "{}" has invalid type. {})", file_path, msg)),
        file_path_(std::move(file_path)),
        msg_(std::move(msg)) {}

  inline const std::string& get_file_path() const { return file_path_; }
  inline const std::string& get_msg() const { return msg_; }

 private:
  std::string file_path_;
  std::string msg_;
};

class FileStreamNotAvailableException : public std::runtime_error {
 public:
  EXCEPTION_NAME(FileStreamNotAvailableException)

  explicit FileStreamNotAvailableException(std::string file_path)
      : std::runtime_error(std::format(R"(File stream is not available for "{}")", file_path)),
        file_path_(std::move(file_path)) {}

  inline const std::string& get_file_path() const { return file_path_; }

 private:
  std::string file_path_;
  std::string error_details_;
};

class FileExtensionNotSupportedException : public std::runtime_error {
 public:
  EXCEPTION_NAME(FileExtensionNotSupportedException)

  explicit FileExtensionNotSupportedException(std::string file_path, std::string extension)
      : std::runtime_error(std::format(R"(File extension "{}" is not supported for "{}".)", extension, file_path)),
        file_path_(std::move(file_path)),
        extension_(std::move(extension)) {}

  inline const std::string& get_file_path() const { return file_path_; }
  inline const std::string& get_extension() const { return extension_; }

 private:
  std::string file_path_;
  std::string extension_;
};

class ShaderMacroInvalidArgumentsCountException : public std::runtime_error {
 public:
  EXCEPTION_NAME(ShaderMacroInvalidArgumentsCountException)

  explicit ShaderMacroInvalidArgumentsCountException(std::string sh_path, std::string macro_name, size_t expected_args,
                                                     size_t actual_args, size_t line)
      : std::runtime_error(std::format(
            R"(Shader with path "{}" contains macro "{}" with invalid arguments count at line {}. Expected {}. Actual: {}.)",
            sh_path, macro_name, line, expected_args, actual_args)),
        sh_path_(sh_path),
        macro_name_(macro_name),
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
class ShaderInvalidMacroArgumentException : public std::runtime_error {
 public:
  EXCEPTION_NAME(ShaderInvalidMacroArgumentException)

  explicit ShaderInvalidMacroArgumentException(std::string sh_path, std::string msg, size_t line)
      : std::runtime_error(std::format(R"(Shader with path "{}" contains macro at line {} with invalid argument. {})",
                                       sh_path, line, msg)),
        sh_path_(sh_path),
        msg_(msg),
        line_(line) {}

  inline const std::string& get_sh_path() const { return sh_path_; }
  inline const std::string& get_msg() const { return msg_; }
  inline size_t get_line() const { return line_; }

 private:
  std::string sh_path_;
  std::string msg_;
  size_t line_;
};

class ShaderIncludeMacroDependencyCycleException : public std::runtime_error {
 public:
  EXCEPTION_NAME(ShaderIncludeMacroDependencyCycleException)

  explicit ShaderIncludeMacroDependencyCycleException(std::string sh_path, size_t line)
      : std::runtime_error(
            std::format(R"(Detected dependency cycle in shader with path "{}" at line {}.)", sh_path, line)),
        sh_path_(std::move(sh_path)),
        line_(line) {}

  inline const std::string& get_sh_path() const { return sh_path_; }
  inline size_t get_line() const { return line_; }

 private:
  std::string sh_path_;
  size_t line_;
};

template <ExceptionConcept Exception, typename... Args>
[[noreturn]] void inline log_throw(Args&&... args)
  requires std::constructible_from<Exception, Args...>
{
  auto e = Exception(std::forward<Args>(args)...);
  resin::Logger::thrown("{}: {}.", Exception::name(), e.what());
  throw std::move(e);
}

}  // namespace resin

#endif  // RESIN_EXCEPTIONS_HPP
