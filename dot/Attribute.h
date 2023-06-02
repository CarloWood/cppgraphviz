#pragma once

#include <string_view>
#include <string>
#include <iosfwd>

namespace cppgraphviz::dot {

class Attribute
{
 private:
  std::string key_;
  mutable std::string value_;

 public:
  // Construct a 'key' Attribute, for searching.
  Attribute(std::string_view key) : key_(key) { }
  // Construct an Attribute with key and value.
  Attribute(std::string_view key, std::string_view value) : key_(key), value_(value) { }

  // Attribute is stored in a std::set.
  friend bool operator<(Attribute const& lhs, Attribute const& rhs)
  {
    return lhs.key_ < rhs.key_;
  }

  // Accessor.
  std::string const& value() const { return value_; }

  void print_on(std::ostream& os) const;
};

inline std::ostream& operator<<(std::ostream& os, Attribute const& attribute)
{
  attribute.print_on(os);
  return os;
}

} // namespace cppgraphviz::dot
