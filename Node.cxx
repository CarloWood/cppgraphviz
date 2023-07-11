#include "sys.h"
#include "Node.h"
#include "Graph.h"
#include "threadsafe/ObjectTracker.inl.h"

namespace cppgraphviz {

NodeTracker::NodeTracker(utils::Badge<threadsafe::TrackedObject<Node, NodeTracker>>, Node& node) :
  threadsafe::ObjectTracker<Node, locked_Node, dot::ItemLockingPolicy>(node)
{
}

locked_Node::locked_Node(std::string_view what) : ItemTemplate(this)
{
  DoutEntering(dc::notice, "locked_Node(root_graph, \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  auto pgt = parent_graph_tracker();
  // A temporary can't be added yet.
  if (pgt)
    pgt->tracked_wat()->add_node(tracker_);
}

// Create a new Node/NodeTracker pair.
locked_Node::locked_Node(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) : ItemTemplate(root_graph, this)
{
  DoutEntering(dc::notice, "locked_Node(root_graph, \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  parent_graph_wat()->add_node(tracker_);
}

// Move a Node, updating its NodeTracker.
locked_Node::locked_Node(locked_Node&& node, std::string_view what) : ItemTemplate<Node, NodeTracker>(std::move(node))
{
  DoutEntering(dc::notice, "locked_Node(locked_Node&& " << &node << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
}

// Copy a Node, creating a new NodeTracker as well.
locked_Node::locked_Node(locked_Node const& other, std::string_view what) :
  ItemTemplate(other.root_graph_tracker(), this)
{
  DoutEntering(dc::notice, "locked_Node(locked_Node const& " << &other << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  std::shared_ptr<GraphTracker> graph_tracker = parent_graph_tracker();
  // locked_Node's that are added to a TableNode are not added to a Graph.
  if (graph_tracker)
    graph_tracker->tracked_wat()->add_node(tracker_);
}

locked_Node::locked_Node(locked_Node const& other) : ItemTemplate(other.root_graph_tracker(), this)
{
  DoutEntering(dc::notice, "default locked_Node(locked_Node const& " << &other << ") [" << this << "]");
  {
    dot::NodePtr::unlocked_type::crat node_item_r{other.tracker_->node_ptr().item()};
    std::string_view what = node_item_r->attribute_list().get_value("what");
    tracker_->set_what(what);
  }
  parent_graph_wat()->add_node(tracker_);
}

locked_Node::~locked_Node()
{
  DoutEntering(dc::notice, "~locked_Node() [" << this << "]");

  // If locked_Node was moved then tracker_ (and parent_graph_tracker_) will be null.
  std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
  if (parent_graph_tracker && tracker_)
    parent_graph_tracker->tracked_wat()->remove_node(std::move(tracker_));
}

void locked_Node::initialize()
{
  // Add the attributes of this locked_Node.
  item_attributes(dot::NodePtr::unlocked_type::wat{tracker_->node_ptr().item()}->attribute_list());
}

#ifdef CWDEBUG
void locked_Node::print_on(std::ostream& os) const
{
  os << '"' << dot::NodePtr::unlocked_type::crat{tracker_->node_ptr().item()}->attribute_list().get("what", "<NO \"what\">") << '"';
}
#endif

} // namespace cppgraphviz
