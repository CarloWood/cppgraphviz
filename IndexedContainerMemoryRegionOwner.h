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
 public:
  using get_begin_type = char const* (*)(IndexedContainerMemoryRegionOwner const*);
  using get_number_of_elements_type = size_t (*)(IndexedContainerMemoryRegionOwner const*);

 private:
  char* const begin_;                   // Pointer to the first element.
  size_t const element_size_;           // Element stride.
  size_t const number_of_elements_;     // The size() of the array.
  dot::TableNodePtr table_node_ptr_;
  std::vector<std::weak_ptr<NodeTracker>> id_to_node_map_; // Map index to the tracker of the associated Node.
  using indexed_container_sets_container_type = std::map<uint64_t, IndexedContainerSet>;
  using indexed_container_sets_t = threadsafe::Unlocked<indexed_container_sets_container_type, threadsafe::policy::Primitive<std::mutex>>;
  static indexed_container_sets_t indexed_container_sets_;

 protected:
  // We can't use virtual functions because these are already needed while we're still constructing Vector.
  get_begin_type get_begin_;
  get_number_of_elements_type get_number_of_elements_;

 protected:
  // Used by Array.
  IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
      char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  IndexedContainerMemoryRegionOwner(char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
      char* begin, std::type_info const& index_type_info, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> other,
      char* begin, std::string_view what);

  // Used by Vector.
  IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
      size_t element_size, size_t number_of_elements,
      get_begin_type get_begin,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
      std::type_info const& index_type_info, std::string_view what);

  IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> other,
      std::string_view what);

 public:
  void call_initialize_on_elements();

 private:
  void on_memory_region_usage(MemoryRegion const& owner_memory_region,
      MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) override;

  void initialize(std::weak_ptr<GraphTracker> const& root_graph,
      char const* label_prefix,
      std::type_info const& index_type_info,
      std::string_view demangled_index_type_name);
};

} // namespace cppgraphviz
