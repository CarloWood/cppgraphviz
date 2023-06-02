#pragma once

#include <cstdint>

namespace cppgraphviz::dot {

using item_type_type = uint32_t;

static constexpr item_type_type main_item_type_unit = 0x10000000;
static constexpr item_type_type main_item_type_mask = 0x30000000;
static constexpr item_type_type item_type_range_mask = main_item_type_unit - 1;
static constexpr item_type_type item_type_half_range_mask = item_type_range_mask >> 1;
#define MAIN_ITEM_TYPE(n) (n * main_item_type_unit | item_type_half_range_mask)

static constexpr item_type_type item_type_node       = MAIN_ITEM_TYPE(0);
static constexpr item_type_type item_type_table_node = MAIN_ITEM_TYPE(1);
static constexpr item_type_type item_type_graph      = MAIN_ITEM_TYPE(2);
static constexpr item_type_type item_type_edge       = MAIN_ITEM_TYPE(3);

inline bool is_node(item_type_type it) { return (it & main_item_type_mask) == (item_type_node & main_item_type_mask); }
inline bool is_table_node(item_type_type it) { return (it & main_item_type_mask) == (item_type_table_node & main_item_type_mask); }
inline bool is_graph(item_type_type it) { return (it & main_item_type_mask) == (item_type_graph & main_item_type_mask); }
inline bool is_edge(item_type_type it) { return (it & main_item_type_mask) == (item_type_edge & main_item_type_mask); }

} // namespace cppgraphviz::dot
