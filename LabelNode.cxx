#include "sys.h"
#include "LabelNode.h"

namespace cppgraphviz {

LabelNode::LabelNode(threadsafe::LockFinalMove<LabelNode> other, std::string_view what) :
  Node(std::move(other)), label_(std::move(other->label_))
{
  tracker_->set_what(what);
}

LabelNode::LabelNode(threadsafe::LockFinalCopy<LabelNode> other, std::string_view what) :
  Node(other), label_(other->label_)
{
  tracker_->set_what(what);
}

} // namespace cppgraphviz
