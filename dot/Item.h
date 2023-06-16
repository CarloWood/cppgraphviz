#pragma once

#include "ItemID.h"
#include "item_types.h"
#include <threadsafe/threadsafe.h>
#include <threadsafe/AIMutex.h>
#include "debug.h"

namespace cppgraphviz::dot {

#if CW_DEBUG
using ItemLockingPolicy = threadsafe::policy::Primitive<AIMutex>;
#else
using ItemLockingPolicy = threadsafe::policy::Primitive<std::mutex>;
#endif

class GraphItem;

// Base class of GraphItem, EdgeItem and NodeItem.
//
// This class provides a unique id for each graph (item) and a general attribute list.
class Item : public AIRefCount, public ItemID
{
 public:
  using unlocked_type = threadsafe::UnlockedBase<Item, ItemLockingPolicy>;
  using graph_item_type = GraphItem;

  Item() : ItemID(s_unique_id_context.get_id()) { }
  Item(Item const& other) = delete;
  Item(Item&& other) = delete;

  bool is_graph() const { return dot::is_graph(item_type()); }

  virtual item_type_type item_type() const = 0;
  virtual void write_dot_to(std::ostream& os, std::string& indentation) const = 0;
};

} // namespace cppgraphviz::dot
