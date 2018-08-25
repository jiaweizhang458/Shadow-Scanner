//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-25 21:55:01 taubin>
//------------------------------------------------------------------------
//
// Graph.hpp
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

#ifndef _Graph_hpp_
#define _Graph_hpp_

#include "Vec.hpp"
#include "GraphEdge.hpp"

class Graph {

protected:

 bool   _isOriented;
 bool   _isConst;
 VecInt _edge;
 VecInt _firstEdge;
 VecInt _oneEdge;

public:

  Graph(bool isOriented=false);
  Graph(Graph& src);
  Graph(int nV, bool isOriented);
  Graph(VecInt coordIndex, int nVsrc);

  void reset(int nV, bool isOriented);
  void reset();
  void reset(bool isOriented);
  void swap(Graph& graph);

  bool       isConst();
  bool       isOriented();
  int        getNumberOfVertices();
  int        getNumberOfEdges();
  int        getIndex(int i, int j);
  bool       setIndex(int i, int j, int indx);
  bool       incrIndex(int i, int j);
  GraphEdge* getEdge(int i, int j, GraphEdge* e);
  GraphEdge* getEdge(int i, int j);
  GraphEdge* getInverseEdge(GraphEdge* e);
  bool       hasInverseEdge(GraphEdge* e);
  GraphEdge* _getPosEdge(int pos, GraphEdge* e);
  GraphEdge* getFirstEdge(int i, GraphEdge* e);
  GraphEdge* getFirstEdge(int i);
  GraphEdge* getNextEdge(GraphEdge* e);
  GraphEdge* getOneEdge(int i, GraphEdge* e);
  GraphEdge* getOneEdge(int i);
  void       enumerateEdges();
  void       insertEdge(int i, int j, int indx);
  void       insertEdge(int i, int j);
  void       insertEdge(VecInt& e);
  void       insert(int i, int j, int indx);
  void       insert(int i, int j);
  void       insert(VecInt& e);
  VecInt*    getVertexOrderArray();
  float      computeMeanEdgeLength(float* v);

};

#endif // _Graph_hpp_
