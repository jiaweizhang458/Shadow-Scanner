//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 15:58:02 taubin>
//------------------------------------------------------------------------
//
// Mesh.hpp
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

#ifndef _Mesh_hpp_
#define _Mesh_hpp_

#include <iostream>
#include <QString>
#include <QVector>

typedef QVector<int>   VecInt;
typedef QVector<float> VecFloat;

class Graph;
class MeshSelection;

class Mesh {

private:

  static void _log(QString s) {
    std::cerr << "Mesh | " << s.toUtf8().constData() << std::endl;
  }
  static void _log(QString s, int& value) {
    std::cerr << "Mesh | " << s.toUtf8().constData() << value << std::endl;
  }

public:

  static QString HEADER;

private:

  VecInt*        _coordIndex;
  VecFloat*      _coordSrc;
  VecFloat*      _coordDst;
  Graph*         _edges;
  VecInt*        _lines;
  MeshSelection* _selection;
  float          _xV;
  float          _yV;

public:

         Mesh();
        ~Mesh();

  void   erase();

  VecInt*   getCoordIndex();
  VecFloat* getCoordSrc();
  VecFloat* getCoordDst();

  void   copyCoordSrcToDst();
  void   copyCoordDstToSrc();

  void   swapCoord();

  int    getNumberOfVertices();
  int    getNumberOfFaces();

  int    getNumberOfEdges();

  bool   hasEdges();
  void   makeEdges();
  Graph* getEdges();

  bool   hasSelection();
  void   makeSelection();
  MeshSelection* getSelection();
  void   clearSelectionVertex();
  void   clearSelectionFace();
  void   clearSelection();
  void   setSelectedVertex(int iV, bool value);
  void   selectAllVertices();
  void   setSelectedFace(int iF, bool value);
  void   selectAllFaces();
  bool   getSelectedVertex(int iV);
  bool   getSelectedFace(int iF);
  void   toggleSelectedVertex(int iV);
  void   toggleSelectedFace(int iF);
  bool   _isNFaceMesh(int n);
  bool   isTriangleMesh();
  bool   isQuadMesh();

  void   initializeTri();
  void   initializeTriSquare();
  void   initializeTriHexahedron();
  void   initializeQuadSquare();
  void   initializeQuadHexahedron();
  void   subdivide();
  void   selectVertexFromFaceThreshold(int threshold, VecInt& minF, VecInt& maxF);
  void   selectBoundaryVertices();

  int     getNumberOfLines();
  bool    hasLines();
  void    makeLines();
  VecInt* getLines();

  void   addVertices(int n);
  void   addLine(int n);

  void   laplacianSmoothing
    (float lambda, int n, bool smoothSrc, bool smoothDst, bool fixSelected);

private:

  int    _makeVertex(float x0, float x1);
  int    _makeTriangle(int iV0, int iV1, int iV2);
  int    _makeQuad(int iV0, int iV1, int iV2, int iV3);

  void   _computeLaplacian(VecFloat& x, VecFloat& dx, VecFloat& w, bool fixSelected);
};


#endif //_Mesh_hpp_
