#ifndef RESIN_SDF_TREE_NODE_HPP
#define RESIN_SDF_TREE_NODE_HPP

#include <libresin/core/id_registry.hpp>
#include <libresin/core/sdf_tree/sdf_tree_node_visitor.hpp>
#include <libresin/core/transform.hpp>
#include <string>

namespace resin {

class SDFTreeRegistry;

class SDFTreeNode;
using SDFTreeNodeId = Id<SDFTreeNode>;

class SDFTreeNode {
 public:
  virtual std::string gen_shader_code() const               = 0;
  virtual void accept_visitor(ISDFTreeNodeVisitor& visitor) = 0;
  virtual std::string_view name() const                     = 0;
  virtual void rename(std::string&&)                        = 0;

  SDFTreeNode() = delete;

  inline IdView<SDFTreeNodeId> node_id() const { return node_id_; }
  inline IdView<TransformId> transform_component_id() const { return transform_id_; }
  inline Transform& transform() { return transform_; }

  // It is the programmer's responsibility to assert that SDFTreeNode class will not outlive the provided registry!
  explicit SDFTreeNode(SDFTreeRegistry& tree);

  virtual ~SDFTreeNode();

  // TODO(migoox): add material component

 protected:
  SDFTreeNodeId node_id_;
  TransformId transform_id_;
  Transform transform_;
  std::reference_wrapper<SDFTreeRegistry> tree_registry_;
};

template <typename T>
concept SDFTreeNodeConcept = std::is_base_of_v<SDFTreeNode, T>;

}  // namespace resin

#endif
