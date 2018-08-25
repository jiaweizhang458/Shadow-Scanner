DESTDIR = ../../lib

TEMPLATE = lib
CONFIG += staticlib c++11
INCLUDEPATH += .. ../../../eigen

NAME = wrl

CONFIG(release, debug|release) {
    TARGET = $$NAME
} else {
    TARGET = $${NAME}_d
}

HEADERS += \
  Node.h \
  SceneGraph.h \
  SceneGraphTraversal.h \
  SceneGraphProcessor.h \
  Group.h \
  Transform.h \
  Shape.h \
  Appearance.h \
  Material.h \
  PixelTexture.h \
  ImageTexture.h \
  IndexedFaceSet.h \
  IndexedLineSet.h \
  Rotation.h \
  SimpleGraphMap.h \
  IsoSurf.h \
  HexahedralMesh.h

SOURCES += \
  Node.cpp \
  SceneGraph.cpp \
  SceneGraphTraversal.cpp \
  SceneGraphProcessor.cpp \
  Group.cpp \
  Transform.cpp \
  Shape.cpp \
  Appearance.cpp \
  Material.cpp \
  PixelTexture.cpp \
  ImageTexture.cpp \
  IndexedFaceSet.cpp \
  IndexedLineSet.cpp \
  Rotation.cpp \
  SimpleGraphMap.cpp \
  IsoSurf.cpp \
  HexahedralMesh.cpp
