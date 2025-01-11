#ifndef RESIN_RESOURCE_MANAGERS_HPP
#define RESIN_RESOURCE_MANAGERS_HPP

#include <libresin/core/resources/shader_resource.hpp>

namespace resin {

class ResourceManagers {
 public:
  static ShaderResourceManager& shader_manager() {
    static ShaderResourceManager instance = ShaderResourceManager();
    return instance;
  }

 private:
  ResourceManagers() = default;
};

}  // namespace resin

#endif  // RESIN_RESOURCE_MANAGERS_HPP
