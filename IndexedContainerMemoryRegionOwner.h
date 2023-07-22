#pragma once

#include "IndexedContainerSet.h"
#include "MemoryRegionOwner.h"
#include "Graph.h"
#include "LabelNode.h"
#include "threadsafe/threadsafe.h"
#include <vector>
#include <map>
#include "debug.h"

namespace cppgraphviz {

class IndexedContainerMemoryRegionOwner : public MemoryRegionOwner, public LabelNode
{
 private:
  char* const begin_;                   // Pointer to the first element.
  size_t const element_size_;           // Element stride.
  size_t const number_of_elements_;     // The size() of the array.
  dot::TableNodePtr table_node_ptr_;
  std::vector<std::weak_ptr<NodeTracker>> id_to_node_map_; // A map of array index to the tracker of the associated Node.
  using indexed_container_sets_container_type = std::map<uint64_t, IndexedContainerSet>;
  using indexed_container_sets_t = threadsafe::Unlocked<indexed_container_sets_container_type, threadsafe::policy::Primitive<std::mutex>>;
  static indexed_container_sets_t indexed_container_sets_;

 public:
  IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph, char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  IndexedContainerMemoryRegionOwner(char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
      char* begin, std::type_info const& index_type_info, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> other,
      char* begin, std::string_view what);

  void call_initialize_on_elements();

 private:
  void on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) override;
};

} // namespace cppgraphviz
