#include "sys.h"
#include "MemoryRegionToOwnerLinker.h"
#include "Node.h"
#include <exception>
#ifdef CWDEBUG
#include "utils/has_print_on.h"
#include "debug_ostream_operators.h"
#endif

namespace cppgraphviz {

std::weak_ptr<MemoryRegionOwnerTracker> const& MemoryRegionToOwner::get_memory_region_owner_tracker(
    MemoryRegion const& memory_region_key, std::weak_ptr<MemoryRegionOwnerTracker> const& default_memory_region_owner_tracker) const
{
  DoutEntering(dc::notice, "MemoryRegionToOwner::get_memory_region_owner_tracker(" <<
      memory_region_key << ", " << default_memory_region_owner_tracker << ")");

  if (memory_region_key.lays_within(memory_region_))
  {
    Dout(dc::notice, memory_region_key << " lays within " << memory_region_ << ": returning " << memory_region_owner_tracker_);
    return memory_region_owner_tracker_;
  }

  // It should not be possible that a memory region is being constructed that only partially falls within the current memory area.
  ASSERT(!memory_region_key.overlaps_with(memory_region_));

  Dout(dc::notice, memory_region_key << " does not lay within " << memory_region_ << ": returning " << default_memory_region_owner_tracker);
  return default_memory_region_owner_tracker;
}

void MemoryRegionToOwner::inform_owner(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) const
{
  auto memory_region_owner_tracker = memory_region_owner_tracker_.lock();
  if (memory_region_owner_tracker)
  {
    memory_region_owner_tracker->tracked_object().on_memory_region_usage(item_memory_region, node_ptr_ptr);
  }
}

#ifdef CWDEBUG
void MemoryRegionToOwner::print_on(std::ostream& os) const
{
  os << memory_region_ << " : " << memory_region_owner_tracker_;
}
#endif

bool MemoryRegionToOwnerLinker::erase_memory_region_to_owner(MemoryRegion const& memory_region)
{
  DoutEntering(dc::notice, "erase_memory_region_to_owner(" << memory_region << ")");

  // find returns any region that overlaps with memory_region.
  // The way memory regions are added (new memory regions can only fall completely inside a previous registered
  // memory region or completely outside of any) there are not partial overlaps.
  auto iter = memory_region_to_owner_map_.find(memory_region);
  if (iter == memory_region_to_owner_map_.end())
    return false;

  // The first time we have a match it is either an exact match, or it contains
  // the region that we are looking for. In the latter can we can't call
  // iter->second.erase_memory_region_to_owner(memory_region) because that would
  // then "find" and remove regions that are actually contained inside our region.
  if (iter->first.memory_region() == memory_region)     // Exact match: we found it.
    memory_region_to_owner_map_.erase(iter);
  else
  {
#if CW_DEBUG
    bool success =
#endif
      // Find and remove memory_region from the container that we just found, which encapsulates it.
      iter->second.erase_memory_region_to_owner(memory_region);
    // This should always succeed: we shouldn't be trying to remove regions that aren't there.
    ASSERT(success);
  }

  Dout(dc::notice, "after:\n" << *this);

  return true;
}

void MemoryRegionToOwnerLinker::inform_owner_of(Item* item, dot::NodePtr* node_ptr_ptr) const
{
  DoutEntering(dc::notice, "MemoryRegionToOwnerLinker::inform_owner_of(" << item << ", " << node_ptr_ptr << ")");

  MemoryRegion item_memory_region(reinterpret_cast<char*>(item), sizeof(Item));
  auto iter = memory_region_to_owner_map_.find(item_memory_region);

  // This can happen for example when creating a temporary in the constructor of a class;
  // we just don't add those to any graph at all until they are moved (or copied) into
  // a memory region that belongs to a managed Class.
  if (iter == memory_region_to_owner_map_.end())
    return;

  iter->second.inform_owner_of(iter->first, item_memory_region, node_ptr_ptr);
}

void MemoryRegionToOwnerLinker::inform_owner_of(
    MemoryRegionToOwner const& default_owner, MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) const
{
  DoutEntering(dc::notice,
      "MemoryRegionToOwnerLinker::inform_owner_of(" << default_owner << ", " << item_memory_region << ", " << node_ptr_ptr << ")");

  auto iter = memory_region_to_owner_map_.find(item_memory_region);

  if (iter == memory_region_to_owner_map_.end())
  {
    Dout(dc::notice, "not found; using: " << default_owner);
    default_owner.inform_owner(item_memory_region, node_ptr_ptr);
    return;
  }

  iter->second.inform_owner_of(iter->first, item_memory_region, node_ptr_ptr);
}

void MemoryRegionToOwnerLinker::register_new_memory_region_for(MemoryRegion memory_region, std::weak_ptr<MemoryRegionOwnerTracker> const& owner)
{
  DoutEntering(dc::notice, "register_new_memory_region_for(" << memory_region << ", " << owner << ")");

  // MemoryRegionToOwner, MemoryRegionToOwnerLinker
  MemoryRegionToOwner memory_region_to_owner(memory_region, owner);
  auto ibp = memory_region_to_owner_map_.try_emplace(std::move(memory_region_to_owner));
  while (!ibp.second)
    ibp = ibp.first->second.memory_region_to_owner_map_.try_emplace(std::move(memory_region_to_owner));

  Dout(dc::notice, "after:\n" << *this);
}

void MemoryRegionToOwnerLinker::unregister_memory_region(MemoryRegion memory_region)
{
  DoutEntering(dc::notice, "unregister_memory_region(" << memory_region << ")");

  // It is possible that we fail to remove a memory region because it fell inside
  // another memory region that was already removed. This happens because when
  // *moving* a class, the order in which the move-constructors are being called
  // is dictated by the construction order (base classes first, left to right, and
  // then the members of the class). Destruction happens in reverse order.
  // As a result, the move-constructors "destruct" (remove) the memory regions in
  // the "wrong" order.
  (void)erase_memory_region_to_owner(memory_region);
}

#ifdef CWDEBUG
void MemoryRegionToOwnerLinker::print_on_with_indentation(std::ostream& os, std::string indentation) const
{
  for (auto& key_value_pair : memory_region_to_owner_map_)
  {
    MemoryRegionToOwner const& memory_region_to_owner = key_value_pair.first;
    MemoryRegionToOwnerLinker const& memory_region_to_owner_linker = key_value_pair.second;
    os << indentation << memory_region_to_owner << '\n';
    if (!memory_region_to_owner_linker.empty())
      memory_region_to_owner_linker.print_on_with_indentation(os, indentation + "    ");
  }
}
#endif

} // namespace cppgraphviz

// Instantiate the singleton.
static SingletonInstance<cppgraphviz::MemoryRegionToOwnerLinkerSingleton> dummy;
