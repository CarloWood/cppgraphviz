#include "sys.h"
#include "Node.h"

namespace cppgraphviz {

#ifdef CWDEBUG
void Node::print_on(std::ostream& os) const
{
  os << '"' << tracker_->node_ptr()->attribute_list().get("what", "<NO \"what\">") << '"';
}
#endif

} // namespace cppgraphviz
