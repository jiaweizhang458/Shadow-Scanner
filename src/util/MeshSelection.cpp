//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 16:09:43 taubin>
//------------------------------------------------------------------------
//
// MeshSelection.cpp
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

#include "MeshSelection.hpp"

MeshSelection::MeshSelection(Mesh& treeMesh):
  _mesh(treeMesh),
  _selVertex(),
  _selFace() {
  setSelectedVertices(false);
  setSelectedFaces(false);
}

void MeshSelection::fit() {
  int nV = _mesh.getNumberOfVertices();
  if(_selVertex.size()>nV) {
    while(_selVertex.size()>nV)
      _selVertex.popBack();
  } else if(_selVertex.size()<nV) {
    while(_selVertex.size()<nV)
      _selVertex.pushBack(false);
  }
  int nF = _mesh.getNumberOfFaces();
  if(_selFace.size()>nF) {
    while(_selFace.size()>nF)
      _selFace.popBack();
  } else if(_selFace.size()<nF) {
    while(_selFace.size()<nF)
      _selFace.pushBack(false);
  }
}

void MeshSelection::erase() {
  _selFace.clear();
  _selVertex.clear();
}

void MeshSelection::erase(bool value) {
  erase();
  fit();
}

bool MeshSelection::hasSelectionFace() {
  return (_selFace.size()==_mesh.getNumberOfFaces());
}
VecBit& MeshSelection::getSelectionFace() {
  return _selFace;
}
int MeshSelection::getSelectionFaceSize() {
  return _selFace.size();
}
bool MeshSelection::getSelectedFace(int iF) {
  return _selFace.get(iF);
}
void MeshSelection::setSelectedFace(int iF, bool value) {
  _selFace.set(iF,value);
}
void MeshSelection::setSelectedFaces(bool value) {
  int nC = _mesh.getNumberOfFaces();
  _selFace.clear();
  for(int iF=0;iF<nC;iF++)
    _selFace.pushBack(value);
}
void MeshSelection::toggleSelectedFace(int iF) {
  bool value = _selFace.get(iF);
  _selFace.set(iF,value);
}

bool MeshSelection::hasSelectionVertex() {
  return (_selVertex.size()==_mesh.getNumberOfVertices());
}
VecBit& MeshSelection::getSelectionVertex() {
  return _selVertex;
}
int MeshSelection::getSelectionVertexSize() {
  return _selVertex.size();
}
bool MeshSelection::getSelectedVertex(int iV) {
  return _selVertex.get(iV);
}
void MeshSelection::setSelectedVertex(int iV, bool value) {
  _selVertex.set(iV,value);
}
void MeshSelection::setSelectedVertices(bool value) {
  int nV = _mesh.getNumberOfVertices();
  _selVertex.clear();
  for(int iV=0;iV<nV;iV++)
    _selVertex.pushBack(value);
}
void MeshSelection::toggleSelectedVertex(int iV) {
  bool value = _selVertex.get(iV);
  _selVertex.set(iV,!value);
}

