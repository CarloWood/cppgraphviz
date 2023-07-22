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
  owner_->unregister_memory_region({static_cast<char*>(ptr), bytes});
  return std::pmr::new_delete_resource()->deallocate(ptr, bytes, alignment);
}

// Using this as type name feels more intuitive (better fits the pattern that I am used to).
// Defined here because it can't be defined in (the header of) MemoryRegionOwner.
using memory_region_to_owner_linker_type = MemoryRegionToOwnerLinkerSingleton::linker_type;

VectorMemoryRegionOwner::VectorMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
    std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what) :
  LabelNode(root_graph, what), allocater_(this)
{
  DoutEntering(dc::notice, "VectorMemoryRegionOwner(" << root_graph <<
      ", index_type_info, \"" << demangled_index_type_name << "\", \"" << what << "\")");
  initialize(index_type_info, demangled_index_type_name);
}

void VectorMemoryRegionOwner::initialize(std::type_info const& index_type_info, std::string const& demangled_index_type_name)
{
  {
    dot::TableNodePtr::unlocked_type::wat table_node_ptr_w{table_node_ptr_.item()};
    table_node_ptr_w->add_attribute({"what", "VectorMemoryRegionOwner::table_node_ptr_"});
    // Instead of copying elements, we create new dot::NodePtr objects and use
    // those temporarily until they can be overwritten later in on_memory_region_usage.
    table_node_ptr_w->copy_elements([](size_t i){
          dot::NodePtr node_ptr;
          dot::NodePtr::unlocked_type::wat{node_ptr.item()}->add_attribute({"what", "default NodePtr for Vector"});
          return node_ptr;
        }, 0);
  }

  std::shared_ptr<GraphTracker> root_graph_tracker = root_graph_tracker_.lock();
  // Paranoia check.
  ASSERT(root_graph_tracker);
  // Create a key that is unique for a _Index / root_graph pair.
  uint64_t key = utils::pointer_hash_combine(index_type_info.hash_code(), root_graph_tracker.get());

  {
    index_container_sets_t::wat index_container_sets_w{index_container_sets_};
    auto ibp = index_container_sets_w->try_emplace(key, "VectorMemoryRegionOwner::index_container_sets_");
    IndexedContainerSet& indexed_container_set = ibp.first->second;
    if (ibp.second)
    {
      dot::GraphPtr::unlocked_type::wat{root_graph_tracker->graph_ptr().item()}->insert(indexed_container_set);
      ibp.first->second.set_label(std::string("Vector[") + demangled_index_type_name + "]");
    }
    indexed_container_set.add_container(table_node_ptr_, dot::TableNodePtr::unlocked_type::crat{table_node_ptr_.item()});
  }
  // Add this vector to the root graph, so that it will call initialize before writing the dot file.
//FIXME  root_graph_tracker->tracked_wat()->add_array(MemoryRegionOwner::tracker_);
}

void VectorMemoryRegionOwner::register_new_memory_region(MemoryRegion memory_region)
{
  memory_region_to_owner_linker_type::wat memory_region_to_owner_linker_w(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
  memory_region_to_owner_linker_w->register_new_memory_region_for(memory_region, MemoryRegionOwner::tracker_);
}

void VectorMemoryRegionOwner::unregister_memory_region(MemoryRegion memory_region)
{
  memory_region_to_owner_linker_type::wat memory_region_to_owner_linker_w(MemoryRegionToOwnerLinkerSingleton::instance().linker_);
  memory_region_to_owner_linker_w->unregister_memory_region(memory_region);
}

void VectorMemoryRegionOwner::on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr)
{
  DoutEntering(dc::notice, "VectorMemoryRegionOwner::on_memory_region_usage(" <<
      item_memory_region << ", " << node_ptr_ptr << ") [" << this << "]");

  char* item_begin = item_memory_region.begin();
  Item* item = reinterpret_cast<Item*>(item_begin);

  // Vector elements should be created without root graph. They get that when they are copied into the array, here.
  item->set_root_graph_tracker(root_graph_tracker_);
}

//static
VectorMemoryRegionOwner::index_container_sets_t VectorMemoryRegionOwner::index_container_sets_;

} // namespace cppgraphviz
