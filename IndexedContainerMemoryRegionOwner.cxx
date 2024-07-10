#include "sys.h"
#include "Array.h"
#include "utils/pointer_hash.h"

namespace cppgraphviz {

namespace {
// Note that the values of the pointers are used to identify the strings.
char const* const array_label_prefix = "Array[";
char const* const vector_label_prefix = "Vector[";
} // namespace

void IndexedContainerMemoryRegionOwner::initialize(std::weak_ptr<GraphTracker> const& root_graph,
    char const* label_prefix,
    std::type_info const& index_type_info, std::string_view demangled_index_type_name)
{
  {
    dot::TableNodePtr::unlocked_type::wat table_node_ptr_w{table_node_ptr_.item()};
    table_node_ptr_w->add_attribute({"what", "IndexedContainerMemoryRegionOwner::table_node_ptr_"});
    // Instead of copying elements, we create new dot::NodePtr objects and use
    // those temporarily until they can be overwritten later in on_memory_region_usage.
    table_node_ptr_w->copy_elements([](size_t i){
          dot::NodePtr node_ptr;
          dot::NodePtr::unlocked_type::wat{node_ptr.item()}->add_attribute({"what", "default NodePtr for Array/Vector"});
          return node_ptr;
        }, number_of_elements_);
  }
  std::shared_ptr<GraphTracker> root_graph_tracker = root_graph.lock();
  // Paranoia check.
  ASSERT(root_graph_tracker);
  // Create a key that is unique for a _Index / root_graph pair, also discriminating between different label prefixes.
  uint64_t key = utils::pointer_hash_combine(index_type_info.hash_code(), root_graph_tracker.get());
  key = utils::pointer_hash_combine(key, label_prefix);
  {
    indexed_container_sets_t::wat indexed_container_sets_w{indexed_container_sets_};
    auto ibp = indexed_container_sets_w->try_emplace(key, "IndexedContainerMemoryRegionOwner::indexed_container_sets_");
    IndexedContainerSet& indexed_container_set = ibp.first->second;
    if (ibp.second)
    {
      // We should never get here when no demangled_index_type_name was passed.
      ASSERT(demangled_index_type_name.size() != 0);
      dot::GraphPtr::unlocked_type::wat{root_graph_tracker->graph_ptr().item()}->insert(indexed_container_set);
      ibp.first->second.set_label(label_prefix + std::string(demangled_index_type_name) + "]");
    }
    indexed_container_set.add_container(table_node_ptr_, dot::TableNodePtr::unlocked_type::crat{table_node_ptr_.item()});
  }
  // Add this array to the root graph, so that it will call initialize before writing the dot file.
  root_graph_tracker->tracked_wat()->add_array(MemoryRegionOwner::tracker_);
}

// This constructor is used by Array:
// begin_ is initialized immediately and get_begin_ isn't used (must be/remain nullptr).
// number_of_elements_ is initialized and never changes, and get_number_of_elements_ isn't used (must be/remain nullptr).
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
    char* begin, size_t element_size, size_t number_of_elements, std::type_info const& index_type_info,
    std::string const& demangled_index_type_name, std::string_view what) :
  MemoryRegionOwner({ begin, element_size * number_of_elements }),
  LabelNode(root_graph, what),
  begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements),
  id_to_node_map_(number_of_elements_),
  get_begin_(nullptr), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(" << root_graph << ", " <<
      (void*)begin << ", " << element_size << ", " << number_of_elements << ", index_type_info, \"" << what << "\")");
  initialize(root_graph, array_label_prefix, index_type_info, demangled_index_type_name);
}

// This constructor is used by Array, see above.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(char* begin, size_t element_size, size_t number_of_elements,
    std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what) :
  MemoryRegionOwner({ begin, element_size * number_of_elements }),
  LabelNode(what),
  begin_(begin), element_size_(element_size), number_of_elements_(number_of_elements),
  id_to_node_map_(number_of_elements_),
  get_begin_(nullptr), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(" <<
      (void*)begin << ", " << element_size << ", " << number_of_elements << ", index_type_info, " <<
      demangled_index_type_name << ", \"" << what << "\")");
  // Not implemented; among others we'll have to get the root graph from the memory region owner that we're in now(?)
  ASSERT(false);
}

// This constructor is used by Vector:
// begin_ isn't used (must be/remain nullptr) and get_begin_ must be initialized.
// number_of_elements_ is set to the initial (currently constructed) number of elements and get_number_of_elements_ must be
// set to nullptr (in order to use number_of_elements_) and be initialized later, after the whole vector has been constructed.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(std::weak_ptr<GraphTracker> const& root_graph,
    size_t element_size, size_t initial_number_of_elements, get_begin_type get_begin,
    std::type_info const& index_type_info, std::string const& demangled_index_type_name, std::string_view what) :
  LabelNode(root_graph, what),
  begin_(nullptr), element_size_(element_size), number_of_elements_(initial_number_of_elements),
  id_to_node_map_(number_of_elements_),
  get_begin_(get_begin), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(" << root_graph << ", " <<
      element_size << ", index_type_info, \"" << what << "\")");
  initialize(root_graph, vector_label_prefix, index_type_info, demangled_index_type_name);
}

// This constructor is used by Array, see above.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(
    threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
    char* begin,
    std::type_info const& index_type_info,
    std::string_view what) :
  MemoryRegionOwner({ begin, other->element_size_ * other->number_of_elements_ }),
  LabelNode(other, what),
  begin_(begin), element_size_(other->element_size_), number_of_elements_(other->number_of_elements_),
  id_to_node_map_(number_of_elements_),
  get_begin_(nullptr), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner const& " <<
      other.operator->() << ", " << (void*)begin << ", index_type_info, \"" << what << "\")");
  initialize(root_graph_tracker(), array_label_prefix, index_type_info, "");
}

// This constructor is used by Array, see above.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> orig,
    char* begin, std::string_view what) :
  MemoryRegionOwner(*orig, { begin, orig->element_size_ * orig->number_of_elements_ }),
  LabelNode(std::move(orig), what),
  begin_(begin), element_size_(orig->element_size_), number_of_elements_(orig->number_of_elements_),
  table_node_ptr_(std::move(orig->table_node_ptr_)), id_to_node_map_(std::move(orig->id_to_node_map_)),
  get_begin_(nullptr), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice,
      "IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner&& " << orig.operator->() << ", " <<
      (void*)begin << ", \"" << what << "\") [" << this << "]");
}

// This constructor is used by Vector, see above.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(threadsafe::LockFinalCopy<IndexedContainerMemoryRegionOwner> other,
    std::type_info const& index_type_info, std::string_view what) :
  LabelNode(other, what),
  begin_(nullptr), element_size_(other->element_size_), number_of_elements_(other->get_number_of_elements_(other.operator->())),
  id_to_node_map_(number_of_elements_),
  get_begin_(other->get_begin_), get_number_of_elements_(nullptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner const& " <<
      other.operator->() << ", index_type_info, \"" << what << "\")");
  initialize(root_graph_tracker(), vector_label_prefix, index_type_info, "");
}

// This constructor is used by Vector, see above.
IndexedContainerMemoryRegionOwner::IndexedContainerMemoryRegionOwner(threadsafe::LockFinalMove<IndexedContainerMemoryRegionOwner> other,
    std::string_view what) :
  LabelNode(std::move(other), what),
  begin_(nullptr), element_size_(other->element_size_), number_of_elements_(other->get_number_of_elements_(other.operator->())),
  table_node_ptr_(std::move(other->table_node_ptr_)), id_to_node_map_(std::move(other->id_to_node_map_)),
  get_begin_(other->get_begin_), get_number_of_elements_(nullptr)
{
  ASSERT(get_number_of_elements_);
  DoutEntering(dc::notice,
      "IndexedContainerMemoryRegionOwner(IndexedContainerMemoryRegionOwner&& " << other.operator->() <<
      ", \"" << what << "\") [" << this << "]");
}

void IndexedContainerMemoryRegionOwner::on_memory_region_usage(MemoryRegion const& owner_memory_region,
    MemoryRegion const& item_memory_region, dot::NodePtr* node_ptr_ptr)
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::on_memory_region_usage(" <<
      owner_memory_region << ", " << item_memory_region << ", " << node_ptr_ptr << ") [" << this << "]");

  char const* begin;
  if (begin_)
  {
    // This is an Array.
    begin = begin_;
  }
  else
  {
    // This is Vector.
    begin = get_begin_(this);
    // While resizing a vector get_begin_ might return the old memory region.
    MemoryRegion first_element{const_cast<char*>(begin), element_size_};
    if (!first_element.lays_within(owner_memory_region))
      begin = owner_memory_region.begin();       // Assume a vector allocates a memory region that starts with the first elements :/
  }

  char* item_begin = item_memory_region.begin();
  Item* item = reinterpret_cast<Item*>(item_begin);
  ptrdiff_t offset = item_begin - begin;
  size_t index = offset / element_size_;

  size_t number_of_elements;
  if (begin_)
  {
    // This is an Array.
    number_of_elements = number_of_elements_;
  }
  else
  {
    // This is Vector.
    if (get_number_of_elements_)
    {
      number_of_elements = get_number_of_elements_(this);
      // vector::size() might not return the correct value yet.
      if (number_of_elements <= index)
        number_of_elements = index + 1;
    }
    else
      number_of_elements = number_of_elements_;

    if (id_to_node_map_.size() != number_of_elements)
    {
      id_to_node_map_.resize(number_of_elements);
      dot::NodePtr node_ptr;
      dot::NodePtr::unlocked_type::wat{node_ptr.item()}->add_attribute({"what", "default NodePtr for Vector"});
      dot::TableNodePtr::unlocked_type::wat{table_node_ptr_.item()}->resize_copied_elements(number_of_elements, node_ptr);
    }
  }

  Dout(dc::notice, "index = " << index << "; number_of_elements = " << number_of_elements);
  ASSERT(0 <= index && index < number_of_elements);

  // node_ptr_ptr can't be nullptr, because if this is to be added to a table node then
  // the item has to be a dot::NodeItem (still being constructed though).
  ASSERT(node_ptr_ptr);
  dot::NodePtr& node_ptr = *node_ptr_ptr;
  dot::TableNodePtr::unlocked_type::wat{table_node_ptr_.item()}->replace_element(index, node_ptr);
  // And item has to be Node.
  Node* node = static_cast<Node*>(item);
  std::weak_ptr<NodeTracker> weak_node_tracker = *node;
  Dout(dc::notice, "Mapping id_to_node_map_[" << index << "] = " << weak_node_tracker);
  id_to_node_map_[index] = weak_node_tracker;

  // Array elements should be created without root graph. They get that when they are copied into the array, here.
  item->set_root_graph_tracker(root_graph_tracker_);
}

void IndexedContainerMemoryRegionOwner::call_initialize_on_elements()
{
  DoutEntering(dc::notice, "IndexedContainerMemoryRegionOwner::call_initialize_on_elements() [" << this << "]");
  size_t index = 0;
  // This calls the lambda for indices 0, 1, 2, ...
  dot::TableNodePtr::unlocked_type::wat{table_node_ptr_.item()}->for_all_elements([this, &index](){
    Dout(dc::notice, "index = " << index);
    auto node_tracker = id_to_node_map_[index].lock();
    ++index;
    if (node_tracker)
      node_tracker->tracked_wat()->initialize_item();
  });
}

//static
IndexedContainerMemoryRegionOwner::indexed_container_sets_t IndexedContainerMemoryRegionOwner::indexed_container_sets_;

} // namespace cppgraphviz
