#pragma once

#include "LabelNode.h"
#include <string>

namespace cppgraphviz {

struct RectangleNode : LabelNode
{
  using LabelNode::LabelNode;

  void item_attributes(dot::AttributeList& list) override
  {
    list += {"shape", "rectangle"};
    LabelNode::item_attributes(list);
  }
};

} // namespace cppgraphviz
