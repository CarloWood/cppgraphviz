#pragma once

#include "AttributeList.h"
#include "Item.h"
#include <utils/Badge.h>
#include <threadsafe/threadsafe.h>
#include <boost/intrusive_ptr.hpp>
#include <concepts>

namespace cppgraphviz::dot {

// A wrapper around a boost::intrusive_ptr<Item, ItemLockingPolicy>.
//
// ItemPtr's can be cheaply moved and even copied: changes made
// to one ItemPtr affect all other ItemPtr that are/were copies.
//
// The (shared) data that a ItemPtr points to (Item) will be
// a base class of either GraphItem, NodeItem or EdgeItem.
class ConstItemPtr
{
 public:
  using unlocked_type = Item::unlocked_type;

 protected:
  // Use a smart pointer to the actual data, so that ItemPtr remains movable and copyable.
  // Note that unlocked_type increments a reference count because Item is derived from AIRefCount.
  unlocked_type shared_item_ptr_;

 public:
  // Create a ConstItemPtr that points to an existing Item object.
  // Increment the reference count of the Item to keep it alive.
  ConstItemPtr(unlocked_type const& item) : shared_item_ptr_(const_cast<unlocked_type&>(item)) { }

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
  unlocked_type const& item() const { return shared_item_ptr_; }
};

class ItemPtr : public ConstItemPtr
{
 public:
  // Create a pointer to a new graph item.
  template<typename T>
  requires std::derived_from<T, Item>
  ItemPtr(std::type_identity<T>) : ConstItemPtr(new threadsafe::Unlocked<T, ItemLockingPolicy>) { }

  // Increment reference count of the item and become a pointer to it.
  ItemPtr(unlocked_type item) : ConstItemPtr(item) { }

  // Accessors.
  using ConstItemPtr::item;
  unlocked_type& item() { return shared_item_ptr_; }
};

template<typename T>
struct ItemPtrTemplate : ItemPtr
{
  using item_type = T;
  using unlocked_type = typename T::unlocked_type;

  ItemPtrTemplate() : ItemPtr(*boost::intrusive_ptr<unlocked_type>(new unlocked_type)) { }

  // Accessors.
  unlocked_type const& item() const { return unlocked_cast<unlocked_type const&>(this->shared_item_ptr_); }
  unlocked_type& item() { return unlocked_cast<unlocked_type&>(this->shared_item_ptr_); }
};

template<typename T>
struct ConstItemPtrTemplate : ConstItemPtr
{
  using item_type = T;
  using unlocked_type = threadsafe::Unlocked<T, ItemLockingPolicy>;

  ConstItemPtrTemplate(T const* item) : ConstItemPtr(item) { }

  // Automatic conversion from ItemPtrTemplate.
  ConstItemPtrTemplate(ItemPtrTemplate<T> const& other) : ConstItemPtr(other) { }
  ConstItemPtrTemplate(ItemPtrTemplate<T>&& other) : ConstItemPtr(std::move(other)) { }

  // Accessor.
  unlocked_type const& item() const { return unlocked_cast<unlocked_type const&>(shared_item_ptr_); }
};

} // namespace cppgraphviz::dot
