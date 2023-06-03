#pragma once

#include "MemoryRegionOwner.h"
#include "Graph.h"
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

class ArrayIndexGraph : public Graph
{
 public:
  using Graph::Graph;

 protected:
  void item_attributes(dot::AttributeList& list) override
  {
    std::string prev_style;
    if (list.has_key("style"))
    {
      prev_style = std::string{list.get_value("style")} + ",";
      list.remove("style");
    }
    list += {{"cluster", "true"}, {"style", prev_style + "rounded"}, {"label", "<Index>"}, {"color", "green"}};
  }
};

template<typename _Index>
class ArrayMemoryRegionOwner : public MemoryRegionOwner
{
 private:
  char* begin_;
  size_t element_size_;
  size_t number_of_elements_;
  std::weak_ptr<GraphTracker> subgraph_tracker_;
  static std::map<uint64_t, ArrayIndexGraph> subgraphs_;

 public:
  ArrayMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph, char* begin, size_t element_size, size_t number_of_elements) :
    MemoryRegionOwner({ begin, number_of_elements * element_size }),
    begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements),
    subgraph_tracker_(get_subgraph_tracker(root_graph)) { }

 private:
  std::weak_ptr<GraphTracker> get_subgraph_tracker(std::weak_ptr<GraphTracker> const& root_graph)
  {
    std::shared_ptr<GraphTracker> root_graph_tracker = root_graph.lock();
    // Paranoia check.
    ASSERT(root_graph_tracker);
    // Create a key that is unique for a _Index / root_graph pair.
    uint64_t key = utils::pointer_hash_combine(typeid(_Index).hash_code(), root_graph_tracker.get());

    // The subgraphs do not have a memory region associated with them. Just pass something that will never match.
    auto ibp = subgraphs_.try_emplace(key, root_graph, "ArrayMemoryRegionOwner::subgraphs_");
    return ibp.first->second;
  }

  void on_memory_region_usage(MemoryRegion const& item_memory_region) override
  {
    DoutEntering(dc::notice, "on_memory_region_usage(" << item_memory_region << ") [" << this << "]");
    //FIXME
    Item* item = reinterpret_cast<Item*>(item_memory_region.begin());
    item->set_parent_graph_tracker(subgraph_tracker_);
  }
};

template<typename T, size_t N, typename _Index = utils::ArrayIndex<T>>
class Array : public ArrayMemoryRegionOwner<_Index>, public utils::Array<T, N, _Index>
{
 public:
  constexpr Array(std::weak_ptr<GraphTracker> const& root_graph, std::initializer_list<T> ilist) :
    ArrayMemoryRegionOwner<_Index>(root_graph, reinterpret_cast<char*>(this), sizeof(T), N),
    utils::Array<T, N, _Index>(ilist)
  {
  }

  constexpr Array(std::weak_ptr<GraphTracker> const& root_graph) :
    ArrayMemoryRegionOwner<_Index>(root_graph, reinterpret_cast<char*>(this), sizeof(T), N),
    utils::Array<T, N, _Index>()
  {
  }

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override
  {
    os << *static_cast<std::array<T, N> const*>(this);
  }
#endif
};

//static
template<typename _Index>
std::map<uint64_t, ArrayIndexGraph> ArrayMemoryRegionOwner<_Index>::subgraphs_;

} // namespace cppgraphviz
