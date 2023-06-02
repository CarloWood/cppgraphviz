add_library(dot_ObjLib OBJECT)

target_sources(dot_ObjLib
  PRIVATE
    Attribute.cxx
    Attribute.h
    AttributeList.cxx
    AttributeList.h
    DotID.cxx
    DotID.h
    Edge.cxx
    Edge.h
    Graph.cxx
    Graph.h
    Port.cxx
    Port.h
    TableNode.cxx
    TableNode.h
    Node.cxx
    Node.h
    Item.h
    item_types.h
)

# Required include search-paths.
get_target_property(CWDS_INTERFACE_INCLUDE_DIRECTORIES AICxx::cwds INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(dot_ObjLib
  PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
  PUBLIC
    "${CWDS_INTERFACE_INCLUDE_DIRECTORIES}" # For sys.h and debug.h.
)

# Make sure we compile with debug support.
target_link_libraries(dot_ObjLib PUBLIC ${AICXX_OBJECTS_LIST})

# Create an ALIAS target.
add_library(CppGraphviz::dot ALIAS dot_ObjLib)
