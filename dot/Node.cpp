#include "sys.h"
#include "Node.hpp"

namespace cppgraphviz::dot {

void NodeItem::write_dot_to(std::ostream& os, std::string& indentation) const
{
  // node_stmt	:	node_id [ attr_list ]
  os << indentation << dot_id() << " [" << attribute_list() << "]\n";
}

} // namespace cppgraphviz::dot
