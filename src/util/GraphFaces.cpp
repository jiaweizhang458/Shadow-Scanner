//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-28 00:12:45 taubin>
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

#include "GraphFaces.hpp"

// public:

GraphFaces::GraphFaces(VecInt& coordIndex, int nV):
  Graph(coordIndex,nV),
  _nF(0),
  _edgeF(),
  _edgeI() {

  if(coordIndex.size()>0) {

    int i,nfi,fi,j,k;
    int nE = getNumberOfEdges();
    _edgeF.reserve(nE+1);
    _edgeF.insert(0,nE+1,0);

    // count number of incident faces
    nfi = 0;
    fi  = -1;
    for(k=0;k<coordIndex.size();k++) {
      if(coordIndex[k]>=0) {
        if(k==0 || coordIndex[k-1]<0)
          { fi = k; nfi = 0; }
        nfi++;
      } else {
        _nF++;
        for(j=0;j<nfi;j++) {
          int h = (j+1)%nfi;
          int fij = coordIndex[fi+j];
          int fih = coordIndex[fi+h];
          GraphEdge* e = getEdge(fij,fih);
          if(e!=(GraphEdge*)0) {
            int ie = e->getIndex();
            _edgeF[ie+1]++;
          }
        }
      }
    }
      
    for(i=0;i<nE;i++) {
      _edgeF[i+1]+=_edgeF[i];
    }
    int nEF = _edgeF[nE];
    _edgeI.insert(0,nEF,-1);
    VecInt nFe(nE,0);
    nfi = 0;
    fi  = -1;
    for(i=k=0;k<coordIndex.size();k++)
      if(coordIndex[k]>=0) {
        if(k==0 || coordIndex[k-1]<0)
          { fi = k; nfi = 0; }
        nfi++;
      } else {  // end of face
        for(j=0;j<nfi;j++) {
          int h = (j+1)%nfi;
          int fij = coordIndex[fi+j];
          int fih = coordIndex[fi+h];
          GraphEdge* e = getEdge(fij,fih);
          if(e!=(GraphEdge*)0) {
            int  ie = e->getIndex();
            _edgeI[_edgeF[ie]+nFe[ie]]=i;
            nFe[ie]++;
          }
        }
        i++;
      }
  }
}
  
int GraphFaces::getNumberOfFaces()
{ return _nF; }

void GraphFaces::insertFace(int nf, VecInt& f) {
  if(_isConst==false && nf>0) {
    for(int i=0;i<nf;i++) {
      int j = (i+1)%nf;
      insertEdge(f[i],f[j]);
    }
    // update _edgeF and _edgeI
  }
}
  
void GraphFaces::insertTriangle(int i, int j, int k) {
  if(_isConst==false) {
    insertEdge(i,j);
    insertEdge(j,k);
    insertEdge(k,i);
  }
  // update _edgeF and _edgeI
}

void GraphFaces::insertTriangle(VecInt& t) {
  if(t.size()>=3)
    insertTriangle(t[0],t[1],t[2]);
}

void GraphFaces::setEdgeFaces(VecInt& edgeF, VecInt& edgeI)
{ _edgeF = edgeF; _edgeI = edgeI; }

int GraphFaces::getNumberOfEdgeFaces(int eIndx)
{ return _edgeF[eIndx+1]-_edgeF[eIndx]; }

int GraphFaces::getNumberOfEdgeFaces(GraphEdge& e)
{ return getNumberOfEdgeFaces(e.getIndex()); }

int GraphFaces::getEdgeFace(int eIndx, int i)
{ return _edgeI[_edgeF[eIndx]+i]; }

int GraphFaces::getEdgeFace(int eIndx) {
  int fi = -1;
  int nf = getNumberOfEdgeFaces(eIndx);
  if(nf==1) nf = 2;
  for(int i=0;i<nf;i++)
    if(_edgeI[_edgeF[eIndx]+i]>=0)
      { fi = _edgeI[_edgeF[eIndx]+i]; break; }
  return fi;
}

int GraphFaces::getEdgeFace(GraphEdge& e, int i)
{ return getEdgeFace(e.getIndex(),i); }

int GraphFaces::getEdgeFace(GraphEdge& e)
{ return getEdgeFace(e.getIndex()); }

int GraphFaces::getOtherEdgeFace(int eIndx, int fi) {
  int fOther = -1;
  int nFe    = _edgeF[eIndx+1]-_edgeF[eIndx];
  if(nFe>1) {
    int j;
    for(j=0;j<nFe;j++)
      if(_edgeI[_edgeF[eIndx]+j]==fi) {
        fOther = (j>0)?_edgeI[_edgeF[eIndx]]:
          _edgeI[_edgeF[eIndx]+1];
        break;
      }
  }
  return fOther;
}
  
int GraphFaces::getOtherEdgeFace(GraphEdge& e, int fi)
{ return getOtherEdgeFace(e.getIndex(),fi); }

bool GraphFaces::isEdgeFace(int eIndx, int fi) {
  bool is_edge_face = false;
  for(int j=_edgeF[eIndx];j<_edgeF[eIndx+1];j++)
    if(_edgeI[j]==fi)
      { is_edge_face=true; break; }
  return is_edge_face;
}

bool GraphFaces::isEdgeFace(GraphEdge& e, int fi)
{ return isEdgeFace(e.getIndex(),fi); }

bool GraphFaces::isUnknownEdge(int eIndx)
{ return (getNumberOfEdgeFaces(eIndx) <0)?true:false; }

bool GraphFaces::isUnknownEdge(GraphEdge& e)
{ return isUnknownEdge(e.getIndex()); }

bool GraphFaces::isIsolatedEdge(int eIndx)
{ return (getNumberOfEdgeFaces(eIndx)==0)?true:false; }

bool GraphFaces::isIsolatedEdge(GraphEdge& e)
{ return isIsolatedEdge(e.getIndex()); }

bool GraphFaces::isBoundaryEdge(int eIndx)
{ return (getNumberOfEdgeFaces(eIndx)==1)?true:false; }

bool GraphFaces::isBoundaryEdge(GraphEdge& e)
{ return isBoundaryEdge(e.getIndex()); }

bool GraphFaces::isRegularEdge(int eIndx)
{ return (getNumberOfEdgeFaces(eIndx)==2)?true:false; }

bool GraphFaces::isRegularEdge(GraphEdge& e)
{ return isRegularEdge(e.getIndex()); }

bool GraphFaces::isManifoldEdge(int eIndx)
{ return ((getNumberOfEdgeFaces(eIndx)==1)||
          (getNumberOfEdgeFaces(eIndx)==2))?true:false; }

bool GraphFaces::isManifoldEdge(GraphEdge& e)
{ return isManifoldEdge(e.getIndex()); }

bool GraphFaces::isSingularEdge(int eIndx)
{ return (getNumberOfEdgeFaces(eIndx)>=3)?true:false; }

bool GraphFaces::isSingularEdge(GraphEdge& e)
{ return isSingularEdge(e.getIndex()); }

Graph* GraphFaces:: makeDualGraph() {
  int nF = getNumberOfFaces();
  Graph* dualGraph = new Graph(nF,false);
  int i;
  GraphEdge* e;
  for(i=0;i<nF;i++)
    for(e=getFirstEdge(i);e!=(GraphEdge*)0;e=getNextEdge(e))
      if(isRegularEdge(*e)==true) {
        int f0 = getEdgeFace(*e,0);
        int f1 = getEdgeFace(*e,1);
        dualGraph->insertEdge(f0,f1);
      }
  return dualGraph;
}
