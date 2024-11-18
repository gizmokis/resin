#include <glad/gl.h>

#include <libresin/core/shader.hpp>
#include <libresin/utils/logger.hpp>
#include <stdexcept>

namespace resin {
ShaderProgram::ShaderProgram(std::string_view name) : shader_name_(name), renderer_id_(glCreateProgram()) {
  if (renderer_id_ == 0) {
    throw std::runtime_error(std::format("Unable to create shader {}", name));
  }
}

GLint ShaderProgram::get_uniform_location(std::string_view name) const {
  auto it = uniform_locations_.find(name);
  if (it != uniform_locations_.end()) {
    return it->second;
  }

  const std::string key(name);
  const GLint location    = glGetUniformLocation(renderer_id_, key.c_str());
  uniform_locations_[key] = location;
  if (location == -1) {
    Logger::err(R"(Unable to find uniform "{}" in shader "{}")", name, shader_name_);
    return location;
  }

  Logger::debug(R"(Caching new uniform location: "{}" = {})", name, location);
  return location;
}
}  // namespace resin