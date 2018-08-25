//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-25 21:57:53 taubin>
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

#include <math.h>
#include "Graph.hpp"

Graph::Graph(bool isOriented):
  _isOriented(isOriented),
  _isConst(false),
  _edge(),
  _firstEdge(),
  _oneEdge() {
}

Graph::Graph(Graph& src):
  _isOriented(src._isOriented),
  _isConst(src._isConst),
  _edge(),
  _firstEdge(),
  _oneEdge() {
  _edge.append(src._edge);
  _firstEdge.append(src._firstEdge);
  _oneEdge.append(src._oneEdge);
}

void Graph::reset(int nV, bool isOriented) {
  _isOriented = isOriented;
  _isConst    = false;
  _edge.clear();
  _firstEdge.clear();
  _oneEdge.clear();
  for(int i=0;i<nV;i++) {
    _firstEdge.append(-1);
    _oneEdge.append(-1);
  }
}

void Graph::reset() {
  reset(0,false);
}

void Graph::reset(bool isOriented) {
  reset(0,isOriented);
}

void Graph::swap(Graph& graph) {

  bool   isOriented = graph._isOriented;
  graph._isOriented = _isOriented;
  _isOriented = isOriented;

 bool   isConst = graph._isConst;
 graph._isConst = _isConst;
 _isConst = isConst;

 _edge.swap(graph._edge);
 _firstEdge.swap(graph._firstEdge);
 _oneEdge.swap(graph._oneEdge);
}

Graph::Graph(int nV, bool isOriented):
  _isOriented(isOriented),
  _isConst(false),
  _edge(),
  _firstEdge(),
  _oneEdge() {
  reset(nV,isOriented);
}

Graph::Graph(VecInt coordIndex, int nVsrc):
  _isOriented(false),
  _isConst(false),
  _edge(),
  _firstEdge(),
  _oneEdge() {

  if(coordIndex.size()==0   ) return;
  if(coordIndex.back()>=0) coordIndex.append(-1);

  int i,nfi,fi,j,k;

  int nV = -1;
  int nF =  0;
  for(i=0;i<coordIndex.size();i++)
    if(coordIndex[i]>=0) {
      if(nV<0 || coordIndex[i]>nV) nV = coordIndex[i];
    } else {
      nF++;
    }
  nV++;
  if(nVsrc>nV) nV = nVsrc;

  for(int i=0;i<nV;i++) {
    _firstEdge.append(-1);
    _oneEdge.append(-1);
  }
  
  nfi = 0;
  fi  = -1;
  for(k=0;k<coordIndex.size();k++)
    if(coordIndex[k]>=0) {
      if(k==0 || coordIndex[k-1]<0) {
        fi = k; nfi = 0;
      }
      nfi++;
    } else {
      // end of face
      for(j=0;j<nfi;j++) {
        int h = (j+1)%nfi;
        insertEdge(coordIndex[fi+j],coordIndex[fi+h]);
      }
    }

  enumerateEdges();
}

bool Graph::isConst() { return _isConst; }

bool Graph::isOriented() { return _isOriented; }

int Graph::getNumberOfVertices() { return _firstEdge.size(); }
  
int Graph::getNumberOfEdges() { return _edge.size()/4; }

int Graph::getIndex(int i, int j) {
  int indx = -1;
  int nV = getNumberOfVertices();
  if(0<=i&&i<nV&&0<=j&&j<nV /* &&i!=j */) {
    if(_isOriented==false && j<i) { int k=j; j=i; i=k; }
    if(_firstEdge[i]>=0) {
      for(int h=_firstEdge[i];h>=0;h=_edge[h+3])
        if(_edge[h+1]==j) {
          indx = _edge[h+2];
          break;
        }
    }
  }
  return indx;
}

bool Graph::setIndex(int i, int j, int indx) {
  bool success = false;
  int nV = getNumberOfVertices();
  if(0<=i&&i<nV&&0<=j&&j<nV /* &&i!=j */) {
    if(_isOriented==false && j<i) { int k=j; j=i; i=k; }
    if(_firstEdge[i]>=0) {
      for(int h=_firstEdge[i];h>=0;h=_edge[h+3])
        if(_edge[h+1]==j) {
          _edge[h+2] = indx;
          success = true;
          break;
        }
    }
  }
  return success;
}

bool Graph::incrIndex(int i, int j) {
  bool success = false;
  int nV = getNumberOfVertices();
  if(0<=i&&i<nV&&0<=j&&j<nV /* &&i!=j */) {
    if(_isOriented==false && j<i) { int k=j; j=i; i=k; }
    if(_firstEdge[i]>=0) {
      for(int h=_firstEdge[i];h>=0;h=_edge[h+3])
        if(_edge[h+1]==j) {
          _edge[h+2]++;
          success = true;
          break;
        }
    }
  }
  return success;
}

GraphEdge* Graph::getEdge(int i, int j, GraphEdge* e) {
  int nV = getNumberOfVertices();
  if(0<=i&&i<nV&&0<=j&&j<nV /*&&i!=j*/ ) {
    if(_isOriented==false && j<i) { int k=j; j=i; i=k; }
    if(_firstEdge[i]>=0) {
      int h;
      // for(h=firstEdge(i);h>=0;h=nextEdge()))
      for(h=_firstEdge[i];h>=0;h=_edge[h+3])
        if(_edge[h+1]==j) {
          if(e==(GraphEdge*)0)
            e = new GraphEdge(_edge,h);
          else
            e->set(_edge,h);
          break;
        }
    }
  }
  return e;
}
GraphEdge* Graph::getEdge(int i, int j)
{ return getEdge(i,j,(GraphEdge*)0); }
  
GraphEdge* Graph::getInverseEdge(GraphEdge* e)
{
  GraphEdge* eInv = (_isOriented==false)?e:
    (e!=(GraphEdge*)0)?
    getEdge(e->getVertex(1),
            e->getVertex(0)):(GraphEdge*)0;
  return eInv;
}

bool Graph::hasInverseEdge(GraphEdge* e)
{ return (getInverseEdge(e)==(GraphEdge*)0)?false:true; }

GraphEdge* Graph::_getPosEdge(int pos, GraphEdge* e)
{
  if(pos<0) return (GraphEdge*)0;
  pos = (pos/4)*4;
  if(pos>=_edge.size()) return (GraphEdge*)0;
  if(e==(GraphEdge*)0)
    e = new GraphEdge(_edge,pos);
  else
    e->set(_edge,pos);
  return e;
}

GraphEdge* Graph::getFirstEdge(int i, GraphEdge* e)
{
  int pos = (i<0||i>=_firstEdge.size())?-1:_firstEdge[i];
  return (pos<0)?(GraphEdge*)0:_getPosEdge(pos,e);
}
GraphEdge* Graph::getFirstEdge(int i)
{ return getFirstEdge(i, (GraphEdge*)0); }
  
GraphEdge* Graph::getNextEdge(GraphEdge* e)
{
  int pos = (e==(GraphEdge*)0)?-1:e->getNext();
  return (pos<0)?(GraphEdge*)0:_getPosEdge(pos,e);
}
  
GraphEdge* Graph::getOneEdge(int i, GraphEdge* e)
{
  int pos = (i<0||i>=_oneEdge.size())?-1:_oneEdge[i];
  return (pos<0)?(GraphEdge*)0:_getPosEdge(pos,e);
}
GraphEdge* Graph::getOneEdge(int i)
{ return getOneEdge(i,(GraphEdge*)0); }

void Graph::enumerateEdges() {
  if(_isConst==false) {
      int nV = getNumberOfVertices();
      int pos,indx,i;
      for(indx=0,i=0;i<nV;i++)
        for(pos=_firstEdge[i];
            pos>=0;
            pos=_edge[pos+3])
          { _edge[pos+2] = indx; indx++; }
      _isConst = true;
    }
}
  
void Graph::insertEdge(int i, int j, int indx) {
  GraphEdge* e = getEdge(i,j);
  if(_isConst==false && e==(GraphEdge*)0) {
    if(_isOriented==false && j<i) { int k=j; j=i; i=k; }

    // int pos = _pushBackEdge(i,j,indx);
    int pos = -1;
    {
      int nV = getNumberOfVertices();
      if(0<=i&&0<=j /*&&i!=j*/) {
        if(i>=nV || j>=nV) {
          int n = (((i>j)?i:j)-nV)+1;
          while(--n>=0) {
            _oneEdge.append(-1);
            _firstEdge.append(-1);
          }
        }
        pos = _edge.size();
        _edge.append( i);
        _edge.append( j);
        _edge.append(indx);
        _edge.append(-1);
      }
    }
        
    if(pos<0) return;

    if(_oneEdge[i]<0) _oneEdge[i] = pos;
    if(_oneEdge[j]<0) _oneEdge[j] = pos;

    // lexicographic insert in i-th. list
    int h = _firstEdge[i];
    if(h<0) {
      _firstEdge[i] = pos;
    } else {
      if(j<_edge[h+1]) { // if(j<e_h.getVertex(1))
        // if new edge < first edge (i)
        _edge[pos+3] = h; // e_pos.setNext(h);
        _firstEdge[i] = pos;
      } else {
        while(true) {
          // if new edge goes after last edge (i)
          if(_edge[h+3]<0) { // if(e_h.getNext()<0);
            _edge[h+3] = pos; // e_h.setNext(pos)
            break;
          } else {
            int k = _edge[h+3]; // getNext();
            if(j<_edge[k+1]) { // if(j<e_k.getVertex(1))
              _edge[h+3] = pos; // e_h.setNext(pos);
              _edge[pos+3] = k; // e_pos.setNext(k);
              break;
            } else { // e_h = e_k;
              h = k;
            }
          }
        }
      }
    }
  }
}
void Graph::insertEdge(int i, int j) {
  insertEdge(i,j,-1);
}

void Graph::insertEdge(VecInt& e) {
  int n = e.size()/2;
  for(int i=0;i<n;i++)
    insertEdge(e[2*i],e[2*i+1]);
}

void Graph::insert(int i, int j, int indx) { insertEdge(i, j, indx); }
void Graph::insert(int i, int j) { insertEdge(i, j); }
void Graph::insert(VecInt& e) { insertEdge(e); }

VecInt* Graph::getVertexOrderArray() {
  int i,j;
  int nV = getNumberOfVertices();
  VecInt* v_order = new VecInt(nV,0);
  GraphEdge* e;
  for(i=0;i<nV;i++) {
    for(e=getFirstEdge(i);e!=(GraphEdge*)0;e=getNextEdge(e)) {
      for(j=0;j<2;j++) {
        (*v_order)[e->getVertex(j)]++;
      }
    }
  }
  return v_order;
}

float Graph::computeMeanEdgeLength(float* v) {
  float mEdgeLength = 0.0f;

  GraphEdge* e = (GraphEdge*)0;
  int nE = getNumberOfEdges();
  int nV = getNumberOfVertices();
  for(int j=0;j<nV;j++) {
    for(e=getFirstEdge(j,e);e!=(GraphEdge*)0;e=getNextEdge(e)) {

      // int ie = e->getIndex();
      int i0 = e->getVertex(0); // ==j
      int i1 = e->getVertex(1);
        
      float e2 = 0.0f;
      for(int h=0;h<3;h++)
        e2 += (v[i1*3+h]-v[i0*3+h])*(v[i1*3+h]-v[i0*3+h]);
        
      float e1 = 0.0f;
      if(e2>0.0f)
        e1 = (float)sqrt(e2);
      mEdgeLength += e1;
    }
  }
  mEdgeLength /= (float)nE;
  
  return mEdgeLength;
}
