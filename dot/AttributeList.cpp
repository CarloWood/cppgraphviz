#include "sys.h"
#include "AttributeList.hpp"
#include <iostream>
#include "debug.h"

namespace cppgraphviz::dot {

bool AttributeList::has_key(std::string_view key) const
{
  auto iter = attributes_.find(Attribute{key});
  return iter != attributes_.end();
}

std::string_view AttributeList::get_value(std::string_view key) const
{
  auto iter = attributes_.find(Attribute{key});
  // Only call get_value if has_key returns true.
  ASSERT(iter != attributes_.end());
  return iter->value();
}

void AttributeList::print_on(std::ostream& os) const
{
  char const* prefix = "";
  for (Attribute const& attribute : attributes_)
  {
    os << prefix << attribute;
    prefix = ", ";
  }
}

} // namespace cppgraphviz::dot
