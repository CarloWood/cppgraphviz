#pragma once

#include <iostream>

namespace cppgraphviz {

class MemoryRegion
{
 private:
  char* begin_;
  char const* end_;

 public:
  MemoryRegion(char* begin, size_t size) : begin_(begin), end_(begin + size) { }

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
