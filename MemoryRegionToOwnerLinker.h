#pragma once

#include "MemoryRegionOwner.h"
#include "dot/Node.h"
#include "utils/Singleton.h"
#include "threadsafe/threadsafe.h"

namespace cppgraphviz {
using utils::has_print_on::operator<<;

class MemoryRegionToOwnerLinker;
class Item;

class MemoryRegionToOwner
{
 private:
  MemoryRegion memory_region_;
  std::weak_ptr<MemoryRegionOwnerTracker> memory_region_owner_tracker_;

 private:
  friend class MemoryRegionToOwnerLinker;
  // Use to construct a key for MemoryRegionToOwnerLinker::memory_region_to_owner_map_.
  MemoryRegionToOwner(MemoryRegion const& memory_region) : memory_region_(memory_region) { }
  friend bool operator<(MemoryRegionToOwner const& lhs, MemoryRegionToOwner const& rhs) { return lhs.memory_region_ < rhs.memory_region_; }

 public:
  MemoryRegionToOwner(MemoryRegion const& memory_region, std::weak_ptr<MemoryRegionOwnerTracker> const& memory_region_owner_tracker) :
    memory_region_(memory_region), memory_region_owner_tracker_(memory_region_owner_tracker) { }

  MemoryRegionToOwner(MemoryRegionToOwner&& other) :
    memory_region_(other.memory_region_), memory_region_owner_tracker_(std::move(other.memory_region_owner_tracker_)) { }

  // Return memory_region_owner_tracker_ if [item, item + size> falls inside memory_region_,
  // otherwise return default_memory_region_owner_tracker.
  std::weak_ptr<MemoryRegionOwnerTracker> const& get_memory_region_owner_tracker(
      MemoryRegion const& memory_region_key, std::weak_ptr<MemoryRegionOwnerTracker> const& default_memory_region_owner_tracker) const;

  void inform_owner(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr) const;

  // Accessor.
  MemoryRegion const& memory_region() const { return memory_region_; }

#ifdef CWDEBUG
  void print_on(std::ostream& os) const;
#endif
};

// MemoryRegion's can be inside one another, but they can not
// partially overlap.
//
// ---
//  ^
//  |     - MemoryRegion 1
//  v
// ---
//
// ---
//  ^
//  |     - MemoryRegion 2
//  |
//  | ---
//  |  ^
//  |  |  - MemoryRegion 3
//  |  v
//  | ---
//  |
//  v
// ---
//

class MemoryRegionToOwnerLinker
{
 private:
  std::map<MemoryRegionToOwner, MemoryRegionToOwnerLinker> memory_region_to_owner_map_;

 private:
  // Returns true if successful.
  bool erase_memory_region_to_owner(MemoryRegion const& memory_region);

  // Called by the public inform_owner_of.
  void inform_owner_of(MemoryRegionToOwner const& default_owner, MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr) const;

 public:
  void inform_owner_of(Item* item, dot::NodePtr* node_ptr = nullptr) const;

  void register_new_memory_region_for(MemoryRegion memory_region, std::weak_ptr<MemoryRegionOwnerTracker> const& owner);
  void unregister_memory_region(MemoryRegion memory_region);

  bool empty() const { return memory_region_to_owner_map_.empty(); }

#ifdef CWDEBUG
  void print_on_with_indentation(std::ostream& os, std::string indentation) const;
  void print_on(std::ostream& os) const { print_on_with_indentation(os, {}); }
#endif
};

class MemoryRegionToOwnerLinkerSingleton : public Singleton<MemoryRegionToOwnerLinkerSingleton>
{
  friend_Instance;
 private:
  MemoryRegionToOwnerLinkerSingleton() = default;
  ~MemoryRegionToOwnerLinkerSingleton() = default;
  MemoryRegionToOwnerLinkerSingleton(MemoryRegionToOwnerLinkerSingleton const&) = delete;

 public:
  using linker_type = threadsafe::Unlocked<MemoryRegionToOwnerLinker, threadsafe::policy::Primitive<std::mutex>>;
  linker_type linker_;
};

} // namespace cppgraphviz
