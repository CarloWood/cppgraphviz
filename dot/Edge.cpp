#include "sys.h"
#include "Edge.hpp"
#include "Node.hpp"
#include "Graph.hpp"

namespace cppgraphviz::dot {

void EdgeItem::set_nodes(Port const& from, Port const& to)
{
  from_ = from;
  to_ = to;
}

void EdgeItem::write_dot_to(std::ostream& os, std::string& indentation) const
{
  bool digraph = DigraphIomanip::get_iword_value(os);

  // edge_stmt	:	(node_id | subgraph) edgeRHS [ attr_list ]
  // edgeRHS	:	edgeop (node_id | subgraph) [ edgeRHS ]
  os << indentation << from_port() << (digraph ? " -> " : " -- ") << to_port() << " [" << attribute_list() << "]\n";
}

} // namespace cppgraphviz::dot
