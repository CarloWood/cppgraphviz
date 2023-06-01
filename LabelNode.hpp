#pragma once

#include "Node.hpp"

namespace cppgraphviz {

class LabelNode : public Node
{
 public:
  using Node::Node;

  void item_attributes(dot::AttributeList& list) override
  {
    // Derive from LabelNode and override item_attributes to add a label, shape etc.
    list.add({"label", "<unknown LabelNode>"});
  }
};

} // namespace cppgraphviz
