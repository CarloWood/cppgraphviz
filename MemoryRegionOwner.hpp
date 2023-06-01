#pragma once

#include "MemoryRegion.hpp"
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
 public:
  virtual void on_memory_region_usage(MemoryRegion const& used) = 0;

#ifdef CWDEBUG
 public:
  virtual void print_on(std::ostream& os) const = 0;
#endif
};

} // namespace cppgraphviz
