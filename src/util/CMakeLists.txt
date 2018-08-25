set(NAME util)

set(HEADERS
   BBox.h
   CircleNeighborhood.hpp
   Graph.hpp
   GraphEdge.hpp
   GraphFaces.hpp
   GraphPlanar.hpp
   GraphTraversal.hpp
   Heap.hpp
   LineDetector6.hpp
   Mesh.hpp
   MeshSelection.hpp
   Partition.hpp
   PartitionLists.hpp
   Polylines.hpp
   StaticRotation.h
   Vec.hpp
) # HEADERS    

set(SOURCES
   BBox.cpp
   CircleNeighborhood.cpp
   Graph.cpp
   GraphFaces.cpp
   GraphPlanar.cpp
   GraphTraversal.cpp
   Heap.cpp
   LineDetector6.cpp
   Mesh.cpp
   MeshSelection.cpp
   Partition.cpp
   PartitionLists.cpp
   Polylines.cpp
   StaticRotation.cpp
   Vec.cpp
) # SOURCES

qt5_wrap_ui(FORMS
) # FORMS

qt5_add_resources(ASSETS
) # ASSETS

add_library(${NAME}
    ${HEADERS}
    ${SOURCES}
    ${FORMS}
    ${ASSETS}
) # add_library

target_link_libraries(${NAME} ${LIB_LIST})
