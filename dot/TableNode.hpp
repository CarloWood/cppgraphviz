#pragma once

#include "Item.hpp"
#include "TableElement.hpp"
#include "Port.hpp"
#include <map>

namespace cppgraphviz::dot {

template <typename T>
concept ConceptIndexableContainer = requires(T t)
{
  { t.ibegin() } -> std::same_as<typename T::index_type>;
  { t.iend() } -> std::same_as<typename T::index_type>;
  { std::declval<T>()[std::declval<typename T::index_type>()] } -> std::same_as<typename T::reference>;
  { std::declval<T const>()[std::declval<typename T::index_type>()] } -> std::same_as<typename T::const_reference>;
};

template <typename T>
concept ConceptSizeTIndexableContainer = requires(T t)
{
  t.size();
  { std::declval<T>()[std::declval<size_t>()] } -> std::same_as<typename T::reference>;
  { std::declval<T const>()[std::declval<size_t>()] } -> std::same_as<typename T::const_reference>;
};

class TableNodeItem : public Item
{
 private:
  std::vector<TableElement> copied_elements_;
  std::function<size_t()> container_size_;
  std::function<TableElement(size_t)> container_reference_;

 public:
  // The link_container member functions store a reference to `container`,
  // which may therefore not be moved anymore after this call.

  template<ConceptIndexableContainer Container>
  void link_container(Container& container)
  {
    container_size_ = [&]() -> size_t { return container.size(); };
    container_reference_ = [&](size_t index) -> TableElement {
      return static_cast<dot::NodePtr&>(container[static_cast<typename Container::index_type>(index)]); };
  }

  template<ConceptSizeTIndexableContainer Container>
  void link_container(Container& container)
  {
    container_size_ = [&]() -> size_t { return container.size(); };
    container_reference_ = [&](size_t index) -> TableElement { return container[index]; };
  }

  void copy_elements(std::function<NodePtr (size_t)> at, size_t size)
  {
    link_container(copied_elements_);
    for (size_t i = 0; i < size; ++i)
      copied_elements_.push_back(at(i));
  }

  template<ConceptIndexableContainer Container>
  void copy_elements(Container const& container)
  {
    copy_elements([&](size_t i){ return container[static_cast<typename Container::index_type>(i)]; }, container.size());
  }

  void write_html_to(std::ostream& os, std::string const& indentation) const;

  Port at(size_t index) const
  {
    return {dot_id(), index};
  }

  void for_all_elements(std::function<void(NodeItem&)> callback)
  {
    for (size_t i = 0; i < container_size_(); ++i)
      callback(container_reference_(i).node_ptr().item());
  }

  item_type_type item_type() const override { return item_type_table_node; }
  void write_dot_to(std::ostream& os, std::string& indentation) const override;
};

template<typename T>
concept ConceptIsTableNodeItem = std::is_base_of_v<TableNodeItem, T>;

struct TableNodePtr : public ItemPtrTemplate<TableNodeItem>
{
  Port operator[](size_t index) const { return item().at(index); }
};

} // namespace cppgraphviz::dot
