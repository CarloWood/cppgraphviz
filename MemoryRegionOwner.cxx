#include "sys.h"
#include "MemoryRegionOwner.h"
#include "Item.h"

namespace cppgraphviz {

MemoryRegionOwner::MemoryRegionOwner(MemoryRegion memory_region) : registered_memory_region_{true}
{
  // If a derived class uses this constructor to register a memory region then
  // that region must be for the whole class, including this base class.
  ASSERT((MemoryRegion{reinterpret_cast<char*>(this), 1}.lays_within(memory_region)));
  Item::current_graph_linker_.register_new_memory_region_for(memory_region, tracker_);
}

MemoryRegionOwner::~MemoryRegionOwner()
{
  // Clean up.
  if (registered_memory_region_)
    Item::current_graph_linker_.unregister_memory_region({reinterpret_cast<char*>(this), 1});
}

} // namespace cppgraphviz
