#include "sys.h"
#include "Vector.h"
#include "MemoryRegionToOwnerLinker.h"
#include "Graph.h"
#include "utils/pointer_hash.h"

namespace cppgraphviz {

void* TrackingAllocator::do_allocate(std::size_t bytes, std::size_t alignment)
{
  Dout(dc::notice, "do_allocate(" << bytes << ", " << alignment << ")");
  void* ptr = std::pmr::new_delete_resource()->allocate(bytes, alignment);
  owner_->register_new_memory_region({static_cast<char*>(ptr), bytes});
  return ptr;
}

void TrackingAllocator::do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment)
{
  Dout(dc::notice, "do_deallocate(" << ptr << ", " << bytes << ", " << alignment << ")");
  IndexedContainerMemoryRegionOwner::unregister_memory_region({static_cast<char*>(ptr), bytes});
  return std::pmr::new_delete_resource()->deallocate(ptr, bytes, alignment);
}

} // namespace cppgraphviz
