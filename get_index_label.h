#pragma once

#include <cxxabi.h>
#include <string>
#include <string_view>

namespace cppgraphviz {

// Specialize this to set a custom label for _Index.
template<typename _Index>
std::string get_index_label()
{
  std::string result;
  int status;
  char const* demangled_name = abi::__cxa_demangle(typeid(_Index).name(), 0, 0, &status);
  if (status != 0)
    demangled_name = "<__cxa_demangle failure>";
  result = demangled_name;
  if (status == 0)
    std::free(const_cast<char*>(demangled_name));
  return result;
}

} // namespace cppgraphviz
