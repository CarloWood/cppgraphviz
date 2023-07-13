#pragma once

#include "Item.h"
#include "dot/Node.h"
#include "threadsafe/ObjectTracker.h"
#include "utils/has_print_on.h"
#include <boost/intrusive_ptr.hpp>
#ifdef CWDEBUG
#include "debug_ostream_operators.h"
#endif

namespace cppgraphviz {
using utils::has_print_on::operator<<;

class GraphTracker;
class locked_Node;

// Define an unlocked tracked Node, protected by a std::mutex.
using Node = threadsafe::UnlockedTrackedObject<locked_Node, dot::ItemLockingPolicy>;

// A Node has a std::shared_ptr<NodeTracker> (tracker_).
// A NodeTracker has a Node* (node_) that points back to the Node.
//
// A Node also has a GraphPtr (graph_ptr_),
// and the Graph has a std::weak_ptr<NodeTracker> (element of node_trackers_)
// that points back to the NodeTracker that that Node points to.
//
//        --graph_ptr_ ------------------------------> Graph
//   Node --tracker_-> NodeTracker <--node_trackers_--
//        <---node_---
//
// The relationship between a Node and a NodeTracker is 1-on-1,
// and the Node manages the NodeTracker. Therefore we can also
// see the 'Node / NodeTracker' pair as a single "object".
// This is why a NodeTracker automatically converts to a Node,
// and we use the name 'node' for both (a variable of type
// std::shared_ptr<NodeTracker> is still called a node_tracker
// though).

// Define a corresponding tracker class.
class NodeTracker final : public threadsafe::ObjectTracker<Node, locked_Node, dot::ItemLockingPolicy>
{
 private:
  dot::NodePtr node_ptr_;       // Unique pointer to the corresponding dot::NodeItem.

 public:
  NodeTracker(utils::Badge<threadsafe::TrackedObject<Node, NodeTracker>>, Node& node);

  void set_what(std::string_view what)
  {
    dot::NodePtr::unlocked_type::wat node_item_w(node_ptr_.item());
    node_item_w->attribute_list().remove("what");
    node_item_w->attribute_list().add({"what", what});
  }

  dot::NodePtr const& node_ptr() const { return node_ptr_; }
  dot::NodePtr& node_ptr() { return node_ptr_; }
};

class locked_Node : public ItemTemplate<Node, NodeTracker>
{
 public:
  locked_Node(std::string_view what);
  locked_Node(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what);
  locked_Node(locked_Node&& node, std::string_view what);
  locked_Node(locked_Node const& other, std::string_view what);
  locked_Node(locked_Node const& other);

  locked_Node(locked_Node&& node) : ItemTemplate<Node, NodeTracker>(std::move(node))
  {
    DoutEntering(dc::notice, "default locked_Node(locked_Node&& " << &node << ") [" << this << "]");
  }

  ~locked_Node();

  operator dot::NodePtr&() const
  {
    return tracker_->node_ptr();
  }

  void initialize() override;

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace cppgraphviz
