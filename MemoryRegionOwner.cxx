#include "sys.h"
#include "MemoryRegionOwner.h"
#include "Item.h"

namespace cppgraphviz {

MemoryRegionOwner::MemoryRegionOwner(MemoryRegion memory_region) : registered_memory_region_{memory_region}
{
  Item::current_graph_linker_.register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::MemoryRegionOwner(MemoryRegionOwner&& orig, MemoryRegion memory_region) :
  utils::TrackedObject<MemoryRegionOwnerTracker>(std::move(orig)),
  registered_memory_region_{memory_region}
{
  // The old memory region should already have been removed before we get here because
  // we just moved the TrackedObject<MemoryRegionOwnerTracker> base class of object,
  // and unregister_memory_region accesses that (for debug output).
  // Therefore, we must stop the destructor from unregistering this memory region again.
  orig.registered_memory_region_ = {};
  Item::current_graph_linker_.register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::~MemoryRegionOwner()
{
  // Clean up.
  if (registered_memory_region_.begin())
    Item::current_graph_linker_.unregister_memory_region(registered_memory_region_);
}

} // namespace cppgraphviz
