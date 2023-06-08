#pragma once

#include "dot/Node.h"
#include "MemoryRegion.h"
#include "utils/ObjectTracker.h"
#ifdef CWDEBUG
#include "utils/has_print_on.h"
#endif

namespace cppgraphviz {
#ifdef CWDEBUG
using utils::has_print_on::operator<<;
#endif

class MemoryRegionOwner;
using MemoryRegionOwnerTracker = utils::ObjectTracker<MemoryRegionOwner>;

class MemoryRegionOwner : public utils::TrackedObject<MemoryRegionOwnerTracker>
{
 protected:
  MemoryRegion registered_memory_region_;

 protected:
  MemoryRegionOwner() = default;
  MemoryRegionOwner(MemoryRegion memory_region);
  MemoryRegionOwner(MemoryRegionOwner&& orig, MemoryRegion memory_region);
  ~MemoryRegionOwner();

 public:
  virtual void on_memory_region_usage(MemoryRegion const& used, dot::NodePtr* node_ptr_ptr) = 0;

#ifdef CWDEBUG
 public:
  virtual void print_on(std::ostream& os) const
  {
    os << "{MemoryRegionOwner@" << this << '}';
  }
#endif
};

} // namespace cppgraphviz
