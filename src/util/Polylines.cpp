//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-30 13:17:15 taubin>
//------------------------------------------------------------------------
//
// Polylines.cpp
//
// Copyright (c) 2016, Gabriel Taubin
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

#include "Polylines.hpp"


Polylines::Polylines() {
}

void      Polylines::reset() {
  _coord.clear();
  _first.clear();
  _next.clear();
}

void      Polylines::swap(Polylines& polylines) {
  _coord.swap(polylines._coord);
  _first.swap(polylines._first);
  _next.swap(polylines._next);
}

VecFloat& Polylines::getCoord() { return _coord; }
VecInt&   Polylines::getFirst() { return _first; }
VecInt&   Polylines::getNext()  { return  _next; }

bool Polylines::resetFirst() {

  // _log(QString("resetFirst() {"));

  bool success = false;

  int nV = _next.size();
  if(nV>0 /* && nV==_coord.size()/2 */) {
    _first.clear();
    
    int iVf,iV0,iV1;

    // _log(QString("  next[%1] = {").arg(nV));
    // convert into doubly linked list
    VecInt prev(nV,-1);
    for(iV0=0;iV0<nV;iV0++) {
      iV1 = _next[iV0];
      // _log(QString("  [%1] = %2").arg(iV0,4).arg(iV1,4));
      if(0<=iV1 && iV1<nV)
        prev[iV1] = iV0;
    }
    // _log(QString("  }"));

    // find first vertices of open paths
    // isolated vertices are included as polylines of length 0
    VecInt visited(nV,0);
    for(iVf=0;iVf<nV;iVf++) {
      if(prev[iVf]<0) {
        _first.append(iVf);
        // traverse the path and mark vertices
        for(iV0=iVf,visited[iV0]=1;(iV1=_next[iV0])>=0;iV0=iV1)
          visited[iV1] = 1;
      }
    }
      
    // int nOpenPaths = _first.size();
    // _log(QString("  nOpenPaths = %1").arg(nOpenPaths));

    // find first vertices of open paths
    for(iVf=0;iVf<nV;iVf++) {
      if(visited[iVf]==0) {
        _first.append(iVf);
        // traverse the path and mark vertices
        for(iV0=iVf,visited[iV0]=1;(iV1=_next[iV0])!=iVf;iV0=iV1)
          visited[iV1] = 1;
      }
    }
      
    // int nClosedPaths = _first.size()-nOpenPaths;
    // _log(QString("  nClosedPaths = %1").arg(nClosedPaths));

  }

  // _log(QString("}"));

  return success;
}

int Polylines::getNumberOfVertices() {
  return _coord.size()/2; // should be equal to _next.size();
}

int Polylines::getNumberOfPolylines() {
  return _first.size();
}

int Polylines::getNumberOfEdges() {
  int nEdges = 0;
  int i,iVfirst,iV0,iV1;
  for(i=0;i<_first.size();i++) {
    iVfirst = _first[i];
    for(iV0=iVfirst;(iV1=_next[iV0])>=0;iV0=iV1) {
      nEdges++;
      if(iV1==iVfirst) break;
    }
  }
  return nEdges;
}

int Polylines::pushBackCoord(float x, float y, int nxt) {
  int iV = _coord.size()/2; // == _next.size()
  _coord.append(x);
  _coord.append(y);
  _next.append(nxt);
  return iV;
}

int Polylines::pushBackFirst(int iV) {
  int retVal = -1;
  if(0<=iV && iV<getNumberOfVertices()) {
    _first.append(iV);
    retVal = iV;
  }
  return retVal;
}

int Polylines::setNext(int iV0, int iV1) {
  int retVal = -1;
  int nV = getNumberOfVertices();
  if(0<=iV0 && iV0<nV && 0<=iV1 && iV1<nV && iV0!=iV1) {
    retVal = _next[iV0] = iV1;
  }
  return retVal;
}

int Polylines::first(int iP) {
  int retVal = -1;
  if(0<=iP && iP<getNumberOfPolylines())
    retVal = _first[iP];
  return retVal;
}

int Polylines::next(int iV) {
  int retVal = -1;
  if(0<=iV && iV<_next.size())
    retVal = _next[iV];
  return retVal;
}

void Polylines::convertToDual() {

  Polylines dual;

  int nP = getNumberOfPolylines();

  int   iP,iVf,iV0,iV1,jVf,jV0,jV1;
  float x,x0,x1,y,y0,y1;

  for(iP=0;iP<nP;iP++) {

    iV0 = iVf = _first[iP];
    x0 = _coord[2*iV0  ];
    y0 = _coord[2*iV0+1];

    if((iV1=_next[iV0])<0) continue;
    x1 = _coord[2*iV1  ];
    y1 = _coord[2*iV1+1];

    x = (x0+x1)/2.0f;
    y = (y0+y1)/2.0f;
    jVf = dual.pushBackCoord(x,y);
    dual._first.append(jVf);

    for(jV0=jVf;(iV1=_next[iV0])>=0;iV0=iV1,jV0=jV1) {
      x0 = x1; y0 = y1;
      x1 = _coord[2*iV1  ];
      y1 = _coord[2*iV1+1];
      x = (x0+x1)/2.0f;
      y = (y0+y1)/2.0f;
      jV1 = dual.pushBackCoord(x,y);
      dual._next[jV0] = jV1;
      if(iV1==iVf) { // close the loop
        dual._next[jV1] = jVf;
        break;
      }
    }
  }

  swap(dual);

}
