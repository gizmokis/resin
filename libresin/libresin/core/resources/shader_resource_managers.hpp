#ifndef SHADER_RESOURCE_MANAGERS_HPP
#define SHADER_RESOURCE_MANAGERS_HPP

#include <libresin/core/resources/shader_resource.hpp>
#include <memory>

namespace resin {

class ShaderResourceManagers {
 public:
  static std::shared_ptr<ShaderResourceManager> get_instance() {
    static std::shared_ptr<ShaderResourceManager> instance = std::make_shared<ShaderResourceManager>();
    return instance;
  }

 private:
  ShaderResourceManagers() = default;
};

}  // namespace resin

#endif  // SHADER_RESOURCE_MANAGERS_HPP
