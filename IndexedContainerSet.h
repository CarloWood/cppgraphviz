#pragma once

#include "dot/Graph.h"
#include "dot/TableNode.h"

namespace cppgraphviz {

class IndexedContainerSet;

namespace detail {

class RankdirGraphData : public dot::GraphItem
{
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
    outer_subgraph_->add_attribute({"what", std::string{what} + ".outer_subgraph_"});
    inner_subgraph_->add_attribute({"what", std::string{what} + ".inner_subgraph_"});
    outer_subgraph_->add_attribute({"cluster", "true"});
    outer_subgraph_->add_attribute({"style", "rounded"});
    outer_subgraph_->add_attribute({"color", "lightblue"});
    outer_subgraph_->add(inner_subgraph_);
    inner_subgraph_->add_attribute({"cluster", "false"});
    // We have to assume the default rankdir=TB and will adjust if set_rankdir is called with LR or RL.
    inner_subgraph_->add_attribute({"rank", "same"});
    outer_subgraph_->set_owner(this);
  }

  IndexedContainerSet(std::string const& label, std::string_view what) : IndexedContainerSet(what)
  {
    set_label(label);
  }

  void set_label(std::string const& label)
  {
    outer_subgraph_->add_attribute({"label", label});
  }

  void add_container(dot::TableNodePtr const& container)
  {
    inner_subgraph_->add(container);
  }

  void add_container(dot::TableNodePtr&& container)
  {
    inner_subgraph_->add(std::move(container));
  }

  void add_to_graph(dot::GraphItem& graph_item);
  void remove_from_graph(dot::GraphItem& graph_item);

  void rankdir_changed(dot::RankDir new_rankdir)
  {
    dot::RankDir old_rankdir = outer_subgraph_->get_rankdir();
    bool old_is_vertical = old_rankdir == dot::TB || old_rankdir == dot::BT;
    bool new_is_vertical = new_rankdir == dot::TB || new_rankdir == dot::BT;
    if (old_is_vertical != new_is_vertical)
    {
      if (new_is_vertical)
        inner_subgraph_->add_attribute({"rank", "same"});
      else
        inner_subgraph_->attribute_list().remove("rank");
    }
  }
};

} // namespace cppgraphviz
