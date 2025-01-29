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

void Scene::clear() {
  tree_.clear();
  lights_.clear();
}

void Scene::set_default() {
  clear();

  auto& m1 = tree().add_material(Material(glm::vec3(0.25F, 0.25F, 0.96F)));
  auto& m2 = tree().add_material(Material(glm::vec3(0.96F, 0.25F, 0.25F)));
  tree().add_material(Material(glm::vec3(1.0F, 1.0F, 0.0F)));
  tree().add_material(Material(glm::vec3(0.0F, 1.0F, 0.0F)));
  tree().add_material(Material(glm::vec3(1.0F, 0.0F, 1.0F)));

  tree().root().push_back_child<SphereNode>(SDFBinaryOperation::SmoothUnion).set_material(m1.material_id());
  auto& group = tree().root().push_back_child<GroupNode>(SDFBinaryOperation::SmoothUnion);
  group.push_back_child<CubeNode>(SDFBinaryOperation::SmoothUnion).transform().set_local_pos(glm::vec3(1, 1, 0));
  group.push_back_child<CubeNode>(SDFBinaryOperation::SmoothUnion).transform().set_local_pos(glm::vec3(-1, -1, 0));
  group.set_material(m2.material_id());

  add_light<DirectionalLight>();
  add_light<PointLight>();
}

}  // namespace resin
