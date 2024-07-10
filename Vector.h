#pragma once

#include "LabelNode.h"
#include "get_index_label.h"
#include "IndexedContainerSet.h"
#include "IndexedContainerMemoryRegionOwner.h"
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

class TrackingAllocator : public std::pmr::memory_resource
{
 private:
  IndexedContainerMemoryRegionOwner* owner_;

 public:
  TrackingAllocator(IndexedContainerMemoryRegionOwner* owner) : owner_(owner) { }

 protected:
  void* do_allocate(std::size_t bytes, std::size_t alignment) override;
  void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override;

  bool do_is_equal(std::pmr::memory_resource const& other) const noexcept override
  {
    return this == &other;
  }
};

class VectorMemoryRegionOwner : public IndexedContainerMemoryRegionOwner
{
 public:
  using get_begin_type = IndexedContainerMemoryRegionOwner::get_begin_type;
  using get_number_of_elements_type = IndexedContainerMemoryRegionOwner::get_number_of_elements_type;

 protected:
  TrackingAllocator allocator_;

 protected:
  VectorMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph, size_t element_size, size_t number_of_elements,
      get_begin_type get_begin,
      std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what) :
    IndexedContainerMemoryRegionOwner(root_graph, element_size, number_of_elements, get_begin,
        index_type_info, demangled_index_type_name, what), allocator_(this) { }

  VectorMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
      std::type_info const& index_type_info, std::string_view what) :
    IndexedContainerMemoryRegionOwner(other, index_type_info, what), allocator_(this) { }

  VectorMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> other,
      std::string_view what) :
    IndexedContainerMemoryRegionOwner(other, what), allocator_(this) { }
};

template <typename T, typename _Index = utils::VectorIndex<T>>
class Vector : public VectorMemoryRegionOwner, public utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>
{
 public:
  using _Base = utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>;

  static char const* get_begin(IndexedContainerMemoryRegionOwner const* self)
  {
    DoutEntering(dc::notice, "cppgraphviz::Vector<" <<
        NAMESPACE_DEBUG::type_name_of<T>() << ", " << NAMESPACE_DEBUG::type_name_of<_Index>() << ">::get_begin(" << (void*)self << ")");
    Vector const* this_ = static_cast<Vector const*>(self);
    char const* data = reinterpret_cast<char const*>(this_->utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>::data());
    Dout(dc::notice, "data = " << (void*)data << " [" << this_ << "]");
    return data;
  }

  static size_t get_number_of_elements(IndexedContainerMemoryRegionOwner const* self)
  {
    DoutEntering(dc::notice, "cppgraphviz::Vector<" <<
        NAMESPACE_DEBUG::type_name_of<T>() << ", " << NAMESPACE_DEBUG::type_name_of<_Index>() << ">::get_number_of_elements(" <<
        (void*)self << ")");
    Vector const* this_ = static_cast<Vector const*>(self);
    size_t size = this_->utils::Vector<T, _Index, std::pmr::polymorphic_allocator<T>>::size();
    Dout(dc::notice, "size = " << size << " [" << this_ << "]");
    return size;
  }

  Vector(std::weak_ptr<GraphTracker> const& root_graph, std::initializer_list<T> ilist, std::string_view what) :
    VectorMemoryRegionOwner(root_graph, sizeof(T), ilist.size(),
        &Vector::get_begin,
        typeid(_Index), get_index_label<_Index>(), what),
    _Base(ilist, &allocator_)
  {
    // Now that the utils::Vector is initialized set this function pointer, so that
    // future calls to IndexedContainerMemoryRegionOwner::on_memory_region_usage will
    // get the correct value instead of the initial size.
    get_number_of_elements_ = &Vector::get_number_of_elements;
  }

  Vector(Vector const& other, std::string_view what) :
    VectorMemoryRegionOwner(other, typeid(_Index), what),
    _Base(other, &allocator_)
  {
    get_number_of_elements_ = &Vector::get_number_of_elements;
  }

#ifdef CPPGRAPHVIZ_USE_WHAT
  Vector(Vector const& other) :
    VectorMemoryRegionOwner(other, typeid(_Index), "Vector(Vector const&) of " + other.get_what()),
    _Base(other, &allocator_)
  {
    get_number_of_elements_ = &Vector::get_number_of_elements;
  }
#endif

  Vector(Vector&& other, std::string_view what) :
    VectorMemoryRegionOwner(std::move(other), what),
    _Base(std::move(other), &allocator_)
  {
    get_number_of_elements_ = &Vector::get_number_of_elements;
  }

#ifdef CPPGRAPHVIZ_USE_WHAT
  Vector(Vector&& other) :
    VectorMemoryRegionOwner(std::move(other), "Vector(Vector&&) of " + other.get_what()),
    _Base(std::move(other), &allocator_)
  {
    get_number_of_elements_ = &Vector::get_number_of_elements;
  }
#endif

#ifdef CWDEBUG
 public:
  void print_on(std::ostream& os) const override
  {
    os << *static_cast<_Base const*>(this);
  }
#endif
};

} // namespace cppgraphviz
