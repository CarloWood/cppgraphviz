#pragma once

#include "LabelNode.h"
#include "MemoryRegionOwner.h"
#include "get_index_label.h"
#include "IndexedContainerSet.h"
#include "dot/TableNode.h"
#include "utils/Vector.h"
#include "utils/has_print_on.h"
#include <memory_resource>
#include "debug.h"

#ifdef CWDEBUG
#include "cwds/debug_ostream_operators.h"
#endif

namespace cppgraphviz {
using utils::has_print_on::operator<<;

class VectorMemoryRegionOwner;

class TrackingAllocator : public std::pmr::memory_resource
{
 private:
  VectorMemoryRegionOwner* owner_;

 public:
  TrackingAllocator(VectorMemoryRegionOwner* owner) : owner_(owner) { }

 protected:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override;
  void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override;

  bool do_is_equal(std::pmr::memory_resource const& other) const noexcept override
  {
    return this == &other;
  }
};

class VectorMemoryRegionOwner : public MemoryRegionOwner, public LabelNode
{
 protected:
  TrackingAllocator allocater_;
  dot::TableNodePtr table_node_ptr_;
  std::vector<std::weak_ptr<NodeTracker>> id_to_node_map_; // A map of vector index to the tracker of the associated Node.
  using index_container_sets_container_type = std::map<uint64_t, IndexedContainerSet>;
  using index_container_sets_t = threadsafe::Unlocked<index_container_sets_container_type, threadsafe::policy::Primitive<std::mutex>>;
  static index_container_sets_t index_container_sets_;

 public:
  VectorMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what);

  void register_new_memory_region(MemoryRegion memory_region);
  void unregister_memory_region(MemoryRegion memory_region);

 private:
  void initialize(std::type_info const& index_type_info, std::string const& demangled_index_type_name);

 private:
  void on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr) override;

  virtual size_t number_of_elements() const = 0;
};

template <typename T, typename _Index = utils::VectorIndex<T>>
class Vector : public VectorMemoryRegionOwner, public utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>
{
 public:
  using _Base = utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>;

  Vector(std::weak_ptr<GraphTracker> const& root_graph, std::initializer_list<T> ilist, std::string_view what) :
    VectorMemoryRegionOwner(root_graph, typeid(_Index), get_index_label<_Index>(), what), _Base(ilist, &allocater_)
  {
  }

 private:
  size_t number_of_elements() const override
  {
    return _Base::size();
  }

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override
  {
    os << *static_cast<_Base const*>(this);
  }
#endif
};

} // namespace cppgraphviz
