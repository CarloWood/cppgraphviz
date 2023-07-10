#include "sys.h"
#include "LabelNode.h"

namespace cppgraphviz {

LabelNode::LabelNode(LabelNode&& label_node, std::string_view what) :
  Node(std::move(label_node), this->noLock), label_(std::move(label_node.label_))
{
  tracker_->set_what(what);
}

LabelNode::LabelNode(LabelNode const& other, std::string_view what) :
  Node(other.do_rdlock(), this->noLock), label_(other.label_)
{
  tracker_->set_what(what);
  other.do_rdunlock();
}

} // namespace cppgraphviz
