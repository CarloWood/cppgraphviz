#include "sys.h"
#include "Item.h"
#include "Graph.h"
#include "debug.h"

namespace cppgraphviz {

void Item::set_parent_graph_tracker(std::weak_ptr<GraphTracker> parent_graph_tracker)
{
  parent_graph_tracker_ = std::move(parent_graph_tracker);
}

void Item::extract_root_graph()
{
  std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
  if (parent_graph_tracker)
  {
    // The parent_graph_tracker_ points to the tracker of the subgraph associated with the class containing this Node.
    auto subgraph_r = parent_graph_tracker->tracked_rat();
    // Get the root graph from that subgraph.
    root_graph_tracker_ = subgraph_r->root_graph_tracker();
  }
}

threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>::wat Item::parent_graph_wat()
{
  std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
  // Don't call parent_graph_wat if this Item doesn't have one.
  ASSERT(parent_graph_tracker);
  return parent_graph_tracker->tracked_wat();
}

threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>::rat Item::parent_graph_rat()
{
  std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
  // Don't call parent_graph_rat if this Item doesn't have one.
  ASSERT(parent_graph_tracker);
  return parent_graph_tracker->tracked_rat();
}

} // namespace cppgraphviz
