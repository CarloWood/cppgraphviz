#include "sys.h"
#include "Graph.hpp"
#include <iostream>
#include "debug.h"

namespace cppgraphviz::dot {

void GraphItem::add_graph_item(Item const* item)
{
  auto ibp = items_.try_emplace(item->dot_id(), item);
  // Do not add the same graph item twice.
  ASSERT(ibp.second);
  if (item->is_graph())
  {
    auto& subgraph = static_cast<ConstItemPtrTemplate<GraphItem>&>(ibp.first->second);
    // The subgraph must be of the same type as this graph.
    subgraph.item().set_digraph(digraph_);
    subgraph.item().set_rankdir(rankdir_);
  }
}

void GraphItem::remove_graph_item(Item const* item)
{
  bool erased = items_.erase(item->dot_id());
  // That's unexpected... we shouldn't be calling remove_graph_item unless it is there.
  ASSERT(erased);
}

void GraphItem::add_node_attribute(Attribute&& attribute)
{
  node_attribute_list_.add(std::move(attribute));
}

void GraphItem::add_edge_attribute(Attribute&& attribute)
{
  edge_attribute_list_.add(std::move(attribute));
}

void GraphItem::set_digraph(bool digraph) const
{
  if (digraph_ != digraph)
  {
    digraph_ = digraph;
    // Recursively change all subgraphs.
    for (auto& graph_pair : items_)
    {
      ConstItemPtr const& item_ptr = graph_pair.second;
      if (!item_ptr.item().is_graph())
        continue;
      ConstItemPtrTemplate<GraphItem> const& graph = static_cast<ConstItemPtrTemplate<GraphItem> const&>(item_ptr);
      graph->set_digraph(digraph);
    }
  }
}

void GraphItem::set_rankdir(RankDir rankdir) const
{
  if (rankdir_ != rankdir)
  {
    rankdir_ = rankdir;
    // Recursively change all subgraphs.
    for (auto& graph_pair : items_)
    {
      ConstItemPtr const& item_ptr = graph_pair.second;
      if (!item_ptr.item().is_graph())
        continue;
      ConstItemPtrTemplate<GraphItem> const& graph = static_cast<ConstItemPtrTemplate<GraphItem> const&>(item_ptr);
      graph->set_rankdir(rankdir);
    }
  }
}

void GraphItem::write_dot(std::ostream& os) const
{
  // [ strict ] (graph | digraph) [ ID ] '{' stmt_list '}'
  if (strict_)
    os << "strict ";
  if (digraph_)
    os << "di";
  os << "graph " << dot_id() << " {\n";
  if (rankdir_ != TB)
  {
    os << "  rankdir=";
    if (rankdir_ == LR)
      os << "LR";
    else if (rankdir_ == BT)
      os << "BT";
    else
      os << "RL";
    os << '\n';
  }
  os << "  compound=true\n";
  if (concentrate_)
    os << "  concentrate=true\n";

  write_body_to(os);

  // Close the [di]graph.
  os << '}' << std::endl;
}

void GraphItem::write_body_to(std::ostream& os, std::string indentation) const
{
  // stmt_list	:	[ stmt [ ';' ] stmt_list ]
  // stmt	:	node_stmt
  //            |       edge_stmt
  //            |       attr_stmt
  //            |       ID '=' ID
  //            |       subgraph

  indentation += "  ";

  // Write default attributes.
  if (attribute_list())
    os << indentation << "graph [" << attribute_list() << "]\n";
  if (node_attribute_list_)
    os << indentation << "node [" << node_attribute_list_ << "]\n";
  if (edge_attribute_list_)
    os << indentation << "edge [" << edge_attribute_list_ << "]\n";

  std::map<item_type_type, std::string> output;

  // Write all items to the output map, sorting them by item_type.
  for (auto const& item_pair : items_)
  {
    std::ostringstream oss;
    if (digraph_)
      oss << digraph;
    Item const& item = item_pair.second.item();
    item.write_dot_to(oss, indentation);
    auto ibp = output.try_emplace(item.item_type());
    ibp.first->second.append(oss.str());
  }

  // Write output to os.
  for (auto item_type_string_pair : output)
    os << item_type_string_pair.second;
}

void GraphItem::write_dot_to(std::ostream& os, std::string& indentation) const
{
  os << indentation << "subgraph " << dot_id() << " {\n";
  write_body_to(os, indentation);
  os << indentation << "}\n";
}

utils::iomanip::Index DigraphIomanip::s_index;
DigraphIomanip digraph;

} // namespace cppgraphviz::dot
