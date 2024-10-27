#include <libresin/core/transform.hpp>

namespace resin {

transform::transform(const glm::vec3 pos, const glm::quat rot,
                     const glm::vec3 scale)
    : pos_(pos), rot_(rot), scale_(scale) {}

transform::~transform() {}

}  // namespace resin
