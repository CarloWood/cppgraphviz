#include "sys.h"
#include "Attribute.hpp"
#include <cctype>
#include <algorithm>
#include <string_view>
#include <iostream>

namespace cppgraphviz::dot {

// Returns true if this string can be used as ID without quotes.
bool is_valid_ID(std::string const& s)
{
  if (s.empty())
    return false;

  // Check if the first character is a digit.
  if (std::isdigit(s[0]))
    // Only return true when all characters are digits.
    return std::all_of(s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); });

  // Check if all characters are either alphabetic, digits, or underscores.
  for (char const c : s)
    if (!std::isalnum(c) && c != '_')
      return false;

  return true;
}

bool has_non_escaped_chars(std::string_view sv)
{
  bool saw_backslash = false;
  for (char c : sv)
  {
    if (c == '"' && !saw_backslash)
      return false;
    // sb   c==\\   new cb
    // 0    0       0
    // 0    1       1
    // 1    0       0
    // 1    1       0
    saw_backslash = c == '\\' && !saw_backslash;
  }
  return saw_backslash;
}

// Allow the user to add quotes around a string themselves.
std::string quoted(std::string const& s, bool no_quotes_required = false)
{
  bool has_quotes = s.size() >= 2 && s[0] == '"' && s[s.size() - 1] == '"';

  if (has_quotes)
  {
    std::string_view sv = s;
    sv.remove_prefix(1);
    sv.remove_suffix(1);
    if (!has_non_escaped_chars(sv))
      return s;                         // Return as-is: is already quoted and does not contain internal quotes. For example: '"hello"' or '"hel\"lo\\"'.
  }
  // If we get here the string either has no quotes are both start and end, or contains an unescaped
  // quote in the middle and/or an unescaped backslash at the end (which then escapes the trailing quote).
  // For example:
  // 'hello'
  // '"hello'
  // 'hello"'
  // '"hel"lo"'
  // '"hello\"'
  // In all of these cases, except the first, no_quotes_required will be false.

  if (no_quotes_required)
    return s;                           // Return as-is: the string only contains alpha-numeric characters and underscores, and no quotes are required.

  // Add quotes and escape any existing quotes and/or backslashes.
  std::string result;
  result = '"';
  for (char const c : s)
  {
    if (c == '"' || c == '\\')
      result += '\\';
    result += c;
  }
  result += '"';
  return result;
}

void Attribute::print_on(std::ostream& os) const
{
  bool no_quotes_required = is_valid_ID(key_);

  // ID '=' ID
  os << quoted(key_, no_quotes_required) << '=' << quoted(value_);
}

} // namespace cppgraphviz::dot
