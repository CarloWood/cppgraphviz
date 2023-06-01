#pragma once

#include "DotID.hpp"
#include "Node.hpp"
#include "Edge.hpp"
#include "TableNode.hpp"
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
 private:
  // Configuration.
  mutable bool digraph_ = false;
  mutable RankDir rankdir_ = TB;
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
  void add_graph_item(Item const* item);
  void remove_graph_item(Item const* item);

  void add(ItemPtr const& item_ptr)
  {
    DoutEntering(dc::notice, "dot::GraphItem::add(" << item_ptr.item().attribute_list().get_value("what") << " [" << item_ptr.item().dot_id() << "]) [" << this << " [" << attribute_list().get_value("what") << "]]");
    add_graph_item(&item_ptr.item());
  }

  void remove(ItemPtr const& item_ptr)
  {
    DoutEntering(dc::notice, "dot::GraphItem::remove(" << item_ptr.item().attribute_list().get_value("what") << " [" << item_ptr.item().dot_id() << "]) [" << this << " [" << attribute_list().get_value("what") << "]]");
    remove_graph_item(&item_ptr.item());
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
    item().set_strict(strict);
  }

 protected:
  GraphPtr(bool digraph, bool strict = false)
  {
    item().set_digraph(digraph);
    item().set_strict(strict);
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
