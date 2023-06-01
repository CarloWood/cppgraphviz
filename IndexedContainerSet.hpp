#pragma once

#include "dot/Graph.hpp"
#include "dot/TableNode.hpp"

namespace cppgraphviz {

template<typename Index>
class IndexedContainerSet;

namespace detail {

template<typename Index>
class RankdirGraphData : public dot::GraphItem
{
 private:
  IndexedContainerSet<Index>* owner_;

  void set_rankdir(dot::RankDir rankdir) const final;

 public:
  void set_owner(IndexedContainerSet<Index>* owner)
  {
    owner_ = owner;
  }
};

template<typename Index>
using RankdirGraph = dot::ItemPtrTemplate<RankdirGraphData<Index>>;

} // namespace detail

// A cluster graph containing one or more indexed containers that use Index as index type.
template<typename Index>
class IndexedContainerSet
{
 public:
  using item_type = dot::GraphItem;

 private:
  detail::RankdirGraph<Index> outer_subgraph_;  // This subgraph wraps the inner subgraph.
  dot::GraphPtr inner_subgraph_;                // This subgraph references the dot::TableNode's that represent the indexed containers.

 private:
  void initialize()
  {
    outer_subgraph_->add_attribute({"cluster", "true"});
    outer_subgraph_->add_attribute({"style", "rounded"});
    outer_subgraph_->add_attribute({"color", "lightblue"});
    outer_subgraph_->add(inner_subgraph_);
    inner_subgraph_->add_attribute({"cluster", "false"});
    // We have to assume the default rankdir=TB and will adjust if set_rankdir is called with LR or RL.
    inner_subgraph_->add_attribute({"rank", "same"});
    outer_subgraph_->set_owner(this);
  }

 public:
  IndexedContainerSet(char const* what)
  {
    outer_subgraph_->add_attribute({"what", std::string{what} + ".outer_subgraph_"});
    inner_subgraph_->add_attribute({"what", std::string{what} + ".inner_subgraph_"});
    initialize();
  }

  IndexedContainerSet(std::string const& label, char const* what)
  {
    outer_subgraph_->add_attribute({"what", std::string{what} + ".outer_subgraph_"});
    inner_subgraph_->add_attribute({"what", std::string{what} + ".inner_subgraph_"});
    initialize();
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

template<typename Index>
void IndexedContainerSet<Index>::add_to_graph(dot::GraphItem& graph_item)
{
  graph_item.add(outer_subgraph_);
}

template<typename Index>
void IndexedContainerSet<Index>::remove_from_graph(dot::GraphItem& graph_item)
{
  graph_item.remove(outer_subgraph_);
}

namespace detail {

template<typename Index>
void RankdirGraphData<Index>::set_rankdir(dot::RankDir rankdir) const
{
  owner_->rankdir_changed(rankdir);
  dot::GraphItem::set_rankdir(rankdir);
}

} // namespace detail

} // namespace cppgraphviz
