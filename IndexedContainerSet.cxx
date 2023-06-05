#include "sys.h"
#include "IndexedContainerSet.h"

namespace cppgraphviz {

void IndexedContainerSet::add_to_graph(dot::GraphItem& graph_item)
{
  graph_item.add(outer_subgraph_);
}

void IndexedContainerSet::remove_from_graph(dot::GraphItem& graph_item)
{
  graph_item.remove(outer_subgraph_);
}

namespace detail {

void RankdirGraphData::set_rankdir(dot::RankDir rankdir) const
{
  owner_->rankdir_changed(rankdir);
  dot::GraphItem::set_rankdir(rankdir);
}

} // namespace detail

} // namespace cppgraphviz
