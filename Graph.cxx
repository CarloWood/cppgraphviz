#include "sys.h"
#include "Graph.h"
#include "Node.h"
#include "Array.h"
#include "threadsafe/ObjectTracker.inl.h"

namespace cppgraphviz {

GraphTracker::GraphTracker(utils::Badge<threadsafe::TrackedObject<Graph, GraphTracker>>, Graph& graph) :
  threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>(graph)
{
}

// Create a new Graph/GraphTracker pair. This is a root graph.
locked_Graph::locked_Graph(std::string_view what) : ItemTemplate<Graph, GraphTracker>({})
{
  DoutEntering(dc::notice, "locked_Graph(\"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
}

// Create a new Graph/GraphTracker pair. This is a subgraph without an associated memory region.
locked_Graph::locked_Graph(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
  ItemTemplate<Graph, GraphTracker>(root_graph, this)
{
  DoutEntering(dc::notice, "locked_Graph(" << root_graph << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  parent_graph_wat()->add_graph(tracker_);
}

// Create a new Graph/GraphTracker pair. This is a subgraph.
locked_Graph::locked_Graph(MemoryRegion memory_region, std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
  ItemTemplate<Graph, GraphTracker>(root_graph, this), MemoryRegionOwner(memory_region)
{
  DoutEntering(dc::notice, "locked_Graph(" << memory_region << ", " << root_graph << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  parent_graph_wat()->add_graph(tracker_);
}

// Move a Graph, updating its GraphTracker.
locked_Graph::locked_Graph(locked_Graph&& orig, std::string_view what) :
  ItemTemplate<Graph, GraphTracker>(std::move(orig)),
  node_trackers_(std::move(orig.node_trackers_)),
  graph_trackers_(std::move(orig.graph_trackers_)),
  array_trackers_(std::move(orig.array_trackers_))
{
  DoutEntering(dc::notice, "locked_Graph(locked_Graph&& " << &orig << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
}

// Move a Graph, updating its GraphTracker.
locked_Graph::locked_Graph(locked_Graph&& orig, MemoryRegion const& memory_region, std::string_view what) :
  ItemTemplate<Graph, GraphTracker>(std::move(orig)),
  MemoryRegionOwner(std::move(orig), memory_region),
  node_trackers_(std::move(orig.node_trackers_)),
  graph_trackers_(std::move(orig.graph_trackers_)),
  array_trackers_(std::move(orig.array_trackers_))
{
  DoutEntering(dc::notice, "locked_Graph(locked_Graph&& " << &orig << ", " << memory_region << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
}

locked_Graph::locked_Graph(MemoryRegion memory_region, locked_Graph const& other, std::string_view what) :
  ItemTemplate<Graph, GraphTracker>(other.root_graph_tracker(), this),
  MemoryRegionOwner(memory_region),
  node_trackers_{},
  graph_trackers_{},
  array_trackers_{}
{
  DoutEntering(dc::notice, "locked_Graph(" << memory_region << ", locked_Graph const& " << &other << ", \"" << what << "\") [" << this << "]");
  tracker_->set_what(what);
  parent_graph_wat()->add_graph(tracker_);
}

locked_Graph::~locked_Graph()
{
  DoutEntering(dc::notice, "~locked_Graph() [" << this << "]");
  // The root graph and moved graphs don't have a parent.
  std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
  if (parent_graph_tracker)
    parent_graph_tracker->tracked_wat()->remove_graph(std::move(tracker_));

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

void locked_Graph::add_node(std::weak_ptr<NodeTracker> weak_node_tracker)
{
  std::shared_ptr<NodeTracker> node_tracker = weak_node_tracker.lock();
  if (node_tracker)
  {
    dot::GraphPtr::unlocked_type::wat{tracker_->graph_ptr().item()}->add(node_tracker->node_ptr());
    node_trackers_.push_back(std::move(weak_node_tracker));
    node_tracker->tracked_wat()->set_parent_graph_tracker(tracker_);
  }
}

void locked_Graph::remove_node(std::shared_ptr<NodeTracker>&& node_tracker)
{
  node_tracker->tracked_wat()->set_parent_graph_tracker({});
  // Erase node_tracker and any expired elements from node_trackers_.
  std::erase_if(node_trackers_,
      [&node_tracker](std::weak_ptr<NodeTracker> const& wp){
        auto sp = wp.lock();
        return !sp || sp == node_tracker;
      });
  dot::GraphPtr::unlocked_type::wat{tracker_->graph_ptr().item()}->remove(node_tracker->node_ptr());
}

void locked_Graph::add_graph(std::weak_ptr<GraphTracker> weak_graph_tracker)
{
  std::shared_ptr<GraphTracker> graph_tracker = weak_graph_tracker.lock();
  if (graph_tracker)
  {
    dot::GraphPtr::unlocked_type::wat{tracker_->graph_ptr().item()}->add(graph_tracker->graph_ptr());
    graph_trackers_.push_back(std::move(weak_graph_tracker));
    graph_tracker->tracked_wat()->set_parent_graph_tracker(tracker_);
  }
}

void locked_Graph::remove_graph(std::shared_ptr<GraphTracker>&& graph_tracker)
{
  graph_tracker->tracked_wat()->set_parent_graph_tracker({});
  // Erase graph_tracker and any expired elements from graph_trackers_.
  std::erase_if(graph_trackers_,
      [&graph_tracker](std::weak_ptr<GraphTracker> const& wp){
        auto sp = wp.lock();
        return !sp || sp == graph_tracker;
      });
  dot::GraphPtr::unlocked_type::wat{tracker_->graph_ptr().item()}->remove(graph_tracker->graph_ptr());
}

void locked_Graph::add_array(std::weak_ptr<MemoryRegionOwnerTracker> weak_array_tracker)
{
  std::shared_ptr<MemoryRegionOwnerTracker> array_tracker = weak_array_tracker.lock();
  if (array_tracker)
  {
    array_trackers_.push_back(std::move(weak_array_tracker));
  }
}

void locked_Graph::remove_array(std::shared_ptr<MemoryRegionOwnerTracker>&& array_tracker)
{
  // Erase array_tracker and any expired elements from array_trackers_.
  std::erase_if(array_trackers_,
      [&array_tracker](std::weak_ptr<MemoryRegionOwnerTracker> const& wp){
        auto sp = wp.lock();
        return !sp || sp == array_tracker;
      });
}

void locked_Graph::initialize()
{
  // Add the attributes of this Node.
  item_attributes(dot::GraphPtr::unlocked_type::wat{tracker_->graph_ptr().item()}->attribute_list());
  call_initialize_on_items();
}

void locked_Graph::call_initialize_on_items() const
{
  for (std::weak_ptr<NodeTracker> const& weak_node_tracker : node_trackers_)
  {
    std::shared_ptr<NodeTracker> node_tracker = weak_node_tracker.lock();
    if (node_tracker)
    {
      auto item_w = node_tracker->tracked_wat();
      item_w->initialize();
    }
  }
  for (std::weak_ptr<GraphTracker> const& weak_graph_tracker : graph_trackers_)
  {
    std::shared_ptr<GraphTracker> graph_tracker = weak_graph_tracker.lock();
    if (graph_tracker)
    {
      auto item_w = graph_tracker->tracked_wat();
      item_w->initialize();
    }
  }
  for (std::weak_ptr<MemoryRegionOwnerTracker> const& weak_array_tracker : array_trackers_)
  {
    std::shared_ptr<MemoryRegionOwnerTracker> array_tracker = weak_array_tracker.lock();
    if (array_tracker)
    {
      MemoryRegionOwner& memory_region_owner = array_tracker->tracked_object();
      ArrayMemoryRegionOwner& array_memory_region_owner = static_cast<ArrayMemoryRegionOwner&>(memory_region_owner);
      array_memory_region_owner.call_initialize_on_elements();
    }
  }
}

void locked_Graph::write_dot(std::ostream& os) const
{
  call_initialize_on_items();
  dot::GraphPtr::unlocked_type::rat{tracker_->graph_ptr().item()}->write_dot(os);
}

#ifdef CWDEBUG
void locked_Graph::print_on(std::ostream& os) const
{
#if CW_DEBUG
  std::weak_ptr<GraphTracker> weak_graph_tracker = ItemTemplate<Graph, GraphTracker>::operator std::weak_ptr<GraphTracker>();
  std::shared_ptr<GraphTracker> graph_tracker = weak_graph_tracker.lock();
  GraphTracker const* graph_tracker2 = graph_tracker ? graph_tracker.get() : nullptr;
  // Do not attempt to print an object that was moved; if this object was moved then ItemTemplate<GraphTracker>::tracker_ points to null.
  ASSERT(graph_tracker2);
#endif

  os << '"' << dot::GraphPtr::unlocked_type::crat{tracker().graph_ptr().item()}->attribute_list().get("what", "<NO \"what\">") << '"';
}
#endif

} // namespace cppgraphviz
