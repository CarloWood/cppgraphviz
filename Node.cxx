#include "sys.h"
#include "Node.h"

namespace cppgraphviz {

#ifdef CWDEBUG
void Node::print_on(std::ostream& os) const
{
  os << '"' << dot::NodePtr::unlocked_type::crat{tracker_->node_ptr().item()}->attribute_list().get("what", "<NO \"what\">") << '"';
}
#endif

} // namespace cppgraphviz
