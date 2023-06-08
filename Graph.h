#pragma once

#include "MemoryRegionOwner.h"
#include "GraphTracker.h"
#include "NodeTracker.h"
#include "Item.h"
#include <vector>
#include <memory>
#ifdef CWDEBUG
#include "debug_ostream_operators.h"
#include "utils/has_print_on.h"
#endif

namespace cppgraphviz {
#ifdef CWDEBUG
using utils::has_print_on::operator<<;
#endif

template<typename T>
class Class;

class Graph : public ItemTemplate<GraphTracker>, public MemoryRegionOwner
{
 public:
  // Disambiguate the tracker() member function.
  using ItemTemplate<GraphTracker>::tracker;

 private:
  // Disambiguate the tracker_ member variable.
  using ItemTemplate<GraphTracker>::tracker_;
  std::vector<std::weak_ptr<NodeTracker>> node_trackers_;               // The nodes that are added to this graph.
  std::vector<std::weak_ptr<GraphTracker>> graph_trackers_;             // The subgraphs that are added to this graph.
  std::vector<std::weak_ptr<MemoryRegionOwnerTracker>> array_trackers_; // Array objects that were added to this (root) graph.

 public:
  // Create a new Graph/GraphTracker pair. This is a root graph.
  Graph(std::string_view what) : ItemTemplate<GraphTracker>({})
  {
    DoutEntering(dc::notice, "Graph(\"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
  }

  // Create a new Graph/GraphTracker pair. This is a subgraph without an associated memory region.
  Graph(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
    ItemTemplate<GraphTracker>(root_graph, this)
  {
    DoutEntering(dc::notice, "Graph(" << root_graph << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    get_parent_graph().add_graph(tracker_);
  }

  // Create a new Graph/GraphTracker pair. This is a subgraph.
  Graph(MemoryRegion memory_region, std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
    ItemTemplate<GraphTracker>(root_graph, this), MemoryRegionOwner(memory_region)
  {
    DoutEntering(dc::notice, "Graph(" << memory_region << ", " << root_graph << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    get_parent_graph().add_graph(tracker_);
  }

  // Move a Graph, updating its GraphTracker.
  // The new instance is not associated with a new MemoryRegion because ... FIXME
  Graph(Graph&& orig, std::string_view what) :
    // Call unregister_memory_region for the memory region stored in the second base class, MemoryRegionOwner,
    // before moving the first base class, ItemTemplate<GraphTracker>. This is necessary because debug code in
    // unregister_memory_region wants to print the 'what' attribute of the memory region owner associated with
    // the memory region being removed. Printing this accessing the tracker_ in ItemTemplate<GraphTracker>,
    // which therefore may not yet be moved.
    ItemTemplate<GraphTracker>((Item::current_graph_linker_.unregister_memory_region(orig.registered_memory_region_), std::move(orig))),
    MemoryRegionOwner(std::move(orig), {}),
    node_trackers_(std::move(orig.node_trackers_)),
    graph_trackers_(std::move(orig.graph_trackers_)),
    array_trackers_(std::move(orig.array_trackers_))
  {
    DoutEntering(dc::notice, "Graph(Graph&& " << &orig << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
  }

  // Copying a Graph is not allowed.
  Graph(Graph const& other) = delete;

 private:
  template<typename T>
  friend class Class;
  Graph(MemoryRegion memory_region, Graph const& other, std::string_view what) :
    ItemTemplate<GraphTracker>(other.root_graph_tracker(), this),
    MemoryRegionOwner(memory_region),
    node_trackers_{},
    graph_trackers_{},
    array_trackers_{}
  {
    DoutEntering(dc::notice, "Graph(" << memory_region << ", Graph const& " << &other << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    get_parent_graph().add_graph(tracker_);
  }

 public:
  ~Graph()
  {
    DoutEntering(dc::notice, "~Graph() [" << this << "]");
    // The root graph and moved graphs don't have a parent.
    std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
    if (parent_graph_tracker)
      parent_graph_tracker->tracked_object().remove_graph(std::move(tracker_));

    // Make a copy of the child items and then remove them from this graph for proper bookkeeping.
    auto node_trackers = std::move(node_trackers_);
    auto graph_trackers = std::move(graph_trackers_);
    auto array_trackers = std::move(array_trackers_);
    for (auto& weak_node_tracker : node_trackers)
      remove_node(weak_node_tracker.lock());
    for (auto& weak_graph_tracker : graph_trackers)
      remove_graph(weak_graph_tracker.lock());
    for (auto& weak_array_tracker : array_trackers)
      remove_array(weak_array_tracker.lock());
  }

  void add_node(std::weak_ptr<NodeTracker> node_tracker);
  void remove_node(std::shared_ptr<NodeTracker>&& node_tracker);
  void add_graph(std::weak_ptr<GraphTracker> graph_tracker);
  void remove_graph(std::shared_ptr<GraphTracker>&& graph_tracker);
  void add_array(std::weak_ptr<MemoryRegionOwnerTracker> weak_array_tracker);
  void remove_array(std::shared_ptr<MemoryRegionOwnerTracker>&& array_tracker);
  void write_dot(std::ostream& os) const;

  void initialize() override
  {
    // Add the attributes of this Node.
    item_attributes(tracker_->graph_ptr().attribute_list());
    call_initialize_on_items();
  }

 private:
  void call_initialize_on_items() const;

  void on_memory_region_usage(MemoryRegion const& UNUSED_ARG(item_memory_region), dot::NodePtr* UNUSED_ARG(node_ptr_ptr)) override
  {
    // It should only be possible that this function is called if a class derived from Graph
    // registered a memory region (for example, Class). That derived class must override this
    // virtual function.
    ASSERT(false);
  }

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override;
#endif
};

} // namespace cppgraphviz
