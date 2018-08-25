//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 16:11:16 taubin>
//------------------------------------------------------------------------
//
// MeshSelection.hpp
//
// Copyright (c) 2009-2012, Gabriel Taubin
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

#ifndef _MeshSelection_hpp_
#define _MeshSelection_hpp_

#include "Vec.hpp"
#include "Mesh.hpp"

class MeshSelection {

private:

  Mesh&  _mesh;
  VecBit _selVertex;
  VecBit _selFace;

public:

  MeshSelection(Mesh& treeMesh);

  void    fit();
  void    erase();
  void    erase(bool value);
  bool    hasSelectionFace();
  VecBit& getSelectionFace();
  int     getSelectionFaceSize();
  bool    getSelectedFace(int iF);
  void    setSelectedFace(int iF, bool value);
  void    setSelectedFaces(bool value);
  void    toggleSelectedFace(int iF);

  bool    hasSelectionVertex();
  VecBit& getSelectionVertex();
  int     getSelectionVertexSize();
  bool    getSelectedVertex(int iV);
  void    setSelectedVertex(int iV, bool value);
  void    setSelectedVertices(bool value);
  void    toggleSelectedVertex(int iV);
  
};

#endif // _MeshSelection_hpp_
