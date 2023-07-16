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
  // Default constructor with a what argument.
  LabelNode(std::string_view what) : Node(what) { }
  // Also specify the root graph.
  LabelNode(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) : Node(root_graph, what) { }

  // Move constructors.
  LabelNode(threadsafe::LockFinalMove<LabelNode> other) : Node(std::move(other)), label_(std::move(other->label_)) { }
  LabelNode(LabelNode&& other) : LabelNode(threadsafe::LockFinalMove<LabelNode>{std::move(other)}) { }

  // Move constructors with what argument.
  LabelNode(threadsafe::LockFinalMove<LabelNode> other, std::string_view what);
  LabelNode(LabelNode&& other, std::string_view what) : LabelNode(threadsafe::LockFinalMove<LabelNode>{std::move(other)}, what) { }

  // Copy constructors.
  LabelNode(threadsafe::LockFinalCopy<LabelNode> other) : Node(other), label_(other->label_) { }
  LabelNode(LabelNode const& other) : LabelNode(threadsafe::LockFinalCopy<LabelNode>{other}) { }

  // Copy constructors with what argument.
  LabelNode(threadsafe::LockFinalCopy<LabelNode> other, std::string_view what);
  LabelNode(LabelNode const& other, std::string_view what) : LabelNode(threadsafe::LockFinalCopy<LabelNode>{other}, what) { }

  void set_label(std::string const& label)
  {
    label_ = label;
  }
};

} // namespace cppgraphviz
