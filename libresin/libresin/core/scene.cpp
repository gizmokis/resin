#include <libresin/core/scene.hpp>

namespace resin {

Scene::Scene() : lights_registry_(25) {}

BaseLightSceneComponent& Scene::add_light(LightType type) {
  switch (type) {
    case LightType::DirectionalLight:
      return add_light<DirectionalLight>();
    case LightType::PointLight:
      return add_light<PointLight>();
    case LightType::_Count:
      throw NonExhaustiveEnumException();
  }

  throw NonExhaustiveEnumException();
}

void Scene::add_light(std::unique_ptr<BaseLightSceneComponent> light_ptr) {
  lights_.emplace(light_ptr->light_id(), std::move(light_ptr));
}

}  // namespace resin
