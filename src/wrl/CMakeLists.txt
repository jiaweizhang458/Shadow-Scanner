set(NAME wrl)

set(HEADERS
  Appearance.h
  Group.h
  HexahedralMesh.h
  ImageTexture.h
  IndexedFaceSet.h
  IndexedLineSet.h
  IsoSurf.h
  Material.h
  Node.h
  PixelTexture.h
  Rotation.h
  SceneGraph.h
  SceneGraphProcessor.h
  SceneGraphTraversal.h
  Shape.h
  SimpleGraphMap.h
  Transform.h
) # HEADERS    

set(SOURCES
  Appearance.cpp
  Group.cpp
  HexahedralMesh.cpp
  ImageTexture.cpp
  IndexedFaceSet.cpp
  IndexedLineSet.cpp
  IsoSurf.cpp
  Material.cpp
  Node.cpp
  PixelTexture.cpp
  Rotation.cpp
  SceneGraph.cpp
  SceneGraphProcessor.cpp
  SceneGraphTraversal.cpp
  Shape.cpp
  SimpleGraphMap.cpp
  Transform.cpp
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
