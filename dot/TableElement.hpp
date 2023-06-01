#pragma once

#include "DotID.hpp"
#include "Node.hpp"
#include <vector>
#include <string>
#include <functional>

namespace cppgraphviz::dot {

class TableElement
{
 private:
  NodePtr node_ptr_;

 public:
  TableElement(NodePtr& node_ptr) : node_ptr_(node_ptr) { }
  TableElement(NodePtr&& node_ptr) : node_ptr_(std::move(node_ptr)) { }

  std::string_view label() const
  {
    return node_ptr_.item().attribute_list().get("label", "<no label>");
  }

  AttributeList const& attribute_list() const
  {
    return node_ptr_.item().attribute_list();
  }

  NodePtr const& node_ptr() const
  {
    return node_ptr_;
  }

  NodePtr& node_ptr()
  {
    return node_ptr_;
  }
};

} // namespace cppgraphviz::dot
