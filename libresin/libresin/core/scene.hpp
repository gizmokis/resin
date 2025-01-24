#ifndef RESIN_SCENE_HPP
#define RESIN_SCENE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/light.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/utils/exceptions.hpp>
#include <memory>
#include <unordered_map>

namespace resin {

class Scene {
 public:
  using LightsMap = std::unordered_map<IdView<LightId>, std::unique_ptr<BaseLightSceneComponent>, IdViewHash<LightId>,
                                       std::equal_to<>>;

  template <LightConcept Light, typename... Args>
    requires std::constructible_from<Light, IdRegistry<BaseLight>&, Args...>
  void add_light(Args&&... args) {
    auto light_ptr = std::make_unique(lights_registry_, std::forward<Args>(args)...);
    lights_.emplace(light_ptr->id, std::move(light_ptr));
  }

  const LightsMap& lights_map() const { return lights_; }
  LightsMap& lights_map() { return lights_; }
  SDFTree& tree() { return tree_; }

 private:
  LightsMap lights_;
  IdRegistry<BaseLight> lights_registry_;

  SDFTree tree_;
};

}  // namespace resin

#endif
