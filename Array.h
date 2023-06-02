#pragma once

#include "utils/Array.h"

namespace cppgraphviz {

template<typename T, std::size_t N, typename _Index = utils::ArrayIndex<T>>
using Array = utils::Array<T, N, _Index>;

} // namespace cppgraphviz
