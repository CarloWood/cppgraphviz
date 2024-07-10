#pragma once

#include "MemoryRegionOwner.h"
#include "Item.h"
#include "dot/Graph.h"
#include "threadsafe/ObjectTracker.h"
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

class GraphTracker;
class NodeTracker;
class locked_Graph;
class Graph;

// GraphTracker objects can only be created by calling the static GraphTracker::create,
// which uses std::make_shared<GraphTracker> to create it.
//
// Like NodeTracker, GraphTracker contains a pointer to both, the corresponding Graph
// object in a 1-on-1 relationship with its GraphTracker, and a pointer to the
// dot::GraphItem that represents the graphical (sub)graph in dot.
//
// In turn Graph has a std::shared_ptr<GraphTracker> graph_tracker_; pointing to
// its GraphTracker object, and whenever the Graph is moved it adjusts the Graph*
// in its tracker by calling set_graph.
//
class GraphTracker final : public threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>
{
 private:
  dot::GraphPtr graph_ptr_;     // Unique pointer to the corresponding dot::GraphItem.

 public:
  GraphTracker(utils::Badge<threadsafe::TrackedObject<Graph, GraphTracker>>, Graph& graph);

  void set_what(std::string_view what)
  {
    dot::GraphPtr::unlocked_type::wat graph_ptr_w{graph_ptr_.item()};
    graph_ptr_w->attribute_list().remove("what");
    graph_ptr_w->attribute_list().add({"what", what});
  }

  // Accessors.
  dot::GraphPtr const& graph_ptr() const { return graph_ptr_; }
  dot::GraphPtr& graph_ptr() { return graph_ptr_; }
};

class locked_Graph : public ItemTemplate<Graph, GraphTracker>, public MemoryRegionOwner
{
 public:
  // Disambiguate the tracker() member function.
  using threadsafe::TrackedObject<Graph, GraphTracker>::tracker;

 protected:
  // Disambiguate the tracker_ member variable.
  using threadsafe::TrackedObject<Graph, GraphTracker>::tracker_;

 private:
  std::vector<std::weak_ptr<NodeTracker>> node_trackers_;               // The nodes that are added to this graph.
  std::vector<std::weak_ptr<GraphTracker>> graph_trackers_;             // The subgraphs that are added to this graph.
  std::vector<std::weak_ptr<MemoryRegionOwnerTracker>> array_trackers_; // Array objects that were added to this (root) graph.

 public:
  // Instantiating the destructor will cause the instantiation of the destructor of graph_trackers_
  // which leads to the instantiation of ~GraphTracker and therefore of ~locked_Graph (this class).
  // Because locked_Graph is still incomplete, we can not define constructors or the destructor in
  // the header.
  locked_Graph(std::string_view what);
  locked_Graph(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what);
  locked_Graph(locked_Graph&& orig, std::string_view what);
  locked_Graph(MemoryRegion memory_region, std::weak_ptr<GraphTracker> const& root_graph, std::string_view what);
  locked_Graph(locked_Graph&& orig, MemoryRegion const& memory_region, std::string_view what);

  // Copying a Graph is not allowed.
  locked_Graph(locked_Graph const& other) = delete;

  // Destructor.
  ~locked_Graph();

 private:
  template<typename T, typename POLICY_MUTEX>
  friend class ::threadsafe::Unlocked;
  locked_Graph(MemoryRegion memory_region, locked_Graph const& other, std::string_view what);

 public:
  void add_node(std::weak_ptr<NodeTracker> node_tracker);
  void remove_node(std::shared_ptr<NodeTracker>&& node_tracker);
  void add_graph(std::weak_ptr<GraphTracker> graph_tracker);
  void remove_graph(std::shared_ptr<GraphTracker>&& graph_tracker);
  void add_array(std::weak_ptr<MemoryRegionOwnerTracker> weak_array_tracker);
  void remove_array(std::shared_ptr<MemoryRegionOwnerTracker>&& array_tracker);
  void write_dot(std::ostream& os) const;

  void initialize_item() override;

 private:
  void call_initialize_on_items() const;

  void on_memory_region_usage(MemoryRegion const& UNUSED_ARG(owner_memory_region),
      MemoryRegion const& UNUSED_ARG(item_memory_region), dot::NodePtr* UNUSED_ARG(node_ptr_ptr)) override
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

// Define an unlocked tracked Graph, protected by a std::mutex.
class Graph : public threadsafe::UnlockedTrackedObject<locked_Graph, dot::ItemLockingPolicy>
{
 public:
  using threadsafe::UnlockedTrackedObject<locked_Graph, dot::ItemLockingPolicy>::UnlockedTrackedObject;

  void write_dot(std::ostream& os) const
  {
    crat graph_r(*this);
    graph_r->write_dot(os);
  }
};

} // namespace cppgraphviz
