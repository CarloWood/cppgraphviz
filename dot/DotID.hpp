#pragma once

#include <utils/UniqueID.h>     // Add https://github.com/CarloWood/ai-utils.git to the root of your project.
#include <cstdint>

namespace cppgraphviz::dot {

using ID_type = uint32_t;
using DotID_type = utils::UniqueID<ID_type>;
extern utils::UniqueIDContext<ID_type> s_unique_id_context;

} // namespace cppgraphviz::dot
