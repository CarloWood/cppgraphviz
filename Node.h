#pragma once

#include "NodeTracker.h"
#include "Item.h"
#include "Graph.h"
#include "utils/has_print_on.h"
#include <boost/intrusive_ptr.hpp>
#ifdef CWDEBUG
#include "debug_ostream_operators.h"
#endif

namespace cppgraphviz {
using utils::has_print_on::operator<<;

template<typename T>
class Class;

class Node : public ItemTemplate<NodeTracker>
{
 public:
  // Create a new Node/NodeTracker pair for a class member.
  // That means that Item should always find a parent graph tracker for this; if we don't then
  // this is probably a temporary that will be moved or copied shortly after to the class member.
  Node(std::string_view what) : ItemTemplate(this)
  {
    DoutEntering(dc::notice, "Node(root_graph, \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    auto pgt = parent_graph_tracker();
    // A temporary can't be added yet.
    if (pgt)
      pgt->tracked_object().add_node(tracker_);
  }

  // Create a new Node/NodeTracker pair.
  Node(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) : ItemTemplate(root_graph, this)
  {
    DoutEntering(dc::notice, "Node(root_graph, \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    get_parent_graph().add_node(tracker_);
  }

  // Move a Node, updating its NodeTracker.
  Node(Node&& node, std::string_view what) : ItemTemplate<NodeTracker>(std::move(node))
  {
    DoutEntering(dc::notice, "Node(Node&& " << &node << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
  }

  Node(Node&& node) : ItemTemplate<NodeTracker>(std::move(node))
  {
    DoutEntering(dc::notice, "default Node(Node&& " << &node << ") [" << this << "]");
  }

  // Copy a Node, creating a new NodeTracker as well.
  Node(Node const& other, std::string_view what) :
    ItemTemplate(other.root_graph_tracker(), this)
  {
    DoutEntering(dc::notice, "Node(Node const& " << &other << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    std::shared_ptr<GraphTracker> graph_tracker = parent_graph_tracker();
    // Node's that are added to a TableNode are not added to a Graph.
    if (graph_tracker)
      graph_tracker->tracked_object().add_node(tracker_);
  }

  Node(Node const& other) : ItemTemplate(other.root_graph_tracker(), this)
  {
    DoutEntering(dc::notice, "default Node(Node const& " << &other << ") [" << this << "]");
    {
      dot::NodePtr::unlocked_type::crat node_item_r{other.tracker_->node_ptr().item()};
      std::string_view what = node_item_r->attribute_list().get_value("what");
      tracker_->set_what(what);
    }
    get_parent_graph().add_node(tracker_);
  }

  ~Node()
  {
    DoutEntering(dc::notice, "~Node() [" << this << "]");

    // If Node was moved then tracker_ (and parent_graph_tracker_) will be null.
    std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
    if (parent_graph_tracker && tracker_)
      parent_graph_tracker->tracked_object().remove_node(std::move(tracker_));
  }

  operator std::weak_ptr<NodeTracker>() const { return tracker_; }
  operator dot::NodePtr&() const { return tracker_->node_ptr(); }

  void initialize() override
  {
    // Add the attributes of this Node.
    item_attributes(dot::NodePtr::unlocked_type::wat{tracker_->node_ptr().item()}->attribute_list());
  }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

} // namespace cppgraphviz
