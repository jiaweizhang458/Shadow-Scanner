//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 16:03:34 taubin>
//------------------------------------------------------------------------
//
// IndexedLineSet.cpp
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
#include "IndexedLineSet.h"

// VRML'97
//
// IndexedLineSet {
//   eventIn       MFInt32 set_colorIndex
//   eventIn       MFInt32 set_coordIndex
//   exposedField  SFNode  coord             NULL
//   field         MFInt32 coordIndex        []        # [-1,)
//   exposedField  SFNode  color             NULL
//   field         MFInt32 colorIndex        []        # [-1,)
//   field         SFBool  colorPerVertex    TRUE
// }

IndexedLineSet::IndexedLineSet():
  _colorPerVertex(true)
{}

void IndexedLineSet::clear() {
  _coord.clear();
  _coordIndex.clear();
  _color.clear();
  _colorIndex.clear();
  _colorPerVertex  = true;
}

bool&          IndexedLineSet::getColorPerVertex()   { return _colorPerVertex;     }
vector<float>& IndexedLineSet::getCoord()            { return _coord;              }
vector<int>&   IndexedLineSet::getCoordIndex()       { return _coordIndex;         }
vector<float>& IndexedLineSet::getColor()            { return _color;              }
vector<int>&   IndexedLineSet::getColorIndex()       { return _colorIndex;         }

int            IndexedLineSet::getNumberOfCoord()    { return (int)(_coord.size()/3);    }
int            IndexedLineSet::getNumberOfColor()    { return (int)(_color.size()/3);    }

int IndexedLineSet::getNumberOfPolylines()   {
  int nPolylines = 0;
  for(int i=0;i<(int)_coordIndex.size();i++)
    if(_coordIndex[i]<0)
      nPolylines++;
  return nPolylines;
}

void IndexedLineSet::setColorPerVertex(bool value) {
  _colorPerVertex = value;
}

void IndexedLineSet::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "IndexedLineSet {\n";
  std::cout << indent << "  nPolylines        = " << getNumberOfPolylines() << "\n";
  std::cout << indent << "  nCoord            = " << _coord.size()/3        << "\n";
  std::cout << indent << "  coordIndex.size() = " << _coordIndex.size()     << "\n";
  std::cout << indent << "  colorPerVertex    = " << _colorPerVertex        << "\n";
  std::cout << indent << "  nColor            = " << _color.size()/3        << "\n";
  std::cout << indent << "  colorIndex.size() = " << _colorIndex.size()     << "\n";
  std::cout << indent << "}\n";
}
