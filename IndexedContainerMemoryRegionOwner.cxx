#include "sys.h"
#include "Array.h"
#include "utils/pointer_hash.h"

namespace cppgraphviz {

IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
    char* begin, size_t element_size, size_t number_of_elements, std::type_info const& index_type_info,
    std::string const& demangled_index_type_name, std::string_view what) :
  MemoryRegionOwner({ begin, element_size * number_of_elements }),
  LabelNode(root_graph, what),
  begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements),
  id_to_node_map_(number_of_elements_)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(" << root_graph << ", " <<
      (void*)begin << ", " << element_size << ", " << number_of_elements << ", index_type_info, \"" << what << "\")");

  {
    dot::TableNodePtr::unlocked_type::wat table_node_ptr_w{table_node_ptr_.item()};
    table_node_ptr_w->add_attribute({"what", "IndexedContainerMemoryRegionOwner::table_node_ptr_"});
    // Instead of copying elements, we create new dot::NodePtr objects and use
    // those temporarily until they can be overwritten later in on_memory_region_usage.
    table_node_ptr_w->copy_elements([](size_t i){
          dot::NodePtr node_ptr;
          dot::NodePtr::unlocked_type::wat{node_ptr.item()}->add_attribute({"what", "default NodePtr for Array"});
          return node_ptr;
        }, number_of_elements);
  }
  std::shared_ptr<GraphTracker> root_graph_tracker = root_graph.lock();
  // Paranoia check.
  ASSERT(root_graph_tracker);
  // Create a key that is unique for a _Index / root_graph pair.
  uint64_t key = utils::pointer_hash_combine(index_type_info.hash_code(), root_graph_tracker.get());

  {
    indexed_container_sets_t::wat indexed_container_sets_w{indexed_container_sets_};
    auto ibp = indexed_container_sets_w->try_emplace(key, "IndexedContainerMemoryRegionOwner::indexed_container_sets_");
    IndexedContainerSet& indexed_container_set = ibp.first->second;
    if (ibp.second)
    {
      dot::GraphPtr::unlocked_type::wat{root_graph_tracker->graph_ptr().item()}->insert(indexed_container_set);
      ibp.first->second.set_label(std::string("Array[") + demangled_index_type_name + "]");
    }
    indexed_container_set.add_container(table_node_ptr_, dot::TableNodePtr::unlocked_type::crat{table_node_ptr_.item()});
  }
  // Add this array to the root graph, so that it will call initialize before writing the dot file.
  root_graph_tracker->tracked_wat()->add_array(MemoryRegionOwner::tracker_);
}

IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(char* begin, size_t element_size, size_t number_of_elements,
    std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what) :
  MemoryRegionOwner({ begin, element_size * number_of_elements }),
  LabelNode(what),
  begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(" <<
      (void*)begin << ", " << element_size << ", " << number_of_elements << ", index_type_info, " <<
      demangled_index_type_name << ", \"" << what << "\")");
  // Not implemented; among others we'll have to get the root graph from the memory region owner that we're in now(?)
  ASSERT(false);
}

IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
    char* begin, std::type_info const& index_type_info, std::string_view what) :
  MemoryRegionOwner({ begin, other->element_size_ * other->number_of_elements_ }),
  LabelNode(other, what),
  begin_(begin), element_size_(other->element_size_), number_of_elements_(other->number_of_elements_),
  id_to_node_map_(number_of_elements_)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner const& " << other.operator->() << ", " <<
      (void*)begin << ", index_type_info, \"" << what << "\")");

  {
    dot::TableNodePtr::unlocked_type::wat table_node_w{table_node_ptr_.item()};
    table_node_w->add_attribute({"what", "IndexedContainerMemoryRegionOwner::table_node_ptr_"});
    // Instead of copying elements, we create new dot::NodePtr objects and use
    // those temporarily until they can be overwritten later in on_memory_region_usage.
    table_node_w->copy_elements([](size_t i){
          dot::NodePtr node_ptr;
          dot::NodePtr::unlocked_type::wat{node_ptr.item()}->add_attribute({"what", "default NodePtr for Array"});
          return node_ptr;
        }, number_of_elements_);
  }
  std::shared_ptr<GraphTracker> root_graph = root_graph_tracker().lock();
  // Paranoia check.
  ASSERT(root_graph);
  // Create a key that is unique for a _Index / root_graph pair.
  uint64_t key = utils::pointer_hash_combine(index_type_info.hash_code(), root_graph.get());
  {
    indexed_container_sets_t::wat indexed_container_sets_w{indexed_container_sets_};
    auto iter = indexed_container_sets_w->find(key);
    ASSERT(iter != indexed_container_sets_w->end());
    IndexedContainerSet& indexed_container_set = iter->second;
    indexed_container_set.add_container(table_node_ptr_, dot::TableNodePtr::unlocked_type::crat{table_node_ptr_.item()});
  }
  // Add this array to the root graph, so that it will call initialize before writing the dot file.
  root_graph->tracked_wat()->add_array(MemoryRegionOwner::tracker_);
}

IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> orig, char* begin, std::string_view what) :
  MemoryRegionOwner(*orig, { begin, orig->element_size_ * orig->number_of_elements_ }),
  LabelNode(std::move(orig), what),
  begin_(begin), element_size_(orig->element_size_), number_of_elements_(orig->number_of_elements_),
  table_node_ptr_(std::move(orig->table_node_ptr_)), id_to_node_map_(std::move(orig->id_to_node_map_))
{
  DoutEntering(dc::notice,
      "IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner&& " << orig.operator->() << ", " <<
      (void*)begin << ", \"" << what << "\") [" << this << "]");
}

void IndexedContainerMemoryRegionOwner::on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::on_memory_region_usage(" <<
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
  dot::TableNodePtr::unlocked_type::wat{table_node_ptr_.item()}->replace_element(index, node_ptr);
  // And item has to be Node.
  Node* node = static_cast<Node*>(item);
  std::weak_ptr<NodeTracker> weak_node_tracker = *node;
  Dout(dc::notice, "Mapping id_to_node_map_[" << index << "] = " << weak_node_tracker);
  id_to_node_map_[index] = weak_node_tracker;

  // Array elements should be created without root graph. They get that when they are copied into the array, here.
  item->set_root_graph_tracker(root_graph_tracker_);
}

void IndexedContainerMemoryRegionOwner::call_initialize_on_elements()
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::call_initialize_on_elements() [" << this << "]");
  size_t index = 0;
  // This calls the lambda for indices 0, 1, 2, ...
  dot::TableNodePtr::unlocked_type::wat{table_node_ptr_.item()}->for_all_elements([this, &index](){
    Dout(dc::notice, "index = " << index);
    auto node_tracker = id_to_node_map_[index].lock();
    ++index;
    if (node_tracker)
      node_tracker->tracked_wat()->initialize_item();
  });
}

//static
IndexedContainerMemoryRegionOwner::indexed_container_sets_t IndexedContainerMemoryRegionOwner::indexed_container_sets_;

} // namespace cppgraphviz
