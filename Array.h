#pragma once

#include "get_index_label.h"
#include "IndexedContainerSet.h"
#include "MemoryRegionOwner.h"
#include "Graph.h"
#include "LabelNode.h"
#include "utils/Array.h"
#include "utils/has_print_on.h"
#include "utils/pointer_hash.h"
#include <map>
#include "debug.h"

#ifdef CWDEBUG
#include "cwds/debug_ostream_operators.h"
#endif

namespace cppgraphviz {
using utils::has_print_on::operator<<;

class ArrayMemoryRegionOwner : public MemoryRegionOwner, public LabelNode
{
 private:
  char* begin_;                         // Pointer to the first element.
  size_t element_size_;                 // Element stride.
  size_t number_of_elements_;           // The size() of the array.
  dot::TableNodePtr table_node_ptr_;
  std::vector<std::weak_ptr<NodeTracker>> id_to_node_map_; // A map of array index to the tracker of the associated Node.
  static std::map<uint64_t, IndexedContainerSet> index_container_sets_;

 public:
  ArrayMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph, char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  ArrayMemoryRegionOwner(char* begin, size_t element_size, size_t number_of_elements,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  ArrayMemoryRegionOwner(ArrayMemoryRegionOwner const& other,
      char* begin, std::type_info const& index_type_info, std::string_view what);

  ArrayMemoryRegionOwner(ArrayMemoryRegionOwner&& other, char* begin, std::string_view what);

  void call_initialize_on_elements();

 private:
  void on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) override;
};

template<typename T, size_t N, typename _Index = utils::ArrayIndex<T>>
class Array : public ArrayMemoryRegionOwner, public utils::Array<T, N, _Index>
{
 public:
  constexpr Array(std::weak_ptr<GraphTracker> const& root_graph, std::initializer_list<T> ilist, std::string_view what) :
    ArrayMemoryRegionOwner(root_graph, reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        sizeof(T), N, typeid(_Index), get_index_label<_Index>(), what),
    utils::Array<T, N, _Index>(ilist)
  {
  }

  Array(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
    ArrayMemoryRegionOwner(root_graph,
        reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        sizeof(T), N, typeid(_Index), get_index_label<_Index>(), what),
    utils::Array<T, N, _Index>()
  {
  }

  Array(Array const& other, std::string_view what) :
    ArrayMemoryRegionOwner(other, reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)), typeid(_Index), what),
    utils::Array<T, N, _Index>(other)
  {
  }

  Array(Array&& other, std::string_view what) :
    ArrayMemoryRegionOwner(std::move(other), reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)), what),
    utils::Array<T, N, _Index>(std::move(other))
  {
  }

#ifdef CPPGRAPHVIZ_USE_WHAT
  Array(Array const& other) :
    ArrayMemoryRegionOwner(other,
        reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        typeid(_Index), "Array(Array const&) of " + other.get_what()),
    utils::Array<T, N, _Index>(other)
  {
  }

  Array(Array&& other) :
    ArrayMemoryRegionOwner(std::move(other),
        reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        "Array(Array&&) of " + other.get_what()),
    utils::Array<T, N, _Index>(std::move(other))
  {
  }
#endif

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override
  {
    os << *static_cast<std::array<T, N> const*>(this);
  }
#endif
};

} // namespace cppgraphviz
