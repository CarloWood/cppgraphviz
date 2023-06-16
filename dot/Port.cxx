#include "sys.h"
#include "Port.h"
#include "Node.h"
#include <iostream>

namespace cppgraphviz::dot {

Port::Port(NodePtr::unlocked_type::crat const& node_item_r) : id_(node_item_r->dot_id())
{
}

void Port::write_to(std::ostream& os) const
{
  os << id_;
  if (port_.has_value())
    os << ':' << port_.value();
}

} // namespace cppgraphviz::dot
