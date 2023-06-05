#include "sys.h"
#include "Array.h"
#include "Node.h"

namespace cppgraphviz {

ArrayMemoryRegionOwner::ArrayMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
    char* begin, size_t element_size, size_t number_of_elements, std::type_info const& index_type_info,
    std::string const& demangled_index_type_name) :
  MemoryRegionOwner({ begin, element_size * number_of_elements }),
  begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements),
  id_to_node_map_(element_size_)
{
  DoutEntering(dc::notice, "ArrayMemoryRegionOwner::ArrayMemoryRegionOwner(" << root_graph << ", " <<
      (void*)begin << ", " << element_size << ", " << number_of_elements << ", index_type_info)");

  table_node_ptr_->add_attribute({"what", "ArrayMemoryRegionOwner::table_node_ptr_"});
  // Instead of copying elements, we create new dot::NodePtr objects and use
  // those temporarily until they can be overwritten later in on_memory_region_usage.
  table_node_ptr_->copy_elements([](size_t i){
        dot::NodePtr node_ptr;
        node_ptr->add_attribute({"what", "default NodePtr for Array"});
        return node_ptr;
      }, number_of_elements);
  std::shared_ptr<GraphTracker> root_graph_tracker = root_graph.lock();
  // Paranoia check.
  ASSERT(root_graph_tracker);
  // Create a key that is unique for a _Index / root_graph pair.
  uint64_t key = utils::pointer_hash_combine(index_type_info.hash_code(), root_graph_tracker.get());

  auto ibp = index_container_sets_.try_emplace(key, "ArrayMemoryRegionOwner::index_container_sets_");
  IndexedContainerSet& indexed_container_set = ibp.first->second;
  if (ibp.second)
  {
    root_graph_tracker->graph_ptr()->insert(indexed_container_set);
    ibp.first->second.set_label(std::string("Array[") + demangled_index_type_name + "]");
  }
  indexed_container_set.add_container(table_node_ptr_);
  // Add this array to the root graph, so that it will call initialize before writing the dot file.
  root_graph_tracker->tracked_object().add_array(tracker_);
}

void ArrayMemoryRegionOwner::on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr)
{
  DoutEntering(dc::notice, "ArrayMemoryRegionOwner::on_memory_region_usage(" <<
      item_memory_region << ", " << node_ptr_ptr << ") [" << this << "]");
  char* item_begin = item_memory_region.begin();
  Item* item = reinterpret_cast<Item*>(item_begin);
  ptrdiff_t offset = item_begin - begin_;
  size_t index = offset / element_size_;
  ASSERT(0 <= index && index < number_of_elements_);
  // node_ptr_ptr can't be nullptr, because if this is to be added to a table node then
  // the item has to be a NodeItem (still being constructed though).
  ASSERT(node_ptr_ptr);
  dot::NodePtr& node_ptr = *node_ptr_ptr;
  table_node_ptr_->replace_element(index, node_ptr);
  // And item has to be Node.
  Node* node = static_cast<Node*>(item);
  std::weak_ptr<NodeTracker> weak_node_tracker = *node;
  Dout(dc::notice, "Mapping id_to_node_map_[" << index << "] = " << weak_node_tracker);
  id_to_node_map_[index] = weak_node_tracker;
}

void ArrayMemoryRegionOwner::call_initialize_on_elements()
{
  DoutEntering(dc::notice, "ArrayMemoryRegionOwner::call_initialize_on_elements() [" << this << "]");
  size_t index = 0;
  // This calls the lambda for indices 0, 1, 2, ...
  table_node_ptr_->for_all_elements([this, &index](dot::NodeItem& node_item){
    Dout(dc::notice, "index = " << index);
    auto node_tracker = id_to_node_map_[index].lock();
    ++index;
    if (node_tracker)
      node_tracker->tracked_object().initialize();
  });
}

//static
std::map<uint64_t, IndexedContainerSet> ArrayMemoryRegionOwner::index_container_sets_;

} // namespace cppgraphviz
