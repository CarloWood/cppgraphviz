#include "sys.h"
#include "MemoryRegionToOwnerLinker.hpp"
#include "Node.hpp"
#include <exception>
#ifdef CWDEBUG
#include "utils/has_print_on.h"
#include "debug_ostream_operators.hpp"
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

void MemoryRegionToOwner::inform_owner(MemoryRegion const& used) const
{
  auto memory_region_owner_tracker = memory_region_owner_tracker_.lock();
  if (memory_region_owner_tracker)
  {
    memory_region_owner_tracker->tracked_object().on_memory_region_usage(used);
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

  auto iter = memory_region_to_owner_map_.find(memory_region);
  if (iter == memory_region_to_owner_map_.end())
    return false;

  Dout(dc::notice, "before:\n" << *this);

  if (!iter->second.erase_memory_region_to_owner(memory_region))
    memory_region_to_owner_map_.erase(iter);

  Dout(dc::notice, "after:\n" << *this);

  return true;
}

void MemoryRegionToOwnerLinker::inform_owner_of(void* object) const
{
  DoutEntering(dc::notice, "MemoryRegionToOwnerLinker::inform_owner_of(" << object << ")");

  // The size of the actual object is probably larger (MemoryRegionOwner is just a base class), but this will have to do.
  MemoryRegion memory_region(reinterpret_cast<char*>(object), sizeof(MemoryRegionOwner));

  inform_owner_of(memory_region);
}

void MemoryRegionToOwnerLinker::inform_owner_of(MemoryRegion const& memory_region) const
{
  DoutEntering(dc::notice, "MemoryRegionToOwnerLinker::inform_owner_of(" << memory_region << ")");

  auto iter = memory_region_to_owner_map_.find(memory_region);

  // This can happen for example when creating a temporary in the constructor of a class;
  // we just don't add those to any graph at all until they are moved (or copied) into
  // a memory region that belongs to a managed Class.
  if (iter == memory_region_to_owner_map_.end())
    return;

  iter->second.inform_owner_of(iter->first, memory_region);
}

void MemoryRegionToOwnerLinker::inform_owner_of(MemoryRegionToOwner const& default_owner, MemoryRegion const& memory_region) const
{
  DoutEntering(dc::notice, "MemoryRegionToOwnerLinker::inform_owner_of(" << default_owner << ", " << memory_region << ")");

  auto iter = memory_region_to_owner_map_.find(memory_region);

  if (iter == memory_region_to_owner_map_.end())
  {
    Dout(dc::notice, "not found; using: " << default_owner);
    default_owner.inform_owner(memory_region);
    return;
  }

  iter->second.inform_owner_of(iter->first, memory_region);
}

void MemoryRegionToOwnerLinker::register_new_memory_region_for(MemoryRegion memory_region, std::weak_ptr<MemoryRegionOwnerTracker> const& owner)
{
  DoutEntering(dc::notice, "register_new_memory_region_for(" << memory_region << ", " << owner << ")");
  Dout(dc::notice, "before:\n" << *this);

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

  bool success = erase_memory_region_to_owner(memory_region);
  // Paranoia check.
  ASSERT(success);
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
