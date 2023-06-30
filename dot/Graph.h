#pragma once

#include "DotID.h"
#include "Node.h"
#include "Edge.h"
#include "TableNode.h"
#include <utils/iomanip.h>
#include <string>
#include <map>
#include <iosfwd>

namespace cppgraphviz::dot {

enum RankDir {
  TB,
  LR,
  BT,
  RL
};

class GraphItem;

template<typename T>
concept ConceptIsGraphItem = std::is_base_of_v<GraphItem, T>;

template<typename T>
concept ConceptHasAddToGraph = requires(T obj)
{
  obj.add_to_graph(std::declval<typename T::item_type::graph_item_type&>());
};

template<typename T>
concept ConceptHasRemoveFromGraph = requires(T obj)
{
  obj.remove_from_graph(std::declval<typename T::item_type::graph_item_type&>());
};

// GraphItem is derived from Item for its id and (optional) attribute list.
// A graph can be directional or not, and/or strict (only allowing one edge between
// nodes (two for a digraph)).
//
// A graph contains a list of nodes, edges and (optionally) other (sub)graphs.
class GraphItem : public Item
{
 public:
  using unlocked_type = threadsafe::Unlocked<GraphItem, ItemLockingPolicy>;

 private:
  // Configuration.
  mutable std::atomic<bool> digraph_ = false;
  mutable std::atomic<RankDir> rankdir_ = TB;
  bool strict_ = false;
  bool concentrate_ = false;

  // Default node and edge attributes. The default (sub)graph attributes are stored in Item.
  AttributeList node_attribute_list_;
  AttributeList edge_attribute_list_;

  // The list of all (sub)graphs of this graph, by ID.
  std::map<DotID_type, ConstItemPtr> items_;

 private:
  void write_body_to(std::ostream& os, std::string indentation = {}) const;

 public:
  //---------------------------------------------------------------------------
  virtual void set_digraph(bool digraph = true) const;
  virtual void set_rankdir(RankDir rankdir) const;
  void set_strict(bool strict = true) { strict_ = strict; }
  void set_concentrate(bool concentrate) { concentrate_ = concentrate; }

  // Write graph to os in dot format.
  void write_dot(std::ostream& os) const;

  // Accessors.
  bool is_digraph() const { return digraph_; }
  bool is_strict() const { return strict_; }
  bool is_concentrate() const { return concentrate_; }
  RankDir get_rankdir() const { return rankdir_; }

  //---------------------------------------------------------------------------

  template<typename ACCESS_TYPE>
  void add_graph_item(typename ACCESS_TYPE::unlocked_type::crat const& item_r, Item::unlocked_type const& item)
  {
    DoutEntering(dc::notice, "dot::GraphItem::add_graph_item(" << item_r->attribute_list().get("what", "<NO WHAT>") <<
        " [" << item_r->dot_id() << "]) [" << this << " [" << attribute_list().get_value("what") << "]]");

    typename ACCESS_TYPE::unlocked_type const& unlocked = unlocked_cast<typename ACCESS_TYPE::unlocked_type const&>(item);

    auto ibp = items_.try_emplace(item_r->dot_id(), item);
    // Do not add the same graph item twice.
    ASSERT(ibp.second);
    if constexpr (std::is_base_of_v<std::remove_cvref_t<decltype(*item_r)>, GraphItem>)
    {
      if (item_r->is_graph())
      {
        GraphItem const& subgraph = static_cast<GraphItem const&>(*item_r);
        // The subgraph must be of the same type as this graph.
        subgraph.set_digraph(digraph_);
        // set_rankdir needs to be called while item is unlocked in some cases.
        item_r.unlock();
        subgraph.set_rankdir(rankdir_);
        item_r.relock(unlocked);
      }
    }
  }

  template<typename ACCESS_TYPE>
  void remove_graph_item(typename ACCESS_TYPE::unlocked_type::crat const& item_r)
  {
    DoutEntering(dc::notice, "dot::GraphItem::remove_graph_item(" << item_r->attribute_list().get_value("what") <<
        " [" << item_r->dot_id() << "]) [" << this << " [" << attribute_list().get_value("what") << "]]");
    bool erased = items_.erase(item_r->dot_id());
    // That's unexpected... we shouldn't be calling remove_graph_item unless it is there.
    ASSERT(erased);
  }

  template<typename ACCESS_TYPE>
  requires utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::AccessConst> ||
           utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::ConstAccess> ||
           utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::Access>
  void add(ItemPtr const& item_ptr, ACCESS_TYPE const& item_r)
  {
    add_graph_item<ACCESS_TYPE>(item_r, item_ptr.item());
  }

  template<typename ACCESS_TYPE>
  requires utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::AccessConst> ||
           utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::ConstAccess> ||
           utils::is_specialization_of_v<ACCESS_TYPE, threadsafe::Access>
  void remove(ACCESS_TYPE const& item_r)
  {
    remove_graph_item<ACCESS_TYPE>(item_r);
  }

  void add(ItemPtr const& item_ptr)
  {
    add(item_ptr, Item::unlocked_type::crat{item_ptr.item()});
  }

  void remove(ItemPtr const& item_ptr)
  {
    remove(Item::unlocked_type::crat{item_ptr.item()});
  }

  template<ConceptHasAddToGraph T>
  void insert(T& obj)
  {
    obj.add_to_graph(*static_cast<typename T::item_type::graph_item_type*>(this));
  }

  template<ConceptHasRemoveFromGraph T>
  void erase(T& obj)
  {
    obj.remove_from_graph(*static_cast<typename T::item_type::graph_item_type*>(this));
  }

  void add_node_attribute(Attribute&& attribute);
  void add_edge_attribute(Attribute&& attribute);

  item_type_type item_type() const override { return item_type_graph; }
  void write_dot_to(std::ostream& os, std::string& indentation) const override;
};

class GraphPtr : public ItemPtrTemplate<GraphItem>
{
 public:
  GraphPtr(bool strict = false)
  {
    unlocked_type::wat item_w{item()};
    item_w->set_strict(strict);
  }

 protected:
  GraphPtr(bool digraph, bool strict = false)
  {
    unlocked_type::wat item_w{item()};
    item_w->set_digraph(digraph);
    item_w->set_strict(strict);
  }
};

static_assert(sizeof(GraphPtr) == sizeof(ItemPtr), "GraphPtr may not have any member variables of its own!");

struct DigraphPtr : public GraphPtr
{
  DigraphPtr(bool strict = false) : GraphPtr(true, strict) { }
};

class DigraphIomanip : public utils::iomanip::Sticky
{
 private:
  static utils::iomanip::Index s_index;

 public:
  DigraphIomanip() : Sticky(s_index, 1L) { }

  static long get_iword_value(std::ostream& os) { return get_iword_from(os, s_index); }
};

extern DigraphIomanip digraph;

} // namespace cppgraphviz::dot
