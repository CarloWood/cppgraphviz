#pragma once

#include "Node.h"

namespace cppgraphviz {

class LabelNode : public Node
{
 protected:
  std::string label_;

  void item_attributes(dot::AttributeList& list) override
  {
    // Derive from LabelNode and override item_attributes to add a shape, color etc.
    // Call set_label to set the label, or derive from LabelNode and override item_attributes to add a label.
    if (label_.empty())
      list.add({"label", "<unknown LabelNode>"});
    else
      list += {"label", label_};
  }

 public:
  LabelNode(std::string_view what) : Node(what) { }
  LabelNode(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) : Node(root_graph, what) { }
  LabelNode(LabelNode&& label_node) : Node(std::move(label_node.do_wrlock()), this->noLock), label_(std::move(label_node.label_))
  {
    do_wrunlock();
  }
  LabelNode(LabelNode&& label_node, std::string_view what);
  LabelNode(LabelNode const& other) : Node(other.do_rdlock(), this->noLock), label_(other.label_)
  {
    do_rdunlock();
  }
  LabelNode(LabelNode const& other, std::string_view what);

  void set_label(std::string const& label)
  {
    label_ = label;
  }
};

} // namespace cppgraphviz
