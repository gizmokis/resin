#include <glad/gl.h>

#include <chrono>
#include <libresin/core/shader.hpp>
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
}

void ShaderProgram::link_program() {
  glLinkProgram(program_id_);

  GLint status = 0;
  glGetProgramiv(program_id_, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length = 0;
    glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &length);

    std::string info(static_cast<GLuint>(length), '\0');
    glGetProgramInfoLog(program_id_, length, &length, info.data());

    log_throw(ShaderProgramLinkingException(shader_name_, std::move(info)));
  }

  glValidateProgram(program_id_);

  status = 0;
  glGetProgramiv(program_id_, GL_VALIDATE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length = 0;
    glGetProgramiv(program_id_, GL_INFO_LOG_LENGTH, &length);

    std::string info(static_cast<GLuint>(length), '\0');
    glGetProgramInfoLog(program_id_, length, &length, info.data());

    log_throw(ShaderProgramValidationException(shader_name_, std::move(info)));
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

  GLint status = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

    std::string info(static_cast<GLuint>(length), '\0');
    glGetShaderInfoLog(shader, length, &length, info.data());

    log_throw(ShaderCreationException(get_shader_type_name(type), shader_name_, std::move(info)));
  }

  return shader;
}

RenderingShaderProgram::RenderingShaderProgram(std::string_view name, ShaderResource vertex_resource,
                                               ShaderResource fragment_resource)
    : ShaderProgram(name), vertex_shader_(std::move(vertex_resource)), fragment_shader_(std::move(fragment_resource)) {
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