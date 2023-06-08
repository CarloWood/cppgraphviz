#include "sys.h"
#include "MemoryRegionOwner.h"
#include "Item.h"

namespace cppgraphviz {

MemoryRegionOwner::MemoryRegionOwner(MemoryRegion memory_region) : registered_memory_region_{memory_region}
{
  MemoryRegionToOwnerLinkerSingleton::instance().register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::MemoryRegionOwner(MemoryRegionOwner&& orig, MemoryRegion memory_region) :
  utils::TrackedObject<MemoryRegionOwnerTracker>(std::move(orig)),
  registered_memory_region_{memory_region}
{
  MemoryRegionToOwnerLinkerSingleton::instance().unregister_memory_region(orig.registered_memory_region_);
  // Stop the destructor from unregistering this memory region again.
  orig.registered_memory_region_ = {};
  MemoryRegionToOwnerLinkerSingleton::instance().register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::~MemoryRegionOwner()
{
  // Clean up.
  if (registered_memory_region_.begin())
    MemoryRegionToOwnerLinkerSingleton::instance().unregister_memory_region(registered_memory_region_);
}

} // namespace cppgraphviz
