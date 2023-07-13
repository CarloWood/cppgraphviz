#pragma once

#include "MemoryRegionToOwnerLinker.h"
#include "dot/AttributeList.h"
#include "dot/Graph.h"
#include "utils/Badge.h"
#include "utils/AIRefCount.h"
#include "threadsafe/ObjectTracker.h"

namespace cppgraphviz {

class locked_Graph;
class MemoryRegionOwner;
class NodeTracker;
class GraphTracker;
class Graph;
class locked_Graph;

class Item
{
 public:
  using memory_region_to_owner_linker_type = MemoryRegionToOwnerLinkerSingleton::linker_type;

 protected:
  std::weak_ptr<GraphTracker> root_graph_tracker_;      // The root graph of this Item.
  std::weak_ptr<GraphTracker> parent_graph_tracker_;    // The graph that this Item was added to.

 private:
  void extract_root_graph();

 public:
  // This is used by Graph when it is the root graph.
  Item(std::weak_ptr<GraphTracker> root_graph_tracker) : root_graph_tracker_(std::move(root_graph_tracker)), parent_graph_tracker_{} { }

  // Called for Array elements, that have the root graph set when being added to the memory region of an array for the first time.
  void set_root_graph_tracker(std::weak_ptr<GraphTracker> root_graph_tracker);

  // The two constructors below call inform_owner_of which calls this function,
  // to finish initialization, if the memory region owner of object could be found.
  // It is also called by Class and Graph.
  void set_parent_graph_tracker(std::weak_ptr<GraphTracker> parent_graph_tracker);

 public:
  // This is used by Node that is a member of a class.
  Item(Item* object)
  {
    // Take the read-lock on the singleton.
    memory_region_to_owner_linker_type::rat memory_region_to_owner_linker_r(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
    memory_region_to_owner_linker_r->inform_owner_of(object);     // This sets parent_graph_tracker_.
    extract_root_graph();
  }

  // This is used by Node when it must be added to root_graph.
  // Only if object is a NodeItem/NodeTracker, node_ptr_ptr will be set, otherwise it is nullptr.
  Item(std::weak_ptr<GraphTracker> const& root_graph_tracker, Item* object, dot::NodePtr* node_ptr_ptr = nullptr) :
    root_graph_tracker_(root_graph_tracker)
  {
    // Take the read-lock on the singleton.
    memory_region_to_owner_linker_type::rat memory_region_to_owner_linker_r(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
    // This call sets parent_graph_tracker_ if object is found in a registered memory region
    // (i.e. an indexed container that is added to the root graph).
    memory_region_to_owner_linker_r->inform_owner_of(object, node_ptr_ptr);
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

  threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>::wat parent_graph_wat();
  threadsafe::ObjectTracker<Graph, locked_Graph, dot::ItemLockingPolicy>::rat parent_graph_rat();

  std::weak_ptr<GraphTracker> const& root_graph_tracker() const { return root_graph_tracker_; }

  virtual void initialize() = 0;

 protected:
  virtual void item_attributes(dot::AttributeList& list) { }
};

template<typename TrackedType, typename Tracker>
class ItemTemplate : public threadsafe::TrackedObject<TrackedType, Tracker>, public Item
{
 public:
  // This is used by locked_Graph when it is the root graph.
  ItemTemplate(utils::Badge<locked_Graph>) : Item(this->tracker_) { }

  // This is used by Node that is a member of a class.
  ItemTemplate(Item* object) : Item(object) { }

  // This is used by Node when it must be added to root_graph.
  template<typename T = Tracker, typename std::enable_if<!std::is_same<T, NodeTracker>::value>::type* = nullptr>
  ItemTemplate(std::weak_ptr<GraphTracker> const& root_graph_tracker, Item* object) :
    Item(root_graph_tracker, object) { }

  template<typename T = Tracker, typename std::enable_if<std::is_same<T, NodeTracker>::value>::type* = nullptr>
  ItemTemplate(std::weak_ptr<GraphTracker> const& root_graph_tracker, Item* object) :
    Item(root_graph_tracker, object, &this->tracker().node_ptr()) { }

  ItemTemplate(ItemTemplate&& orig) : threadsafe::TrackedObject<TrackedType, Tracker>(std::move(orig)), Item(std::move(orig)) { }

  // Return the value of the "what" attribute.
  std::string get_what() const;
};

template<typename TrackedType, typename Tracker>
std::string ItemTemplate<TrackedType, Tracker>::get_what() const
{
  std::string what;
  if constexpr (std::is_same_v<Tracker, NodeTracker>)
    what = std::string(dot::NodePtr::unlocked_type::crat{this->tracker_->node_ptr().item()}->attribute_list().get_value("what"));
  else
    what = std::string(dot::GraphPtr::unlocked_type::crat{this->tracker_->graph_ptr().item()}->attribute_list().get_value("what"));
  return what;
}

} // namespace cppgraphviz
