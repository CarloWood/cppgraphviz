#pragma once

#include "Attribute.hpp"
#include <iosfwd>
#include <set>

namespace cppgraphviz::dot {

class AttributeList
{
 private:
  std::set<Attribute> attributes_;

 public:
  void add(Attribute&& attribute)
  {
    attributes_.insert(std::move(attribute));
  }

  void remove(Attribute const& key)
  {
    attributes_.erase(key);
  }

  void remove(std::string_view key)
  {
    attributes_.erase(key);
  }

  bool has_key(std::string_view key) const;
  std::string_view get_value(std::string_view key) const;

  std::string_view get(std::string_view key, std::string_view default_value) const
  {
    return has_key(key) ? get_value(key) : default_value;
  }

  operator bool() const { return !attributes_.empty(); }

  //---------------------------------------------------------------------------
  // Allow adding attributes in bulk.

  // Either with:
  //
  // attribute_list += { "key", "value" };
  AttributeList& operator+=(std::pair<std::string_view, std::string_view> attribute)
  {
    add(Attribute{attribute.first, attribute.second});
    return *this;
  }

  // Or with:
  //
  // attribute_list += { { "key", "value" }, { "key", "value" }, ... };
  AttributeList& operator+=(std::initializer_list<Attribute> list)
  {
    for (Attribute const& attribute : list)
      attributes_.insert(attribute);
    return *this;
  }

  //---------------------------------------------------------------------------
  void print_on(std::ostream& os) const;
};

inline std::ostream& operator<<(std::ostream& os, AttributeList const& attribute_list)
{
  attribute_list.print_on(os);
  return os;
}

} // namespace cppgraphviz::dot
