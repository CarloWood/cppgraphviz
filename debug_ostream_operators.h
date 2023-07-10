#pragma once

#include "utils/is_specialization_of.h"
#include "utils/ObjectTracker.h"
#include <iostream>
#include <memory>
#include <libcwd/type_info.h>

namespace std {

template<typename Tracker>
std::ostream& operator<<(std::ostream& os, std::shared_ptr<Tracker> const& tracker)
{
  if (tracker)
  {
    if constexpr (utils::is_specialization_of_v<Tracker, utils::ObjectTracker>)
      os << tracker->tracked_object();
    else
      os << *tracker->tracked_rat();
  }
  else
    os << "<empty std::shared_ptr<" << libcwd::type_info_of<Tracker>().demangled_name() << ">>";
  return os;
}

template<typename Tracker>
std::ostream& operator<<(std::ostream& os, std::weak_ptr<Tracker> const& weak_tracker)
{
  std::shared_ptr<Tracker> tracker = weak_tracker.lock();
  if (tracker)
    os << tracker;
  else
    os << "<deleted " << libcwd::type_info_of<Tracker>().demangled_name() << ">";
  return os;
}

} // namespace std
