#ifndef RESIN_SCENE_HPP
#define RESIN_SCENE_HPP

#include <libresin/core/light.hpp>
#include <libresin/core/sdf_tree/group_node.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>
#include <memory>
#include <unordered_map>

namespace resin {

class Scene {
 public:
  using LightsMap = std::unordered_map<IdView<LightId>, std::unique_ptr<BaseLightSceneComponent>, IdViewHash<LightId>,
                                       std::equal_to<>>;
  Scene();

  template <LightConcept Light, typename... Args>
    requires std::constructible_from<Light, Args...>
  LightSceneComponent<Light>& add_light(Args&&... args) {
    auto light_ptr  = std::make_unique<LightSceneComponent<Light>>(lights_registry_,
                                                                   std::make_unique<Light>(std::forward<Args>(args)...));
    auto& light_ref = *light_ptr;
    add_light(std::move(light_ptr));
    return light_ref;
  }

  void add_light(std::unique_ptr<BaseLightSceneComponent> light_ptr);

  BaseLightSceneComponent& add_light(LightType type);

  const LightsMap& lights() const { return lights_; }
  LightsMap& lights() { return lights_; }
  SDFTree& tree() { return tree_; }

  void set_default();
  void clear();

 private:
  IdRegistry<BaseLight> lights_registry_;
  LightsMap lights_;
  SDFTree tree_;
};

}  // namespace resin

#endif
