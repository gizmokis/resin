#ifndef RESIN_FRAMEBUFFER_HPP
#define RESIN_FRAMEBUFFER_HPP

#include <glad/gl.h>

#include <cstddef>

namespace resin {

class Framebuffer {
 public:
  Framebuffer(size_t width, size_t height);
  virtual ~Framebuffer();

  void bind() const;
  void unbind() const;  // remember to glVieport after

  virtual void clear()                             = 0;
  virtual void resize(size_t width, size_t height) = 0;

  inline size_t width() const { return width_; }
  inline size_t height() const { return height_; }

 protected:
  void start_init() const;
  void end_init() const;

 protected:
  size_t width_, height_;
  GLuint framebuffer_id_;
};

class ViewportFramebuffer : public Framebuffer {
 public:
  ViewportFramebuffer(size_t width, size_t height);
  virtual ~ViewportFramebuffer();

  void begin_pick_render() const;
  void end_pick_render() const;

  int sample_mouse_pick(size_t x, size_t y) const;

  void clear() override;
  void resize(size_t width, size_t height) override;

  inline GLuint color_texture() const { return color_attachment_texture_; }

 private:
  GLuint color_attachment_texture_, mouse_pick_attachment_texture_;
  GLuint depth_renderbuffer_;
};

class ImageFramebuffer : public Framebuffer {
 public:
  ImageFramebuffer(size_t width, size_t height);
  virtual ~ImageFramebuffer();

  void clear() override;
  void resize(size_t width, size_t height) override;

  inline GLuint color_texture() const { return color_attachment_texture_; }

 private:
  GLuint color_attachment_texture_;
};

}  // namespace resin

#endif  // RESIN_FRAMEBUFFER_HPP
