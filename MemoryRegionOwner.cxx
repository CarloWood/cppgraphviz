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
  orig.registered_memory_region_ = {};  // Stop the destructor from unregistering this memory region again.
  Item::current_graph_linker_.register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::~MemoryRegionOwner()
{
  // Clean up.
  if (registered_memory_region_.begin())
    Item::current_graph_linker_.unregister_memory_region(registered_memory_region_);
}

} // namespace cppgraphviz
