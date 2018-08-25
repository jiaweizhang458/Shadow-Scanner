//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 23:32:09 taubin>
//------------------------------------------------------------------------
//
// GraphFaces.hpp
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


#ifndef _GraphFaces_hpp_
#define _GraphFaces_hpp_

#include "Graph.hpp"

class GraphFaces: public Graph {

private:

  int       _nF;
  VecInt _edgeF;
  VecInt _edgeI;

public:

  GraphFaces(VecInt& coordIndex, int nV);
  
  int     getNumberOfFaces();
  void    insertFace(int nf, VecInt& f);
  void    insertTriangle(int i, int j, int k);
  void    insertTriangle(VecInt& t);
  void    setEdgeFaces(VecInt& edgeF, VecInt& edgeI);
  int     getNumberOfEdgeFaces(int eIndx);
  int     getNumberOfEdgeFaces(GraphEdge& e);
  int     getEdgeFace(int eIndx, int i);
  int     getEdgeFace(int eIndx);
  int     getEdgeFace(GraphEdge& e, int i);
  int     getEdgeFace(GraphEdge& e);
  int     getOtherEdgeFace(int eIndx, int fi);
  int     getOtherEdgeFace(GraphEdge& e, int fi);
  bool    isEdgeFace(int eIndx, int fi);
  bool    isEdgeFace(GraphEdge& e, int fi);
  bool    isUnknownEdge(int eIndx);
  bool    isUnknownEdge(GraphEdge& e);
  bool    isIsolatedEdge(int eIndx);
  bool    isIsolatedEdge(GraphEdge& e);
  bool    isBoundaryEdge(int eIndx);
  bool    isBoundaryEdge(GraphEdge& e);
  bool    isRegularEdge(int eIndx);
  bool    isRegularEdge(GraphEdge& e);
  bool    isManifoldEdge(int eIndx);
  bool    isManifoldEdge(GraphEdge& e);
  bool    isSingularEdge(int eIndx);
  bool    isSingularEdge(GraphEdge& e);
  Graph*  makeDualGraph();
  
};

#endif // _GraphFaces_hpp_
