#pragma once

#include "dot/Graph.h"
#include "dot/TableNode.h"

namespace cppgraphviz {

class IndexedContainerSet;

namespace detail {

class RankdirGraphData : public dot::GraphItem
{
 public:
  using unlocked_type = threadsafe::Unlocked<RankdirGraphData, dot::ItemLockingPolicy>;

 private:
  IndexedContainerSet* owner_;

  void set_rankdir(dot::RankDir rankdir) const final;

 public:
  void set_owner(IndexedContainerSet* owner)
  {
    owner_ = owner;
  }
};

using RankdirGraph = dot::ItemPtrTemplate<RankdirGraphData>;

} // namespace detail

// A cluster graph containing one or more indexed containers.
class IndexedContainerSet
{
 public:
  using item_type = dot::GraphItem;

 private:
  detail::RankdirGraph outer_subgraph_;         // This subgraph wraps the inner subgraph.
  dot::GraphPtr inner_subgraph_;                // This subgraph references the dot::TableNodeItem's that represent the indexed containers.

 public:
  IndexedContainerSet(std::string_view what)
  {
    detail::RankdirGraph::unlocked_type::wat outer_subgraph_w{outer_subgraph_.item()};
    dot::GraphPtr::unlocked_type::wat inner_subgraph_w{inner_subgraph_.item()};
    outer_subgraph_w->add_attribute({"what", std::string{what} + ".outer_subgraph_"});
    inner_subgraph_w->add_attribute({"what", std::string{what} + ".inner_subgraph_"});
    outer_subgraph_w->add_attribute({"cluster", "true"});
    outer_subgraph_w->add_attribute({"style", "rounded"});
    outer_subgraph_w->add_attribute({"color", "lightblue"});
    outer_subgraph_w->add(inner_subgraph_, inner_subgraph_w);
    inner_subgraph_w->add_attribute({"cluster", "false"});
    // We have to assume the default rankdir=TB and will adjust if set_rankdir is called with LR or RL.
    inner_subgraph_w->add_attribute({"rank", "same"});
    outer_subgraph_w->set_owner(this);
  }

  IndexedContainerSet(std::string const& label, std::string_view what) : IndexedContainerSet(what)
  {
    set_label(label);
  }

  void set_label(std::string const& label)
  {
    detail::RankdirGraph::unlocked_type::wat outer_subgraph_w{outer_subgraph_.item()};
    outer_subgraph_w->add_attribute({"label", label});
  }

  void add_container(dot::TableNodePtr const& container, dot::TableNodePtr::unlocked_type::crat const& container_r)
  {
    dot::GraphPtr::unlocked_type::wat inner_subgraph_w{inner_subgraph_.item()};
    inner_subgraph_w->add(container, container_r);
  }

  void add_container(dot::TableNodePtr const& container)
  {
    dot::GraphPtr::unlocked_type::wat inner_subgraph_w{inner_subgraph_.item()};
    inner_subgraph_w->add(container);
  }

  void add_to_graph(dot::GraphItem& graph_item);
  void remove_from_graph(dot::GraphItem& graph_item);

  void rankdir_changed(dot::RankDir new_rankdir)
  {
    detail::RankdirGraph::unlocked_type::crat outer_subgraph_r{outer_subgraph_.item()};
    dot::RankDir old_rankdir = outer_subgraph_r->get_rankdir();
    bool old_is_vertical = old_rankdir == dot::TB || old_rankdir == dot::BT;
    bool new_is_vertical = new_rankdir == dot::TB || new_rankdir == dot::BT;
    if (old_is_vertical != new_is_vertical)
    {
      dot::GraphPtr::unlocked_type::wat inner_subgraph_w{inner_subgraph_.item()};
      if (new_is_vertical)
        inner_subgraph_w->add_attribute({"rank", "same"});
      else
        inner_subgraph_w->attribute_list().remove("rank");
    }
  }
};

} // namespace cppgraphviz
