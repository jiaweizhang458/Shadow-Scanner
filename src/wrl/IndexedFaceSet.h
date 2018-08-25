//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 15:58:06 taubin>
//------------------------------------------------------------------------
//
// IndexedFaceSet.h
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

#ifndef _IndexedFaceSet_h_
#define _IndexedFaceSet_h_

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

#include "Node.h"
#include <vector>

using namespace std;

class IndexedFaceSet : public Node {

private:

  bool           _ccw;
  bool           _convex;
  float          _creaseAngle;
  bool           _solid;

  vector<float>  _coord;
  vector<int>    _coordIndex;

  bool           _normalPerVertex;
  vector<float>  _normal;
  vector<int>    _normalIndex;

  bool           _colorPerVertex;
  vector<float>  _color;
  vector<int>    _colorIndex;

  vector<float>  _texCoord;
  vector<int>    _texCoordIndex;

public:
  
  IndexedFaceSet();

  void            clear();
  bool&           getCcw();
  bool&           getConvex();
  float&          getCreaseangle();
  bool&           getSolid();
  bool&           getNormalPerVertex();
  bool&           getColorPerVertex();
  vector<float>&  getCoord();
  vector<int>&    getCoordIndex();
  vector<float>&  getNormal();
  vector<int>&    getNormalIndex();
  vector<float>&  getColor();
  vector<int>&    getColorIndex();
  vector<float>&  getTexCoord();
  vector<int>&    getTexCoordIndex();

  int             getNumberOfFaces();
  int             getNumberOfCorners();

  int             getNumberOfCoord();
  int             getNumberOfNormal();
  int             getNumberOfColor();
  int             getNumberOfTexCoord();

  void            setNormalPerVertex(bool value);
  void            setColorPerVertex(bool value);

  enum Binding {
    PB_NONE = 0,
    PB_PER_VERTEX,
    PB_PER_FACE,
    PB_PER_FACE_INDEXED,
    PB_PER_CORNER
  };

  static const string stringBinding(Binding b) {
    return
      (b==PB_PER_VERTEX      )?"PER_VERTEX":
      (b==PB_PER_FACE        )?"PER_FACE":
      (b==PB_PER_FACE_INDEXED)?"PER_FACE_INDEXED":
      (b==PB_PER_CORNER      )?"PER_CORNER":
                               "NONE";
  }
  
  Binding         getCoordBinding();
  Binding         getNormalBinding();
  Binding         getColorBinding();
  Binding         getTexCoordBinding();

  virtual bool    isIndexedFaceSet() const { return             true; }
  virtual string  getType()          const { return "IndexedFaceSet"; }
  typedef bool    (*Property)(IndexedFaceSet& ifs);
  typedef void    (*Operator)(IndexedFaceSet& ifs);

  virtual void    printInfo(string indent);
};

#endif /* _IndexedFaceSet_h_ */
