#pragma once

#include "get_index_label.h"
#include "IndexedContainerMemoryRegionOwner.h"
#include "utils/Array.h"
#include "utils/has_print_on.h"
#include "debug.h"

namespace cppgraphviz {
using utils::has_print_on::operator<<;

template<typename T, size_t N, typename _Index = utils::ArrayIndex<T>>
class Array : public IndexedContainerMemoryRegionOwner, public utils::Array<T, N, _Index>
{
 public:
  constexpr Array(std::weak_ptr<GraphTracker> const& root_graph, std::initializer_list<T> ilist, std::string_view what) :
    IndexedContainerMemoryRegionOwner(root_graph, reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        sizeof(T), N, typeid(_Index), get_index_label<_Index>(), what),
    utils::Array<T, N, _Index>(ilist)
  {
  }

  Array(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
    IndexedContainerMemoryRegionOwner(root_graph,
        reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        sizeof(T), N, typeid(_Index), get_index_label<_Index>(), what),
    utils::Array<T, N, _Index>()
  {
  }

  Array(Array const& other, std::string_view what) :
    IndexedContainerMemoryRegionOwner(other, reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)), typeid(_Index), what),
    utils::Array<T, N, _Index>(other)
  {
  }

  Array(Array&& other, std::string_view what) :
    IndexedContainerMemoryRegionOwner(std::move(other), reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)), what),
    utils::Array<T, N, _Index>(std::move(other))
  {
  }

#ifdef CPPGRAPHVIZ_USE_WHAT
  Array(Array const& other) :
    IndexedContainerMemoryRegionOwner(other,
        reinterpret_cast<char*>(static_cast<std::array<T, N>*>(this)),
        typeid(_Index), "Array(Array const&) of " + other.get_what()),
    utils::Array<T, N, _Index>(other)
  {
  }

  Array(Array&& other) :
    IndexedContainerMemoryRegionOwner(std::move(other),
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
