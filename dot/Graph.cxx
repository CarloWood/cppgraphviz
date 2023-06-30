#include "sys.h"
#include "Graph.h"
#include <iostream>
#include "debug.h"

namespace cppgraphviz::dot {

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
      ConstItemPtr::unlocked_type::crat item_ptr_r{item_ptr.item()};
      if (!item_ptr_r->is_graph())
        continue;
      GraphItem const& graph = static_cast<GraphItem const&>(*item_ptr_r);
      graph.set_digraph(digraph);
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
      ConstItemPtr::unlocked_type::crat item_ptr_r{item_ptr.item()};
      if (!item_ptr_r->is_graph())
        continue;
      GraphItem const& graph = static_cast<GraphItem const&>(*item_ptr_r);
      graph.set_rankdir(rankdir);
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
    Item::unlocked_type::crat item_r(item_pair.second.item());
    item_r->write_dot_to(oss, indentation);
    auto ibp = output.try_emplace(item_r->item_type());
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
