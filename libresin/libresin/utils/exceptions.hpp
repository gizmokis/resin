#ifndef RESIN_EXCEPTIONS_HPP
#define RESIN_EXCEPTIONS_HPP
#include <stdexcept>

namespace resin {

template <typename T>
concept ResinExceptionConcept = std::is_base_of_v<std::runtime_error, T>;

class FileDoesNotExistException : public std::runtime_error {
 public:
  FileDoesNotExistException() : std::runtime_error("File does not exist.") {}
};

class InvalidFileTypeException : public std::runtime_error {
 public:
  InvalidFileTypeException() : std::runtime_error("File has invalid type.") {}
};

class FileStreamNotAvailableException : public std::runtime_error {
 public:
  FileStreamNotAvailableException() : std::runtime_error("File stream is not available.") {}
};

class FileExtensionNotSupportedException : public std::runtime_error {
 public:
  FileExtensionNotSupportedException() : std::runtime_error("File exception is not supported.") {}
};

class ShaderDependencyCycleException : public std::runtime_error {
 public:
  ShaderDependencyCycleException() : std::runtime_error("Detected dependency cycle during shader preprocessing.") {}
};

class ShaderInvalidMacroArgumentsException : public std::runtime_error {
 public:
  ShaderInvalidMacroArgumentsException() : std::runtime_error("Shader contains a macro with invalid arguments.") {}
};

}  // namespace resin

#endif  // RESIN_EXCEPTIONS_HPP
