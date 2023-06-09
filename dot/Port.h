#pragma once

#include "ItemPtr.h"
#include "DotID.h"
#include "Node.h"
#include <optional>
#include <iosfwd>

namespace cppgraphviz::dot {

class Port
{
 private:
  ID_type id_;
  std::optional<size_t> port_;

 public:
  Port() = default;
  Port(NodePtr::unlocked_type::crat const& node_item_r);
  Port(NodePtr const& node_ptr) : Port(NodePtr::unlocked_type::crat{node_ptr.item()}) { }
  Port(ID_type id, size_t port) : id_(id), port_(port) { }

  void set_port(ID_type id) { id_ = id; port_.reset(); }
  void write_to(std::ostream& os) const;

  ID_type id() const { return id_; }
  bool has_port() const { return port_.has_value(); }
  size_t port() const { return port_.value(); }
};

inline std::ostream& operator<<(std::ostream& os, Port const& port)
{
  port.write_to(os);
  return os;
}

} // namespace cppgraphviz::dot
