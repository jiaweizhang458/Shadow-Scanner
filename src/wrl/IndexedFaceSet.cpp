//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 16:01:29 taubin>
//------------------------------------------------------------------------
//
// IndexedFaceSet.cpp
//
// Software developed for the Fall 2015 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2015, Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include "IndexedFaceSet.h"

// VRML'97
//
// IndexedFaceSet {
//   eventIn       MFInt32 set_colorIndex
//   eventIn       MFInt32 set_coordIndex
//   eventIn       MFInt32 set_normalIndex
//   eventIn       MFInt32 set_texCoordIndex
//   exposedField  SFNode  color             NULL
//   exposedField  SFNode  coord             NULL
//   exposedField  SFNode  normal            NULL
//   exposedField  SFNode  texCoord          NULL
//   field         SFBool  ccw               TRUE
//   field         MFInt32 colorIndex        []        # [-1,)
//   field         SFBool  colorPerVertex    TRUE
//   field         SFBool  convex            TRUE
//   field         MFInt32 coordIndex        []        # [-1,)
//   field         SFFloat creaseAngle       0         # [ 0,)
//   field         MFInt32 normalIndex       []        # [-1,)
//   field         SFBool  normalPerVertex   TRUE
//   field         SFBool  solid             TRUE
//   field         MFInt32 texCoordIndex     []        # [-1,)
// }

IndexedFaceSet::IndexedFaceSet():
  _ccw(true),
  _convex(true),
  _creaseAngle(0),
  _solid(true),
  _normalPerVertex(true),
  _colorPerVertex(true)
{}

void IndexedFaceSet::clear() {
  _ccw             = true;
  _convex          = true;
  _creaseAngle     = 0.0;
  _solid           = true;
  _normalPerVertex = true;
  _colorPerVertex  = true;
  _coord.clear();
  _coordIndex.clear();
  _normal.clear();
  _normalIndex.clear();
  _color.clear();
  _colorIndex.clear();
  _texCoord.clear();
  _texCoordIndex.clear();
}

bool&          IndexedFaceSet::getCcw()              { return _ccw;                }
bool&          IndexedFaceSet::getConvex()           { return _convex;             }
float&         IndexedFaceSet::getCreaseangle()      { return _creaseAngle;        }
bool&          IndexedFaceSet::getSolid()            { return _solid;              }
bool&          IndexedFaceSet::getNormalPerVertex()  { return _normalPerVertex;    }
bool&          IndexedFaceSet::getColorPerVertex()   { return _colorPerVertex;     }
vector<float>& IndexedFaceSet::getCoord()            { return _coord;              }
vector<int>&   IndexedFaceSet::getCoordIndex()       { return _coordIndex;         }
vector<float>& IndexedFaceSet::getNormal()           { return _normal;             }
vector<int>&   IndexedFaceSet::getNormalIndex()      { return _normalIndex;        }
vector<float>& IndexedFaceSet::getColor()            { return _color;              }
vector<int>&   IndexedFaceSet::getColorIndex()       { return _colorIndex;         }
vector<float>& IndexedFaceSet::getTexCoord()         { return _texCoord;           }
vector<int>&   IndexedFaceSet::getTexCoordIndex()    { return _texCoordIndex;      }
int            IndexedFaceSet::getNumberOfCoord()    { return (int)(_coord.size()/3);    }
int            IndexedFaceSet::getNumberOfNormal()   { return (int)(_normal.size()/3);   }
int            IndexedFaceSet::getNumberOfColor()    { return (int)(_color.size()/3);    }
int            IndexedFaceSet::getNumberOfTexCoord() { return (int)(_texCoord.size()/2); }

int IndexedFaceSet::getNumberOfFaces()   {
  int nFaces = 0;
  for(int i=0;i<(int)_coordIndex.size();i++)
    if(_coordIndex[i]<0)
      nFaces++;
  return nFaces;
}

int IndexedFaceSet::getNumberOfCorners() {
  return (int)(_coordIndex.size())-getNumberOfFaces();
}
  
IndexedFaceSet::Binding IndexedFaceSet::getCoordBinding() {
  return PB_PER_VERTEX;
}
  
IndexedFaceSet::Binding IndexedFaceSet::getNormalBinding() {
  // if(normal.size()==0) {
  //   // NO_NORMALS
  // } else if(normalPerVertex==FALSE) {
  //   if(normalIndex.size()>0) {
  //     // NORMAL_PER_FACE_INDEXED;
  //     // assert(normalIndex.size()==getNumberOfFaces())
  //   } else {
  //     // NORMAL_PER_FACE;
  //     // assert(normal.size()/3==getNumberOfFaces())
  //   }
  // } else /* if(normalPerVertex==TRUE) */ {
  //   if(normalIndex.size()>0) {
  //     // NORMAL_PER_CORNER_INDEXED;
  //     // assert(normalIndex.size()==coordIndex.size())
  //   } else {
  //     // NORMAL_PER_VERTEX;
  //     // assert(normal.size()/3==coord.size()/3)
  //   }
  // }
  return
    (_normal.size()==0      )?PB_NONE:
    (_normalPerVertex==false)?
    ((_normalIndex.size()>0  )?PB_PER_FACE_INDEXED:PB_PER_FACE  ):
    ((_normalIndex.size()>0  )?PB_PER_CORNER      :PB_PER_VERTEX);
}

IndexedFaceSet::Binding IndexedFaceSet::getColorBinding() {
  // if(color.size()==0) {
  //   // NO_COLORS
  // } else if(colorPerVertex==FALSE) {
  //   if(colorIndex.size()>0) {
  //     // COLOR_PER_FACE_INDEXED;
  //     // assert(colorIndex.size()==getNumberOfFaces())
  //   } else {
  //     // COLOR_PER_FACE;
  //     // assert(color.size()/3==getNumberOfFaces())
  //   }
  // } else /* if(colorPerVertex==TRUE) */ {
  //   if(colorIndex.size()>0) {
  //     // COLOR_PER_CORNER_INDEXED;
  //     // assert(colorIndex.size()==coordIndex.size())
  //   } else {
  //     // COLOR_PER_VERTEX;
  //     // assert(color.size()/3==coord.size()/3)
  //   }
  // }
  return
    (_color.size()==0      )?PB_NONE:
    (_colorPerVertex==false)?
    ((_colorIndex.size()>0  )?PB_PER_FACE_INDEXED:PB_PER_FACE  ):
    ((_colorIndex.size()>0  )?PB_PER_CORNER      :PB_PER_VERTEX);
}

IndexedFaceSet::Binding IndexedFaceSet::getTexCoordBinding() {
  // if(texCoord.size()==0) {
  //   // NO_TEX_COORD
  // } else if(texCoordIndex.size()>0) {
  //   // TEX_COORD_PER_CORNER_INDEXED;
  //   // assert(texCoordIndex.size()==coordIndex.size());
  // } else {
  //   // TEX_COORD_PER_VERTEX;
  //   // assert(texCoord.size()/3==coord.size()/3)
  // }
  return
    (_texCoord.size()==0    )?PB_NONE:
    (_texCoordIndex.size()>0)?PB_PER_CORNER:
    PB_PER_VERTEX;
}

void IndexedFaceSet::setNormalPerVertex(bool value) {
  _normalPerVertex = value;
}

void IndexedFaceSet::setColorPerVertex(bool value) {
  _colorPerVertex = value;
}

void IndexedFaceSet::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "IndexedFaceSet {\n";
  std::cout << indent << "  nFaces             = " <<
    getNumberOfFaces() << "\n";
  std::cout << indent << "  ccw                = " <<
    _ccw << "\n";
  std::cout << indent << "  convex             = " <<
    _convex << "\n";
  std::cout << indent << "  creaseAngle        = " <<
    _creaseAngle << "\n";
  std::cout << indent << "  solid              = " <<
    _solid << "\n";
  std::cout << indent << "  coordBinding       = " <<
    stringBinding(getCoordBinding()) << "\n";
  std::cout << indent << "  nCoord             = " <<
    _coord.size()/3 << "\n";
  std::cout << indent << "  coordIndex.size()  = " <<
    _coordIndex.size() << "\n";
  std::cout << indent << "  normalBinding      = " <<
    stringBinding(getNormalBinding()) << "\n";
  std::cout << indent << "  normalPerVertex    = " <<
    _normalPerVertex << "\n";
  std::cout << indent << "  nNormal            = " <<
    _normal.size()/3 << "\n";
  std::cout << indent << "  normalIndex.size() = " <<
    _normalIndex.size() << "\n";
  std::cout << indent << "  colorBinding       = " <<
    stringBinding(getColorBinding()) << "\n";
  std::cout << indent << "  colorPerVertex     = " <<
    _colorPerVertex << "\n";
  std::cout << indent << "  nColor             = " <<
    _color.size()/3 << "\n";
  std::cout << indent << "  colorIndex.size()  = " <<
    _colorIndex.size() << "\n";
  std::cout << indent << "  texCoordBinding    = " <<
    stringBinding(getTexCoordBinding()) << "\n";
  std::cout << indent << "  nTexCoord          = " <<
    _texCoord.size()/2 << "\n";
  std::cout << indent << "  texCoord.siZe()    = " <<
    _texCoordIndex.size() << "\n";
  std::cout << indent << "}\n";
}
