#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/sdf_tree.hpp>
#include <string>

namespace resin {

using SDFTreeNodeId = Id<SDFTreeNode>;

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const               = 0;
  virtual void accept_visitor(ISDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                     = 0;
  virtual void rename(std::string&&)                        = 0;

  virtual ~SDFTreeNode() = default;

  SDFTreeNode() = delete;
  explicit SDFTreeNode(const std::shared_ptr<SDFTreeRegistry>& tree)
      : node_id_(tree->nodes_registry_), transform_id_(tree->transform_component_registry_), tree_(tree) {}

  inline IdView<SDFTreeNodeId> node_id() const { return node_id_; }
  inline IdView<TransformId> transform_component_id() const { return transform_id_; }
  inline Transform& transform() { return transform_; }

  // TODO(migoox): add material component

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
  std::shared_ptr<SDFTreeRegistry> tree_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;
}  // namespace resin

#endif
