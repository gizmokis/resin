#ifndef RESIN_LIGHT_HPP
#define RESIN_LIGHT_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/transform.hpp>
#include <libresin/utils/macros.hpp>
#include <memory>

namespace resin {

struct BaseLight {
  explicit BaseLight(const glm::vec3& _color) : color(_color) {}
  virtual ~BaseLight() = default;

  Transform transform;
  glm::vec3 color;
};

struct DirectionalLight : public BaseLight {
  explicit DirectionalLight(const glm::vec3& _color = glm::vec3(1.0F, 1.0F, 1.0F), float _ambient_impact = 1.0F)
      : BaseLight(_color), ambient_impact(_ambient_impact) {}

  float ambient_impact;
};

struct PointLight : public BaseLight {
  struct Attenuation {
    explicit Attenuation(float _constant = 1.0F, float _linear = 1.0F, float _quadratic = 1.0F)
        : constant(_constant), linear(_linear), quadratic(_quadratic) {}

    float constant, linear, quadratic;
  };

  explicit PointLight(const glm::vec3& _color         = glm::vec3(1.0F, 1.0F, 1.0F),
                      const glm::vec3& _pos           = glm::vec3(1.0F, 1.0F, 1.0F),
                      const Attenuation& _attenuation = Attenuation())
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
  virtual ~ILightSceneComponentVisitor() = default;

  virtual void visit_point_light(LightSceneComponent<PointLight>& point_light)             = 0;
  virtual void visit_directional_light(LightSceneComponent<DirectionalLight>& point_light) = 0;
};

using LightId = Id<BaseLight>;
class BaseLightSceneComponent {
 public:
  virtual ~BaseLightSceneComponent() = default;
  explicit BaseLightSceneComponent(IdRegistry<BaseLight>& id_registry) : light_id_(id_registry) {}

  DISABLE_COPY_AND_MOVE(BaseLightSceneComponent)

  IdView<LightId> light_id() { return light_id_; }
  std::string_view name() const { return name_; }
  void rename(std::string&& name) { name_ = std::move(name); }

  virtual BaseLight& light_base()                                   = 0;
  virtual const BaseLight& light_base() const                       = 0;
  virtual void accept_visitor(ILightSceneComponentVisitor& visitor) = 0;

 private:
  LightId light_id_;
  std::string name_;
};

template <LightConcept Light>
class LightSceneComponent : public BaseLightSceneComponent {
 public:
  LightSceneComponent(IdRegistry<BaseLight>& id_registry, std::unique_ptr<Light> light)
      : BaseLightSceneComponent(id_registry), light_(std::move(light)) {}

  DISABLE_COPY_AND_MOVE(LightSceneComponent)

  BaseLight& light_base() override { return *light_; }
  const BaseLight& light_base() const override { return *light_; }
  Light& light() { return *light_; }
  const Light& light() const { return *light_; }

  void accept_visitor(ILightSceneComponentVisitor& visitor) override {
    if constexpr (std::is_same_v<Light, PointLight>) {
      visitor.visit_point_light(*this);
    } else if constexpr (std::is_same_v<Light, DirectionalLight>) {
      visitor.visit_directional_light(*this);
    }
  }

 private:
  std::unique_ptr<Light> light_;
};

}  // namespace resin

#endif  // RESIN_LIGHT_HPP
