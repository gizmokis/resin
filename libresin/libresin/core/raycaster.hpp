#ifndef RESIN_RAYCASTER_HPP
#define RESIN_RAYCASTER_HPP

#include <glad/gl.h>

#include <array>

namespace resin {

class Raycaster {
 public:
  Raycaster();
  ~Raycaster();

  void bind() const;
  void unbind() const;

  void draw_call() const;

  Raycaster(const Raycaster&)            = delete;
  Raycaster(Raycaster&&)                 = delete;
  Raycaster& operator=(const Raycaster&) = delete;
  Raycaster& operator=(Raycaster&&)      = delete;

 private:
  GLuint vertex_array_, vertex_buffer_, index_buffer_;

  static constexpr std::array<float, 4 * 3> kVertices = {
      -1.F, -1.F, 0.F,  //
      1.F,  -1.F, 0.F,  //
      -1.F, 1.F,  0.F,  //
      1.F,  1.F,  0.F   //
  };

  static constexpr std::array<GLuint, 2 * 3> kIndices = {
      0, 1, 2,  //
      1, 3, 2   //
  };
};

}  // namespace resin

#endif  // RESIN_RAYCASTER_HPP