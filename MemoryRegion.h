#pragma once

#include "utils/Badge.h"
#include <iostream>

namespace cppgraphviz {

class MemoryRegionOwner;

class MemoryRegion
{
 private:
  char* begin_;
  char const* end_;

 public:
  constexpr MemoryRegion() : begin_(nullptr), end_(nullptr) { }
  constexpr MemoryRegion(char* begin, size_t size) : begin_(begin), end_(begin + size) { }

  // Disallow assignment because that wouldn't be thread-safe without demanding
  // external synchronization of all MemoryRegion objects. This way we know we don't need that.
  MemoryRegion& operator=(MemoryRegion const&) = delete;

  // However, if you are sure that this external synchronization is there, then one can use this function.
  void critical_area_assign(MemoryRegion const& other)
  {
    begin_ = other.begin_;
    end_ = other.end_;
  }

  // However, when a MemoryRegionOwner is moved, its registered_memory_region_ is reset.
  // This is ok because being moved implies external synchronization.
  void reset(utils::Badge<MemoryRegionOwner>)
  {
    begin_ = nullptr;
    end_ = nullptr;
  }

  bool operator==(MemoryRegion const& other) const
  {
    return begin_ == other.begin_ && end_ == other.end_;
  }

  bool operator<(MemoryRegion const& other) const
  {
    return end_ <= other.begin_;
  }

  bool overlaps_with(MemoryRegion const& other) const
  {
    // The regions overlap when neither is less than the other.
    return !(end_ <= other.begin_) && !(other.end_ <= begin_);
  }

  bool lays_within(MemoryRegion const& other) const
  {
    return begin_ >= other.begin_ && end_ <= other.end_;
  }

  friend std::ostream& operator<<(std::ostream& os, MemoryRegion const& memory_region)
  {
    os << "[" << (void*)memory_region.begin_ << ", " << (void*)memory_region.end_ << ">";
    return os;
  }

  // Accessor.
  char* begin() const { return begin_; }
};

} // namespace cppgraphviz
