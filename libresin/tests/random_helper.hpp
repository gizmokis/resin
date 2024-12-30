#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <libresin/core/material.hpp>
#include <libresin/core/transform.hpp>
#include <random>

inline std::mt19937& get_random_gen() {
  static std::mt19937 generator(42);
  return generator;
}

inline float random_float(float min = -10.0F, float max = 10.0F) {
  std::uniform_real_distribution<float> distribution(min, max);
  return distribution(get_random_gen());
}

inline glm::vec3 random_vec3(float min = -10.0F, float max = 10.0F) {
  return glm::vec3(random_float(min, max), random_float(min, max), random_float(min, max));
}

inline glm::quat random_rot_quat() {
  glm::vec3 axis = glm::normalize(random_vec3(-1.0F, 1.0F));
  float angle    = random_float(0.0F, glm::pi<float>());
  return glm::angleAxis(angle, axis);
}

inline void randomize_transform(resin::Transform& transform) {
  transform.set_local_pos(random_vec3());
  transform.set_local_rot(random_rot_quat());
  transform.set_local_scale(random_float());
}

inline void randomize_material(resin::Material& material) {
  material.albedo           = random_vec3(0.0F, 1.0F);
  material.ambientFactor    = random_float(0.0F, 1.0F);
  material.diffuseFactor    = random_float(0.0F, 1.0F);
  material.specularFactor   = random_float(0.0F, 1.0F);
  material.specularExponent = random_float(0.0F, 1.0F);
}
