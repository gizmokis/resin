#include <glad/gl.h>

#include <chrono>
#include <format>
#include <libresin/core/resources/shader_resource.hpp>
#include <libresin/core/shader.hpp>
#include <libresin/core/uniform_buffer.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>
#include <stdexcept>

namespace resin {
ShaderProgram::ShaderProgram(std::string_view name) : shader_name_(name), program_id_(glCreateProgram()) {
  if (program_id_ == 0) {
    throw std::runtime_error(std::format("Unable to create shader {}", name));
  }
}

ShaderProgram::~ShaderProgram() { glDeleteProgram(program_id_); }

void ShaderProgram::bind() const { glUseProgram(program_id_); }

void ShaderProgram::unbind() const { glUseProgram(0); }  // NOLINT

void ShaderProgram::recompile() {
  using clock = std::chrono::high_resolution_clock;
  auto start  = clock::now();

  glDeleteProgram(program_id_);
  program_id_ = glCreateProgram();
  create_program();

  auto stop     = clock::now();
  auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
  Logger::debug("Shader {} recompilation took {}", shader_name_, duration);

  // Reconnect UBO bindings
  for (auto& pair : uniform_block_bindings_) {
    glUniformBlockBinding(program_id_, pair.first, pair.second);
  }
}

std::optional<std::string> ShaderProgram::get_shader_status(GLuint shader, GLenum type) {
  GLint status = 0;
  glGetShaderiv(shader, type, &status);
  if (status == GL_FALSE) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    std::string info(static_cast<GLuint>(length), '\0');
    glGetShaderInfoLog(shader, length, &length, info.data());
    return info;
  }

  return std::nullopt;
}

std::optional<std::string> ShaderProgram::get_program_status(GLuint program, GLenum type) {
  GLint status = 0;
  glGetProgramiv(program, type, &status);
  if (status == GL_FALSE) {
    GLint length = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

    std::string info(static_cast<GLuint>(length), '\0');
    glGetProgramInfoLog(program, length, &length, info.data());
    return info;
  }

  return std::nullopt;
}

void ShaderProgram::link_program() {
  glLinkProgram(program_id_);

  auto link_status = get_program_status(program_id_, GL_LINK_STATUS);
  if (link_status.has_value()) {
    log_throw(ShaderProgramLinkingException(shader_name_, std::move(link_status.value())));
  }

  glValidateProgram(program_id_);

  auto validate_status = get_program_status(program_id_, GL_VALIDATE_STATUS);
  if (validate_status.has_value()) {
    log_throw(ShaderProgramValidationException(shader_name_, std::move(validate_status.value())));
  }
}

GLint ShaderProgram::get_uniform_location(std::string_view name) const {
  auto it = uniform_locations_.find(name);
  if (it != uniform_locations_.end()) {
    return it->second;
  }

  const std::string key(name);
  const GLint location    = glGetUniformLocation(program_id_, key.c_str());
  uniform_locations_[key] = location;
  if (location == -1) {
    Logger::err(R"(Unable to find uniform "{}" in shader "{}")", name, shader_name_);
    return location;
  }

  Logger::debug(R"(Caching new uniform location: "{}" = {})", name, location);
  return location;
}

GLuint ShaderProgram::create_shader(const ShaderResource& resource, GLenum type) {
  const GLuint shader = glCreateShader(type);

  if (shader == 0) {
    log_throw(ShaderCreationException(get_shader_type_name(type), shader_name_, "unable to create shader."));
  }

  const GLchar* source = resource.get_glsl().c_str();
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  auto compile_status = get_shader_status(shader, GL_COMPILE_STATUS);
  if (compile_status.has_value()) {
    log_throw(ShaderCreationException(get_shader_type_name(type), shader_name_, std::move(compile_status.value())));
  }

  return shader;
}

void ShaderProgram::bind_uniform_buffer(std::string_view name, const UniformBuffer& ubo) const {
  const GLuint index = glGetUniformBlockIndex(program_id_, name.data());
  if (index == GL_INVALID_INDEX) {
    log_throw(ShaderProgramValidationException(
        shader_name_, std::format(R"(Unable to find uniform block "{}" in shader "{}")", name, shader_name_)));
  }

  GLint size = 0;
  glGetActiveUniformBlockiv(program_id_, index, GL_UNIFORM_BLOCK_DATA_SIZE, &size);

  // According to std140 standard the padding at the end of an array MAY be added
  // https://registry.khronos.org/OpenGL/specs/gl/glspec45.core.pdf#page=159
  if (size != static_cast<GLint>(ubo.buffer_size()) &&
      size != static_cast<GLint>(ubo.buffer_size_without_end_padding())) {
    log_throw(ShaderProgramValidationException(
        shader_name_, std::format(R"(Unexpected uniform block size: {}. Expected: {} or {})", size, ubo.buffer_size(),
                                  ubo.buffer_size_without_end_padding())));
  }

  uniform_block_bindings_[index] = static_cast<GLuint>(ubo.binding());
  glUniformBlockBinding(program_id_, index, uniform_block_bindings_[index]);
  Logger::debug(R"(Bound uniform block "{}" (index: {}, size: {} bytes) in shader "{}" to binding: {})", name, index,
                size, shader_name_, ubo.binding());
}

void ShaderProgram::bind_uniform_buffer(std::string_view name, size_t binding) const {
  const GLuint index = glGetUniformBlockIndex(program_id_, name.data());
  if (index == GL_INVALID_INDEX) {
    log_throw(ShaderProgramValidationException(
        shader_name_, std::format(R"(Unable to find uniform block "{}" in shader "{}")", name, shader_name_)));
  }

  uniform_block_bindings_[index] = static_cast<GLuint>(binding);
  glUniformBlockBinding(program_id_, index, uniform_block_bindings_[index]);
  Logger::debug(R"(Bound uniform block "{}" (index: {}) in shader "{}" to binding: {})", name, index, shader_name_,
                binding);
}

RenderingShaderProgram::RenderingShaderProgram(std::string_view name, ShaderResource vertex_resource,
                                               ShaderResource fragment_resource)
    : ShaderProgram(name), vertex_shader_(std::move(vertex_resource)), fragment_shader_(std::move(fragment_resource)) {
  if (vertex_shader_.get_type() != ShaderType::Vertex) {
    log_throw(ShaderTypeMismatchException(get_shader_type_name(GL_VERTEX_SHADER), shader_name_,
                                          vertex_shader_.get_extension()));
  }
  if (fragment_shader_.get_type() != ShaderType::Fragment) {
    log_throw(ShaderTypeMismatchException(get_shader_type_name(GL_FRAGMENT_SHADER), shader_name_,
                                          fragment_shader_.get_extension()));
  }
  using clock = std::chrono::high_resolution_clock;
  auto start  = clock::now();

  create_program();

  auto stop     = clock::now();
  auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
  Logger::info("Shader {} creation took {}", name, duration);
}

void RenderingShaderProgram::create_program() {
  GLuint vertex_shader   = create_shader(vertex_shader_, GL_VERTEX_SHADER);
  GLuint fragment_shader = create_shader(fragment_shader_, GL_FRAGMENT_SHADER);

  glAttachShader(program_id_, vertex_shader);
  glAttachShader(program_id_, fragment_shader);

  link_program();

  glDetachShader(program_id_, vertex_shader);
  glDetachShader(program_id_, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
}

ComputeShaderProgram::ComputeShaderProgram(std::string_view name, ShaderResource compute_shader)
    : ShaderProgram(name), compute_shader_(std::move(compute_shader)) {
  if (compute_shader_.get_type() != ShaderType::Compute) {
    log_throw(ShaderTypeMismatchException(get_shader_type_name(GL_COMPUTE_SHADER), shader_name_,
                                          compute_shader_.get_extension()));
  }

  using clock = std::chrono::high_resolution_clock;
  auto start  = clock::now();

  create_program();

  auto stop     = clock::now();
  auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
  Logger::info("Shader {} creation took {}", name, duration);
}

void ComputeShaderProgram::create_program() {
  GLuint compute_shader = create_shader(compute_shader_, GL_COMPUTE_SHADER);
  glAttachShader(program_id_, compute_shader);

  link_program();

  glDetachShader(program_id_, compute_shader);
  glDeleteShader(compute_shader);
}

}  // namespace resin
