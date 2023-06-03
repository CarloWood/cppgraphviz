#pragma once

#include "Node.h"

namespace cppgraphviz {

template<typename T>
class Class : public Graph
{
 public:
  Class(std::weak_ptr<GraphTracker> const& root_graph, char const* what) :
    Graph({reinterpret_cast<char*>(static_cast<T*>(this)), sizeof(T)}, root_graph, what)
  {
    DoutEntering(dc::notice, "Class<" << libcwd::type_info_of<T>().demangled_name() << ">(" <<
        root_graph << ", \"" << what << "\") [" << this << "]");
  }

  Class(Class const& other, char const* what) :
    Graph({reinterpret_cast<char*>(static_cast<T*>(this)), sizeof(T)}, other, what)
  {
    DoutEntering(dc::notice, "Class<" << libcwd::type_info_of<T>().demangled_name() << ">(Class const& " <<
        &other << ", \"" << what << "\") [" << this << "]");
  }

  // Moving is the same as copying in this context.
  Class(Class&& other, char const* what) : Class(other, what)
  {
  }

 private:
  // Implement virtual function of MemoryRegionOwner.
  void on_memory_region_usage(MemoryRegion const& item_memory_region) override
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
    list += {{"cluster", "true"}, {"style", prev_style + "rounded"}, {"label", "<Class>"}};
  }
};

} // namespace cppgraphviz
