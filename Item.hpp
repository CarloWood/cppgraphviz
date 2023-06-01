#pragma once

#include "MemoryRegionToOwnerLinker.hpp"
#include "utils/Badge.h"
#ifdef CWDEBUG
#include "debug_ostream_operators.hpp"
#endif

namespace cppgraphviz {

class Graph;

class Item
{
 protected:
  std::weak_ptr<GraphTracker> root_graph_tracker_;      // The root graph of this Item.
  std::weak_ptr<GraphTracker> parent_graph_tracker_;    // The graph that this Item was added to.

  template<typename T> friend class Class;
  static thread_local MemoryRegionToOwnerLinker current_graph_linker_;

 private:
  void extract_root_graph();

 public:
  // This is used by Graph when it is the root graph.
  Item(std::weak_ptr<GraphTracker> root_graph_tracker) : root_graph_tracker_(std::move(root_graph_tracker)), parent_graph_tracker_{} { }

  // The two constructors below call inform_owner_of which calls this function,
  // to finish initialization, if the memory region owner of object could be found.
  // It is also called by ItemTracker and Graph.
  void set_parent_graph_tracker(std::weak_ptr<GraphTracker> parent_graph_tracker);

 public:
  // This is used by Node that is a member of a class.
  Item(Item* object)
  {
    current_graph_linker_.inform_owner_of(object);      // This sets parent_graph_tracker_.
    extract_root_graph();
  }

  // This is used by Node when it must be added to root_graph.
  Item(std::weak_ptr<GraphTracker> const& root_graph_tracker, Item* object) :
    root_graph_tracker_(root_graph_tracker)
  {
    current_graph_linker_.inform_owner_of(object);      // This sets parent_graph_tracker_ if object is found in a registered memory region.
    // If object does not fall into a registered memory region, then it has to be added to the root graph.
    if (parent_graph_tracker_.use_count() == 0)
      parent_graph_tracker_ = root_graph_tracker;

    if (root_graph_tracker_.use_count() == 0 && parent_graph_tracker_.use_count() > 0)
    {
      // The root graph is still unknown, but now we just initialized the member of a class.
      extract_root_graph();
    }
  }

  Item(Item&& other) :
    root_graph_tracker_(std::move(other.root_graph_tracker_)), parent_graph_tracker_(std::move(other.parent_graph_tracker_)) { }

  // Accessors.

  std::shared_ptr<GraphTracker const> parent_graph_tracker() const
  {
    return parent_graph_tracker_.lock();
  }

  std::shared_ptr<GraphTracker> parent_graph_tracker()
  {
    return parent_graph_tracker_.lock();
  }

  Graph& get_parent_graph()
  {
    std::shared_ptr<GraphTracker> parent_graph_tracker = parent_graph_tracker_.lock();
    // Don't call get_parent_graph if this Item doesn't have one.
    ASSERT(parent_graph_tracker);
    return *parent_graph_tracker;
  }

  Graph const& get_parent_graph() const
  {
    std::shared_ptr<GraphTracker const> parent_graph_tracker = parent_graph_tracker_.lock();
    // Don't call get_parent_graph if this Item doesn't have one.
    ASSERT(parent_graph_tracker);
    return *parent_graph_tracker;
  }

  std::weak_ptr<GraphTracker> const& root_graph_tracker() const { return root_graph_tracker_; }

  virtual void initialize() = 0;

 protected:
  virtual void item_attributes(dot::AttributeList& list) { }
};

template<typename Tracker>
class ItemTemplate : public utils::TrackedObject<Tracker>, public Item
{
 public:
  // This is used by Graph when it is the root graph.
  ItemTemplate(utils::Badge<Graph>) : Item(this->tracker_) { }

  // This is used by Node that is a member of a class.
  ItemTemplate(Item* object) : Item(object) { }

  // This is used by Node when it must be added to root_graph.
  ItemTemplate(std::weak_ptr<GraphTracker> const& root_graph_tracker, Item* object) : Item(root_graph_tracker, object) { }

  ItemTemplate(ItemTemplate&& other) : utils::TrackedObject<Tracker>(std::move(other)), Item(std::move(other)) { }
};

} // namespace cppgraphviz
