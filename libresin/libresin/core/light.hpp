#ifndef RESIN_LIGHT_HPP
#define RESIN_LIGHT_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/macros.hpp>

namespace resin {

struct BaseLight {
  explicit BaseLight(const glm::vec3& _color) : color(_color) {}
  virtual ~BaseLight() = default;

  Transform transform;
  glm::vec3 color;
};

struct DirectionalLight : public BaseLight {
  DirectionalLight(const glm::vec3& _color, float _ambient_impact)
      : BaseLight(_color), ambient_impact(_ambient_impact) {}

  float ambient_impact;
};

struct PointLight : public BaseLight {
  struct Attenuation {
    Attenuation(float _constant, float _linear, float _quadratic)
        : constant(_constant), linear(_linear), quadratic(_quadratic) {}

    float constant, linear, quadratic;
  };

  PointLight(const glm::vec3& _color, const glm::vec3& _pos, const Attenuation& _attenuation)
      : BaseLight(_color), attenuation(_attenuation) {
    transform.set_local_pos(_pos);
  }

  Attenuation attenuation;
};

template <typename L>
concept LightConcept = std::derived_from<L, BaseLight>;

template <LightConcept Light>
class LightSceneComponent;

class ILightSceneComponentVisitor {
 public:
  void visit_point_light(LightSceneComponent<PointLight>& point_light);
  void visit_directional_light(LightSceneComponent<DirectionalLight>& point_light);
};

using LightId = Id<BaseLight>;
class BaseLightSceneComponent {
 public:
  virtual ~BaseLightSceneComponent() = default;
  explicit BaseLightSceneComponent(IdRegistry<BaseLight>& id_registry) : light_id_(id_registry) {}

  DISABLE_COPY_AND_MOVE(BaseLightSceneComponent)

  IdView<LightId> light_id() { return light_id_; }
  std::string_view name() { return name_; }
  void rename(std::string&& name) { name_ = std::move(name); }

  virtual BaseLight& light_base()                                   = 0;
  virtual void accept_visitor(ILightSceneComponentVisitor& visitor) = 0;

 private:
  LightId light_id_;
  std::string name_;
};

template <LightConcept Light>
class LightSceneComponent : public BaseLightSceneComponent {
 public:
  LightSceneComponent(IdRegistry<BaseLight>& id_registry, Light light)
      : BaseLightSceneComponent(id_registry), light_(light) {}

  DISABLE_COPY_AND_MOVE(LightSceneComponent)

  BaseLight& light_base() override { return light_; }
  Light& light() { return light_; }

  void accept_visitor(ILightSceneComponentVisitor& visitor) override {
    if constexpr (std::is_same_v<Light, PointLight>) {
      visitor.visit_point_light(*this);
    } else if constexpr (std::is_same_v<Light, DirectionalLight>) {
      visitor.visit_directional_light(*this);
    }
  }

 private:
  Light light_;
};

}  // namespace resin

#endif  // RESIN_LIGHT_HPP
