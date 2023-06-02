cmake_minimum_required(VERSION 3.12...3.26)

project(cppgraphviz
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

#==============================================================================

# Specify configure file.
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/config.h
               @ONLY)

#==============================================================================
# BUILD OBJECT LIBRARY
#

# This project is an OBJECT-library, used by other git submodules and the main project.
add_library(cppgraphviz_ObjLib OBJECT)

# Tell sys.h that we have a config.h.
target_compile_definitions(cppgraphviz_ObjLib
  PUBLIC
    HAVE_CPPGRAPHVIZ_CONFIG_H
)

# Set SOURCE_DIR to the current directory.
set_property(
  TARGET cppgraphviz_ObjLib
  PROPERTY SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

#==============================================================================

# The list of source files.
target_sources(cppgraphviz_ObjLib
  PRIVATE
    Array.h
    Graph.cxx
    Graph.h
    GraphTracker.cxx
    GraphTracker.h
    IndexedContainerSet.h
    Item.cxx
    Item.h
    ItemTracker.cxx
    ItemTracker.h
    LabelNode.h
    MemoryRegion.h
    MemoryRegionToOwnerLinker.cxx
    MemoryRegionToOwnerLinker.h
    Node.h
    NodeTracker.cxx
    NodeTracker.h
    debug_ostream_operators.h
)

target_link_libraries(cppgraphviz_ObjLib
  PUBLIC
    CppGraphviz::dot
)

add_subdirectory(dot)

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(cppgraphviz_ObjLib
  PUBLIC
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For sys.h and debug.h.
    "${top_objdir}"                         # For cppgraphviz/config.h.
)

# Require support for C++20.
target_compile_features(cppgraphviz_ObjLib
  PUBLIC cxx_std_20
)

# Create an ALIAS target.
add_library(AICxx::cppgraphviz ALIAS cppgraphviz_ObjLib)
