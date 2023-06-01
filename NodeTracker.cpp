#include "sys.h"
#include "NodeTracker.hpp"
#include "Node.hpp"

namespace cppgraphviz {

Node const& NodeTracker::get_node() const
{
  return *tracked_object_ptr_;
}

Node& NodeTracker::get_node()
{
  return *tracked_object_ptr_;
}

} // namespace cppgraphviz
