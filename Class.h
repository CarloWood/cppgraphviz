#pragma once

#include "Node.h"

namespace cppgraphviz {

template<typename T>
class Class : public Graph
{
 protected:
  std::string label_;

 public:
  Class(std::weak_ptr<GraphTracker> const& root_graph, std::string_view what) :
    Graph(MemoryRegion{reinterpret_cast<char*>(static_cast<T*>(this)), sizeof(T)}, root_graph, what)
  {
    DoutEntering(dc::notice, "Class<" << libcwd::type_info_of<T>().demangled_name() << ">(" <<
        root_graph << ", \"" << what << "\") [" << this << "]");
  }

  Class(Class const& other, std::string_view what) :
    Graph(MemoryRegion{reinterpret_cast<char*>(static_cast<T*>(this)), sizeof(T)}, other, what),
    label_(other.label_)
  {
    DoutEntering(dc::notice, "Class<" << libcwd::type_info_of<T>().demangled_name() << ">(Class const& " <<
        &other << ", \"" << what << "\") [" << this << "]");
  }

  Class(Class&& other, std::string_view what) : Graph(std::move(other), what), label_(std::move(other.label_))
  {
    DoutEntering(dc::notice, "Class<" << libcwd::type_info_of<T>().demangled_name() << ">(Class&& " <<
        &other << ", \"" << what << "\") [" << this << "]");
  }

  void set_label(std::string const& label)
  {
    label_ = label;
  }

 private:
  // Implement virtual function of MemoryRegionOwner.
  void on_memory_region_usage(MemoryRegion const& item_memory_region, dot::NodePtr* UNUSED_ARG(node_ptr_ptr)) override
  {
    // `item_memory_region` starts at the `Item* object` that was passed to inform_owner_of in the constructor of some Item.
    Item* item = reinterpret_cast<Item*>(item_memory_region.begin());
    std::weak_ptr<GraphTracker> subgraph_tracker = *this;
    item->set_parent_graph_tracker(std::move(subgraph_tracker));
  }

 protected:
  void item_attributes(dot::AttributeList& list) override
  {
    std::string prev_style;
    if (list.has_key("style"))
    {
      prev_style = std::string{list.get_value("style")} + ",";
      list.remove("style");
    }
    list += {{"cluster", "true"}, {"style", prev_style + "rounded"}};
    // Derive from Class and override item_attributes to add a shape, color etc.
    // Call set_label to set the label, or derive from Class and override item_attributes to add a label.
    if (label_.empty())
      list.add({"label", "<unknown Class>"});
    else
      list += {"label", label_};
  }
};

} // namespace cppgraphviz
