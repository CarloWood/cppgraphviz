#pragma once

#include "AttributeList.hpp"
#include "Item.hpp"
#include <utils/Badge.h>
#include <boost/intrusive_ptr.hpp>
#include <concepts>

namespace cppgraphviz::dot {

// A wrapper around a boost::intrusive_ptr<Item>.
//
// ItemPtr's can be cheaply moved and even copied: changes made
// to one ItemPtr affect all other ItemPtr that are/were copies.
//
// The (shared) data that a ItemPtr points to (Item) will be
// a base class of either GraphItem, NodeItem or EdgeItem.
class ConstItemPtr
{
 protected:
  // Use a smart pointer to the actual data, so that ItemPtr remains movable and copyable.
  boost::intrusive_ptr<Item> shared_item_ptr_;

 public:
  // Create a ConstItemPtr that points to an existing Item object.
  // Increment the reference count of the Item to keep it alive.
  ConstItemPtr(Item const* ptr) : shared_item_ptr_(const_cast<Item*>(ptr)) { }

  // Derived classes have virtual functions.
  // Allow destruction by (base class) pointer in case this object was allocated with new.
  virtual ~ConstItemPtr() = default;

  // Declaring the above destructor stops the compiler from generating move functions
  // (and deprecates the implicit generation of the copy constructor).
  // So we need to explicitly request them.
  ConstItemPtr(ConstItemPtr&&) = default;
  ConstItemPtr& operator=(ConstItemPtr&&) = default;
  // Declaring the move constructor stopped the compiler from generating the copy constructor
  // (so this is beyond a deprecation now).
  ConstItemPtr(ConstItemPtr const&) = default;
  ConstItemPtr& operator=(ConstItemPtr const&) = default;

  // Accessor for the pointed-to Item. Only give const- access.
  Item const& item() const { return *shared_item_ptr_; }

  // Give access to the attribute list of the item, allowing the user to read attributes.
  AttributeList const& attribute_list() const { return shared_item_ptr_->attribute_list(); }
};

class ItemPtr : public ConstItemPtr
{
 public:
  // Create a pointer to a new graph item.
  template<typename T>
  requires std::derived_from<T, Item>
  ItemPtr(std::type_identity<T>) : ConstItemPtr(new T) { }

  // Increment reference count of the item and become a pointer to it.
  ItemPtr(Item* item) : ConstItemPtr(item) { }

  // Give access to the attribute list of the item, allowing the user to add attributes.
  using ConstItemPtr::attribute_list;
  AttributeList& attribute_list() { return shared_item_ptr_->attribute_list(); }

  // Accessors.
  using ConstItemPtr::item;
  Item& item() { return *shared_item_ptr_; }
};

template<typename T>
struct ItemPtrTemplate : ItemPtr
{
  using item_type = T;

  ItemPtrTemplate() : ItemPtr(new T) { }

  // Accessors.
  T const& item() const { return static_cast<T const&>(*this->shared_item_ptr_); }
  T& item() { return static_cast<T&>(*this->shared_item_ptr_); }

  T const* operator->() const { return static_cast<T const*>(this->shared_item_ptr_.get()); }
  T* operator->() { return static_cast<T*>(this->shared_item_ptr_.get()); }
};

template<typename T>
struct ConstItemPtrTemplate : ConstItemPtr
{
  ConstItemPtrTemplate(T const* item) : ConstItemPtr(item) { }

  // Automatic conversion from ItemPtrTemplate.
  ConstItemPtrTemplate(ItemPtrTemplate<T> const& other) : ConstItemPtr(other) { }
  ConstItemPtrTemplate(ItemPtrTemplate<T>&& other) : ConstItemPtr(std::move(other)) { }

  // Accessor.
  T const& item() const { return static_cast<T const&>(*shared_item_ptr_); }

  T const* operator->() const { return static_cast<T const*>(this->shared_item_ptr_.get()); }
};

} // namespace cppgraphviz::dot
