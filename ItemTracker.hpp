#pragma once

#include "utils/ObjectTracker.h"

namespace cppgraphviz {

// Note that Tracked should be Node or Graph, both of which are derived from Item.
template<typename Tracked>
using ItemTracker = utils::ObjectTracker<Tracked>;

} // namespace cppgraphviz
