cmake_minimum_required(VERSION 3.12...3.26)

project(libcppgraphviz
  DESCRIPTION "A collection of classes that allow to chart the relationship between C++ objects as a graph"
  LANGUAGES CXX
)

include(AICxxProject)

#==============================================================================
# OPTIONS

# Option 'EnableCppGraphvizWhat' is available when OptionEnableLibcwd is ON (otherwise OFF).
cw_option(EnableCppGraphvizWhat
          "Enable debug code that allows setting the 'what' attribute" ON
          "OptionEnableLibcwd" OFF
)

if (OptionEnableLibcwd)
  message(DEBUG "OptionEnableCppGraphvizWhat is ${OptionEnableCppGraphvizWhat}")
endif ()
if (OptionEnableCppGraphvizWhat)
  set(CPPGRAPHVIZ_USE_WHAT 1)
endif ()

# Specify configure file.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h
               @ONLY)

#==============================================================================

# Create a shared library.
add_library(libcppgraphviz SHARED)

target_sources(libcppgraphviz
  PRIVATE
    Graph.cpp
    Graph.hpp
    GraphTracker.cpp
    GraphTracker.hpp
    IndexedContainerSet.hpp
    Item.cpp
    Item.hpp
    ItemTracker.cpp
    ItemTracker.hpp
    LabelNode.hpp
    MemoryRegion.hpp
    MemoryRegionToOwnerLinker.cpp
    MemoryRegionToOwnerLinker.hpp
    Node.hpp
    NodeTracker.cpp
    NodeTracker.hpp
    debug_ostream_operators.hpp
)

target_link_libraries(libcppgraphviz
 PRIVATE
  CppGraphviz::dot
)

add_subdirectory(dot)
