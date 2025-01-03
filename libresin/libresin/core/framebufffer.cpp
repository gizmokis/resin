#include <glad/gl.h>

#include <libresin/core/framebuffer.hpp>
#include <libresin/utils/exceptions.hpp>
#include <libresin/utils/logger.hpp>

namespace resin {

namespace util {

static void prepare_texture(GLenum format, GLint internal_format, GLsizei width, GLsizei height) {
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);
}

}  // namespace util

Framebuffer::Framebuffer(size_t width, size_t height)
    : width_(width), height_(height), framebuffer_id_(0), color_attachment_texture_(0), depth_renderbuffer_(0) {
  // Create the framebuffer
  glCreateFramebuffers(1, &framebuffer_id_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);

  // Setup color attachment
  glGenTextures(1, &color_attachment_texture_);
  glBindTexture(GL_TEXTURE_2D, color_attachment_texture_);
  util::prepare_texture(GL_RGBA, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_attachment_texture_, 0);

  // Setup depth attachment (without stencil)
  glGenRenderbuffers(1, &depth_renderbuffer_);
  glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height));
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer_);

  // Verify framebuffer creation
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_throw(FramebufferCreationException("Framebuffer is incomplete."));
  }

  // Bind default framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Framebuffer::~Framebuffer() {
  glDeleteRenderbuffers(1, &depth_renderbuffer_);
  glDeleteTextures(1, &color_attachment_texture_);
  glDeleteFramebuffers(1, &framebuffer_id_);
}

void Framebuffer::bind() const {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
  glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
}

void Framebuffer::unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }  // NOLINT

void Framebuffer::resize(size_t width, size_t height) {
  width_  = width;
  height_ = height;

  // Resize color attachment
  glBindTexture(GL_TEXTURE_2D, color_attachment_texture_);
  util::prepare_texture(GL_RGBA, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

  // Resize depth attachment
  glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, static_cast<GLsizei>(width),
                        static_cast<GLsizei>(height));
}

}  // namespace resin