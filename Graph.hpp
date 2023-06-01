#pragma once

#include "MemoryRegionOwner.hpp"
#include "GraphTracker.hpp"
#include "NodeTracker.hpp"
#include "Item.hpp"
#include <vector>
#include <memory>
#ifdef CWDEBUG
#include "debug_ostream_operators.hpp"
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
  std::vector<std::weak_ptr<NodeTracker>> node_trackers_;       // The nodes that are added to this graph.
  std::vector<std::weak_ptr<GraphTracker>> graph_trackers_;     // The subgraphs that are added to this graph.

 public:
  // Create a new Graph/GraphTracker pair. This is a root graph.
  Graph(char const* what) : ItemTemplate<GraphTracker>({})
  {
    DoutEntering(dc::notice, "Graph(\"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
  }

  // Create a new Graph/GraphTracker pair. This is a subgraph.
  Graph(std::weak_ptr<GraphTracker> const& root_graph, char const* what) : ItemTemplate<GraphTracker>(root_graph, this)
  {
    DoutEntering(dc::notice, "Graph(root_graph, \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
    get_parent_graph().add_graph(tracker_);
  }

  // Move a Graph, updating its GraphTracker.
  Graph(Graph&& other, char const* what) :
    ItemTemplate<GraphTracker>(std::move(other)),
    node_trackers_(std::move(other.node_trackers_)),
    graph_trackers_(std::move(other.graph_trackers_))
  {
    DoutEntering(dc::notice, "Graph(Graph&& " << &other << ", \"" << what << "\") [" << this << "]");
    tracker_->set_what(what);
  }

  // Copying a Graph is not allowed.
  Graph(Graph const& other) = delete;

 private:
  template<typename T>
  friend class Class;
  Graph(Graph const& other, char const* what) :
    ItemTemplate<GraphTracker>(other.root_graph_tracker(), this),
    node_trackers_{},
    graph_trackers_{}
  {
    DoutEntering(dc::notice, "Graph(Graph const& " << &other << ", \"" << what << "\") [" << this << "]");
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
    for (auto& weak_node_tracker : node_trackers)
      remove_node(weak_node_tracker.lock());
    for (auto& weak_graph_tracker : graph_trackers)
      remove_graph(weak_graph_tracker.lock());
  }

  void add_node(std::weak_ptr<NodeTracker> node_tracker);
  void remove_node(std::shared_ptr<NodeTracker>&& node_tracker);
  void add_graph(std::weak_ptr<GraphTracker> graph_tracker);
  void remove_graph(std::shared_ptr<GraphTracker>&& graph_tracker);
  void write_dot(std::ostream& os) const;

  void initialize() override
  {
    // Add the attributes of this Node.
    item_attributes(tracker_->graph_ptr().attribute_list());
    call_initialize_on_items();
  }

 private:
  void call_initialize_on_items() const;

  // Implement virtual function of MemoryRegionOwner.
  void on_memory_region_usage(MemoryRegion const& used) override
  {
    Item* item = reinterpret_cast<Item*>(used.begin());
    std::weak_ptr<GraphTracker> subgraph_tracker = *this;
    item->set_parent_graph_tracker(std::move(subgraph_tracker));
  }

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override;
#endif
};

} // namespace cppgraphviz
