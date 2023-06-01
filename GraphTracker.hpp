#pragma once

#include "ItemTracker.hpp"
#include "dot/Graph.hpp"
#include <memory>

namespace cppgraphviz {

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
class GraphTracker : public ItemTracker<Graph>
{
 private:
  dot::GraphPtr graph_ptr_;     // Unique pointer to the corresponding dot::GraphItem.

 public:
  // Private constructor, called by create.
  GraphTracker(Graph* graph) : ItemTracker<Graph>(graph) { }

  void set_what(std::string_view what)
  {
    graph_ptr_->attribute_list().remove("what");
    graph_ptr_->attribute_list().add({"what", what});
  }

  // Accessors.
  dot::GraphPtr const& graph_ptr() const { return graph_ptr_; }
  dot::GraphPtr& graph_ptr() { return graph_ptr_; }
};

} // namespace cppgraphviz
