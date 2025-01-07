#ifndef RESIN_FRAMEBUFFER_HPP
#define RESIN_FRAMEBUFFER_HPP

#include <glad/gl.h>

#include <array>
#include <cstddef>

namespace resin {

class Framebuffer {
 public:
  Framebuffer(size_t width, size_t height);
  ~Framebuffer();

  void bind() const;
  void unbind() const;  // remember to glViewport after!

  void resize(size_t width, size_t height);

  inline size_t width() const { return width_; }
  inline size_t height() const { return height_; }
  inline GLuint color_texture() const { return color_attachment_texture_; }

  int sample_mouse_pick(size_t x, size_t y) const;

 private:
  static constexpr std::array<GLenum, 2> kAttachments = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

  size_t width_, height_;
  GLuint framebuffer_id_;
  GLuint color_attachment_texture_, mouse_pick_attachment_texture_;
  GLuint depth_renderbuffer_;
};

}  // namespace resin

#endif  // RESIN_FRAMEBUFFER_HPP
