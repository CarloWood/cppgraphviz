#pragma once

#include "Item.hpp"
#include "Port.hpp"
#include <concepts>
#include <type_traits>

namespace cppgraphviz::dot {

class EdgeItem : public Item
{
 private:
  // The nodes of this edge.
  Port from_;
  Port to_;

 public:
  void set_nodes(Port const& from, Port const& to);

  Port const& from_port() const { return from_; }
  Port const& to_port() const { return to_; }

  item_type_type item_type() const override { return item_type_edge; }
  void write_dot_to(std::ostream& os, std::string& indentation) const override;
};

template<typename T>
concept ConceptIsEdgeItem = std::is_base_of_v<EdgeItem, T>;

// This class may not have any additional members.
struct EdgePtr : public ItemPtrTemplate<EdgeItem>
{
  EdgePtr() = default;
  EdgePtr(Port const& from, Port const& to) { item().set_nodes(from, to); }
};

static_assert(sizeof(EdgePtr) == sizeof(ItemPtr), "EdgePtr may not have any additional members!");

} // namespace cppgraphviz::dot
