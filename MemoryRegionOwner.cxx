#include "sys.h"
#include "MemoryRegionOwner.h"
#include "MemoryRegionToOwnerLinker.h"
#include "Item.h"

namespace cppgraphviz {

// Using this as type name feels more intuitive (better fits the pattern that I am used to).
// Defined here because it can't be defined in (the header of) MemoryRegionOwner.
using memory_region_to_owner_linker_type = MemoryRegionToOwnerLinkerSingleton::linker_type;

MemoryRegionOwner::MemoryRegionOwner(MemoryRegion memory_region) : registered_memory_region_{memory_region}
{
  memory_region_to_owner_linker_type::wat memory_region_to_owner_linker_w(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
  memory_region_to_owner_linker_w->register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::MemoryRegionOwner(MemoryRegionOwner&& orig, MemoryRegion memory_region) :
  utils::TrackedObject<MemoryRegionOwnerTracker>(std::move(orig)),
  registered_memory_region_{memory_region}
{
  memory_region_to_owner_linker_type::wat memory_region_to_owner_linker_w(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
  memory_region_to_owner_linker_w->unregister_memory_region(orig.registered_memory_region_);
  // Stop the destructor from unregistering this memory region again.
  orig.registered_memory_region_ = {};
  memory_region_to_owner_linker_w->register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::~MemoryRegionOwner()
{
  // Clean up.
  if (registered_memory_region_.begin())
  {
    memory_region_to_owner_linker_type::wat memory_region_to_owner_linker_w(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
    memory_region_to_owner_linker_w->unregister_memory_region(registered_memory_region_);
  }
}

} // namespace cppgraphviz
