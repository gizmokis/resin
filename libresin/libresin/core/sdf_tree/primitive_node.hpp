#ifndef RESIN_PRIMITIVE_BASE_NODE_HPP
#define RESIN_PRIMITIVE_BASE_NODE_HPP

#include <functional>
#include <libresin/core/sdf_shader_consts.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node.hpp>
#include <libresin/utils/optional_ref.hpp>
#include <libresin/utils/static_vector.hpp>
#include <optional>

namespace resin {
struct SDFPrimitiveTypeDescription;
class GroupNode;

struct PrimitiveNodeParam {
  std::string_view name;
  float value = 1.F;
};

class PrimitiveNode;
using PrimitiveNodeId = Id<PrimitiveNode>;

class PrimitiveNode final : public SDFTreeNode {
 public:
  using Params = StaticVector<PrimitiveNodeParam, sdf_shader_consts::kSDFMaxParamCount>;

  PrimitiveNode()                                = delete;
  PrimitiveNode(const PrimitiveNode&)            = delete;
  PrimitiveNode(PrimitiveNode&&)                 = delete;
  PrimitiveNode& operator=(const PrimitiveNode&) = delete;
  PrimitiveNode& operator=(PrimitiveNode&&)      = delete;

  /**
   * @brief Construct a new Primitive Node.
   *
   * @throw TooManySDFPrimitiveParameters Thrown when more than 3 SDF parameters are provided.
   *
   * @param tree
   * @param desc
   */
  explicit PrimitiveNode(SDFTreeRegistry& tree, std::optional<uint32_t> primitive_type_id = std::nullopt);

  explicit PrimitiveNode(SDFTreeRegistry& tree);
  ~PrimitiveNode() override;

  std::optional<uint32_t> type_id() const;
  void set_type_id(std::optional<uint32_t> new_type_id);
  optional_ref<const SDFPrimitiveTypeDescription> type() const;

  IdView<MaterialId> default_material_id() const { return tree_registry_.default_material.material_id(); }
  IdView<MaterialId> active_material_id_or_default() const {
    auto mat = active_material_id();
    return mat ? *mat : default_material_id();
  }

  void set_material(IdView<MaterialId> mat_id) final {
    mark_primitives_dirty();
    mat_id_ = mat_id;
  }

  void remove_material() final {
    mark_primitives_dirty();
    mat_id_ = std::nullopt;
  }

  void accept_visitor(ISDFTreeNodeVisitor& visitor) override {
    SDFTreeNode::accept_visitor(visitor);
    visitor.visit_primitive(*this);
  }

  [[nodiscard]] std::unique_ptr<SDFTreeNode> copy() final;

  bool is_leaf() final { return true; }
  IdView<PrimitiveNodeId> primitive_id() const { return prim_id_; }

  std::string gen_shader_code(GenShaderMode mode) const final;

  const Params& params() const { return params_; }
  Params& params() { return params_; }

 private:
  void update_id() const;

  void insert_leaves_to(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.emplace(node_id());
  }

  void remove_leaves_from(
      std::unordered_set<IdView<SDFTreeNodeId>, IdViewHash<SDFTreeNodeId>, std::equal_to<>>& leaves) final {
    leaves.erase(leaves.find(node_id()));
  }

  void push_dirty_primitives() final { tree_registry_.dirty_primitives.emplace(node_id()); }
  void set_ancestor_mat_id(IdView<MaterialId> mat_id) final { ancestor_mat_id_ = mat_id; }
  void remove_ancestor_mat_id() final { ancestor_mat_id_ = std::nullopt; }
  void delete_material_from_subtree(IdView<MaterialId> mat_id) final {
    tree_registry_.is_tree_dirty = true;
    if (mat_id == mat_id_) {
      mat_id_ = std::nullopt;
    }
  }

  void fix_material_ancestors() final;

  void update_glsl_args(size_t args_count, size_t prim_id);

 private:
  Params params_;
  std::string glsl_args_;

  PrimitiveNodeId prim_id_;

  mutable std::optional<uint32_t> type_id_;
};  // namespace resin

}  // namespace resin

#endif
