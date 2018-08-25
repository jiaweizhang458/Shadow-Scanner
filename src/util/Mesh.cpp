//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-28 21:53:49 taubin>
//------------------------------------------------------------------------
//
// Mesh.cpp
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
#include "Mesh.hpp"
#include "MeshSelection.hpp"
#include "Graph.hpp"
#include "GraphTraversal.hpp"
#include <vector>

QString Mesh::HEADER = "#MeshSrcDst V1.0";

Mesh::Mesh():
  _coordIndex((VecInt*)0),
  _coordSrc((VecFloat*)0),
  _coordDst((VecFloat*)0),
  _edges((Graph*)0),
  _lines((VecInt*)0),
  _selection((MeshSelection*)0),
  _xV(0.025f),
  _yV(0.025f) {
  _coordIndex = new VecInt();
  _coordSrc   = new VecFloat();
  _coordDst   = new VecFloat();
}

Mesh::~Mesh() {
  delete _coordIndex;
  delete _coordSrc;
  delete _coordDst;
  delete _edges;
  delete _lines;
  delete _selection;
}

void Mesh::erase() {
  _coordIndex->clear();
  _coordSrc->clear();
  _coordDst->clear();
  delete     _edges;     _edges = (Graph*)0;
  delete     _lines;     _lines = (VecInt*)0;
  delete _selection; _selection = (MeshSelection*)0;
}

VecInt*   Mesh::getCoordIndex() { return _coordIndex; }
VecFloat* Mesh::getCoordSrc()   { return _coordSrc;   }
VecFloat* Mesh::getCoordDst()   { return _coordDst;   }

void Mesh::copyCoordSrcToDst() {
  _coordDst->clear();
  _coordDst->append(*_coordSrc);
}

void Mesh::copyCoordDstToSrc() {
  _coordSrc->clear();
  _coordSrc->append(*_coordDst);
}

void Mesh::swapCoord() {
  _coordSrc->swap(*_coordDst);
}

int Mesh::getNumberOfVertices() {
  // we should have _coordSrc->size()==_coordDst->size()
  return (_coordSrc==(VecFloat*)0)?0:_coordSrc->size()/2;
}

int Mesh::getNumberOfFaces() {
  int nF = 0;
  if(_coordIndex!=(VecInt*)0) {
    int i0,i1;
    for(i0=i1=0;i1<_coordIndex->size();i1++) {
      if((*_coordIndex)[i1]<0) {
        nF++; i0 = i1+1;
      }
    }
  }
  return nF;
}

int Mesh::getNumberOfEdges() {
  if(_edges==(Graph*)0) makeEdges();
  return (_edges!=(Graph*)0)?_edges->getNumberOfEdges():0;
}

bool Mesh::hasEdges() {
  return (_edges!=(Graph*)0);
}
void Mesh::makeEdges() {
  int nV = getNumberOfVertices();
  if(nV>0) _edges = new Graph(*_coordIndex,nV);
}
Graph* Mesh::getEdges() {
  return _edges;
}

bool Mesh::hasSelection() {
  return (_selection!=(MeshSelection*)0);
}
void Mesh::makeSelection() {
  if(_selection==(MeshSelection*)0)
    _selection = new MeshSelection(*this);
  _selection->fit();
}
MeshSelection* Mesh::getSelection() {
  return _selection;
}
void Mesh::clearSelectionVertex() {
  if(_selection==(MeshSelection*)0)
    _selection = new MeshSelection(*this);
  _selection->setSelectedVertices(false);
}
void Mesh::clearSelectionFace() {
  if(_selection==(MeshSelection*)0)
    _selection = new MeshSelection(*this);
  _selection->setSelectedFaces(false);
}
void Mesh::clearSelection() {
  clearSelectionVertex();
  clearSelectionFace();
}
void Mesh::setSelectedVertex(int iV, bool value) {
  _selection->setSelectedVertex(iV, value);
}
void Mesh::selectAllVertices() {
  if(_selection==(MeshSelection*)0)
    _selection = new MeshSelection(*this);
  _selection->setSelectedVertices(true);
}
void Mesh::setSelectedFace(int iF, bool value) {
  _selection->setSelectedFace(iF, value);
}
void Mesh::selectAllFaces() {
  if(_selection==(MeshSelection*)0)
    _selection = new MeshSelection(*this);
  _selection->setSelectedFaces(true);
}
bool Mesh::getSelectedVertex(int iV) {
  return _selection->getSelectedVertex(iV);
}
bool Mesh::getSelectedFace(int iF) {
  return _selection->getSelectedFace(iF);
}
void Mesh::toggleSelectedVertex(int iV) {
  _selection->toggleSelectedVertex(iV);
}
void Mesh::toggleSelectedFace(int iF) {
  _selection->toggleSelectedFace(iF);
}

bool Mesh::_isNFaceMesh(int n) {
  bool value = false;
  if(_coordIndex!=(VecInt*)0) {
    value = true;
    int i0,i1;
    for(i0=i1=0;i1<_coordIndex->size();i1++) {
      if((*_coordIndex)[i1]<0) {
        if(i1-i0!=n) { value = false; break; }
        i0 = i1+1;
      }
    }
  }
  return value;
}

bool Mesh::isTriangleMesh() {
  return _isNFaceMesh(3);
}

bool Mesh::isQuadMesh() {
  return _isNFaceMesh(4);
}

int Mesh::_makeVertex(float x0, float x1) {
  int iV = _coordSrc->size()/2;
  _coordSrc->append(x0);
  _coordSrc->append(x1);
  _coordDst->append(x0);
  _coordDst->append(x1);
  return iV;
}
int Mesh::_makeTriangle(int iV0, int iV1, int iV2) {
  int iT = _coordIndex->size()/4;
  _coordIndex->append(iV0);
  _coordIndex->append(iV1);
  _coordIndex->append(iV2);
  _coordIndex->append(-1);
  return iT;
}
int Mesh::_makeQuad(int iV0, int iV1, int iV2, int iV3) {
  int iT = _coordIndex->size()/4;
  _coordIndex->append(iV0);
  _coordIndex->append(iV1);
  _coordIndex->append(iV2);
  _coordIndex->append(iV3);
  _coordIndex->append(-1);
  return iT;
}

void Mesh::initializeTri() {
  erase();
  //    0
  //  /   \
  // 1 --- 2
  int iV0 = _makeVertex(0.5f,0.1f);
  int iV1 = _makeVertex(0.1f,0.9f);
  int iV2 = _makeVertex(0.9f,0.9f);
  /* int iT0 = */ _makeTriangle(iV0,iV1,iV2);
  makeEdges();
  makeLines();
  makeSelection();
}

void Mesh::initializeTriSquare() {
  erase();
  // 1 --- 2
  // |  0  |
  // 4 --- 3
  int iV0 = _makeVertex(0.5f,0.5f);
  int iV1 = _makeVertex(0.0f,0.0f);
  int iV2 = _makeVertex(1.0f,0.0f);
  int iV3 = _makeVertex(1.0f,1.0f);
  int iV4 = _makeVertex(0.0f,1.0f);
  /* int iT0 = */ _makeTriangle(iV0,iV1,iV2);
  /* int iT1 = */ _makeTriangle(iV0,iV2,iV3);
  /* int iT2 = */ _makeTriangle(iV0,iV3,iV4);
  /* int iT3 = */ _makeTriangle(iV0,iV4,iV1);
  makeEdges();
  makeLines();
  makeSelection();
}

void Mesh::initializeTriHexahedron() {
  erase();
  float L = 0.80f;
  float H = L*(float)(sqrt(0.75));

  int iV0 = _makeVertex(0.5f,0.5f);
  int iV1 = _makeVertex(0.5f+L     ,0.5f);
  int iV2 = _makeVertex(0.5f+L/2.0f,0.5f+H);
  int iV3 = _makeVertex(0.5f-L/2.0f,0.5f+H);
  int iV4 = _makeVertex(0.5f-L     ,0.5f);
  int iV5 = _makeVertex(0.5f-L/2.0f,0.5f-H);
  int iV6 = _makeVertex(0.5f+L/2.0f,0.5f-H);

  /* int iT0 = */ _makeTriangle(iV0,iV1,iV2);
  /* int iT1 = */ _makeTriangle(iV0,iV2,iV3);
  /* int iT2 = */ _makeTriangle(iV0,iV3,iV4);
  /* int iT3 = */ _makeTriangle(iV0,iV4,iV5);
  /* int iT4 = */ _makeTriangle(iV0,iV5,iV6);
  /* int iT5 = */ _makeTriangle(iV0,iV6,iV1);

  makeEdges();
  makeLines();
  makeSelection();
}

void Mesh::initializeQuadSquare() {
  erase();
  // 1 --- 2
  // |     |
  // 0 --- 3
  int iV0 = _makeVertex(0.0f,1.0f);
  int iV1 = _makeVertex(0.0f,0.0f);
  int iV2 = _makeVertex(1.0f,0.0f);
  int iV3 = _makeVertex(1.0f,1.0f);
  /* int iQ0 = */ _makeQuad(iV0,iV1,iV2,iV3);
  makeEdges();
  makeLines();
  makeSelection();
}

void Mesh::initializeQuadHexahedron() {
  erase();
  float L = 0.80f;
  float H = L*(float)(sqrt(0.75));
  int iV0 = _makeVertex(0.5f,0.5f);
  int iV1 = _makeVertex(0.5f+L     ,0.5f);
  int iV2 = _makeVertex(0.5f+L/2.0f,0.5f+H);
  int iV3 = _makeVertex(0.5f-L/2.0f,0.5f+H);
  int iV4 = _makeVertex(0.5f-L     ,0.5f);
  int iV5 = _makeVertex(0.5f-L/2.0f,0.5f-H);
  int iV6 = _makeVertex(0.5f+L/2.0f,0.5f-H);
  /* int iQ0 = */ _makeQuad(iV0,iV1,iV2,iV3);
  /* int iQ1 = */ _makeQuad(iV0,iV3,iV4,iV5);
  /* int iQ2 = */ _makeQuad(iV0,iV5,iV6,iV1);
  makeEdges();
  makeLines();
  makeSelection();
}

void Mesh::subdivide() {
  _log("subdivide {");

  if(_selection==(MeshSelection*)0) {
    _log("  _selection==null");
  } else if(isTriangleMesh()==false) {
    _log("  isTriangleMesh()==false");
  } else {

    if(_edges==(Graph*)0) makeEdges();
    int nV = getNumberOfVertices();
    int nT = getNumberOfFaces();
    int nE = getNumberOfEdges();
	//int eV[nE];
	std::vector<int> eV(nE);

    _log("  before {");
    _log("    nV = ",nV);
    _log("    nE = ",nE);
    _log("    nT = ",nT);
    _log("  }");

    int   iV0,iV1,iV2,iE,iT,code,iV01,iV12,iV20,i;
    float x0,x1;

    // create new vertices
    GraphTraversal gt(*_edges);
    GraphEdge* e;
    gt.start();
    while((e=gt.next())!=(GraphEdge*)0) {
      iE  = e->getIndex();
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      if(getSelectedVertex(iV0) && getSelectedVertex(iV1)) {
        eV[iE] = _coordSrc->size()/2;
        for(i=0;i<2;i++) {
          x0 = (*_coordSrc)[2*iV0+i];
          x1 = (*_coordSrc)[2*iV1+i];
          _coordSrc->append((x0+x1)/2.0f);
          x0 = (*_coordDst)[2*iV0+i];
          x1 = (*_coordDst)[2*iV1+i];
          _coordDst->append((x0+x1)/2.0f);
        }
      } else {
        eV[iE] = -1;
      }
    }

    // split faces
    for(iT=0;iT<nT;iT++) {

      code = 0x0;
      iV0 = (*_coordIndex)[4*iT  ];
      if(_selection->getSelectedVertex(iV0)) code |= 0x1;
      iV1 = (*_coordIndex)[4*iT+1];
      if(_selection->getSelectedVertex(iV1)) code |= 0x2;
      iV2 = (*_coordIndex)[4*iT+2];
      if(_selection->getSelectedVertex(iV2)) code |= 0x4;
        
      switch(code) {

      case 3:
        //     2
        //    / \
        // 0=X---X=1
        e = _edges->getEdge(iV0,iV1);
        iV01 = eV[e->getIndex()];
        (*_coordIndex)[4*iT+1] = iV01;
        _coordIndex->append(iV01);
        _coordIndex->append(iV1);
        _coordIndex->append(iV2);
        _coordIndex->append(-1);
        break;

      case 5:
        //   2=X
        //    / \
        // 0=X---1
        e = _edges->getEdge(iV2,iV0);
        iV20 = eV[e->getIndex()];
        (*_coordIndex)[4*iT+2] = iV20;
        _coordIndex->append(iV20);
        _coordIndex->append(iV1);
        _coordIndex->append(iV2);
        _coordIndex->append(-1);
        break;

      case 6:
        //   2=X
        //    / \
        //   0---X=1
        e = _edges->getEdge(iV1,iV2);
        iV12 = eV[e->getIndex()];
        (*_coordIndex)[4*iT+2] = iV12;
        _coordIndex->append(iV0);
        _coordIndex->append(iV12);
        _coordIndex->append(iV2);
        _coordIndex->append(-1);
        break;

      case 7:
        //   2=X
        //    / \
        // 0=X---X=1
        e = _edges->getEdge(iV0,iV1);
        iV01 = eV[e->getIndex()];
        e = _edges->getEdge(iV1,iV2);
        iV12 = eV[e->getIndex()];
        e = _edges->getEdge(iV2,iV0);
        iV20 = eV[e->getIndex()];
        (*_coordIndex)[4*iT+0] = iV01;
        (*_coordIndex)[4*iT+1] = iV12;
        (*_coordIndex)[4*iT+2] = iV20;
        _coordIndex->append(iV0);
        _coordIndex->append(iV01);
        _coordIndex->append(iV20);
        _coordIndex->append(-1);
        _coordIndex->append(iV1);
        _coordIndex->append(iV12);
        _coordIndex->append(iV01);
        _coordIndex->append(-1);
        _coordIndex->append(iV2);
        _coordIndex->append(iV20);
        _coordIndex->append(iV12);
        _coordIndex->append(-1);
        break;

      default:
        break;
      }
    }

    makeEdges();
    makeLines();
    makeSelection();

    nV = getNumberOfVertices();
    nT = getNumberOfFaces();
    nE = getNumberOfEdges();

    _log("  after {");
    _log("    nV = ",nV);
    _log("    nE = ",nE);
    _log("    nT = ",nT);
    _log("  }");

  }

  _log("}");
}

void Mesh::selectVertexFromFaceThreshold
(int threshold, VecInt& minF, VecInt& maxF) {
  int nF = getNumberOfFaces();
  if(minF.size()>=nF && maxF.size()>=nF) {
    clearSelectionVertex();
    // determine faces for which the difference is above the threshold
    int i0,i1,i,iF,iV,diff;
    for(iF=i0=i1=0;i1<_coordIndex->size();i1++) {
      if((*_coordIndex)[i1]<0) {
        // face iF;
        if((diff=maxF[iF]-minF[iF])>=threshold) {
          // if(minF.get(iF)<threshold) {
          for(i=i0;i<i1;i++) {
            iV = (*_coordIndex)[i];
            setSelectedVertex(iV,true);
          }
        }
        iF++; i0 = i1+1;
      }
    }
  }
}

void Mesh::selectBoundaryVertices() {

  // int nV = getNumberOfVertices();
  int nE = getNumberOfEdges();
  //int nFaces[nE];
  std::vector<int> nFaces(nE);
  int i0,i1,i,iF,iV0,iV1,iE;
  for(iE=0;iE<nE;iE++)
    nFaces[iE] = 0;
  GraphEdge* e;
  for(iF=i0=i1=0;i1<_coordIndex->size();i1++) {
    if((*_coordIndex)[i1]<0) {
      // face iF;
      iV0 = (*_coordIndex)[i1-1];
      for(i=i0;i<i1;i++) {
        iV1 = (*_coordIndex)[i];
        e = _edges->getEdge(iV0,iV1);
        iE = e->getIndex();
        nFaces[iE]++;
        iV0 = iV1;
      }
      iF++; i0 = i1+1;
    }
  }

  GraphTraversal gt(*_edges);
    
  gt.start();
  while((e=gt.next())!=(GraphEdge*)0) {
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);
    iE = e->getIndex();
  }
  gt.start();
  while((e=gt.next())!=(GraphEdge*)0) {
    iE = e->getIndex();
    if(nFaces[iE]==1) { // is boundary edge
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      setSelectedVertex(e->getVertex(0),true);
      setSelectedVertex(e->getVertex(1),true);
    }
  }
}

void Mesh::_computeLaplacian
(VecFloat& x, VecFloat& dx, VecFloat& w, bool fixSelected) {

  int i,iV,iV0,iV1;
  int nV = getNumberOfVertices();
  // int nE = getNumberOfEdges();
  float dx0e,dx1e,we,wv,tmp;

  GraphTraversal gt(*_edges);
  GraphEdge* e;

  // initialize
  dx.clear();
    for(i=0;i<2*nV;i++)dx.append(0.0f);
  w.clear();
    for(i=0;i<nV;i++)w.append(0.0f);
  // accumulate
  gt.start();
  while((e=gt.next())!=(GraphEdge*)0) {
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);
    dx0e = x[2*iV1+0]-x[2*iV0+0];
    dx1e = x[2*iV1+1]-x[2*iV0+1];
    we   = 1.0f;
    // dx[iV0] += dxe;
    tmp = dx[2*iV0+0]+dx0e; dx[2*iV0+0] = tmp;
    tmp = dx[2*iV0+1]+dx1e; dx[2*iV0+1] = tmp;
    // w[iV0]  +=  we;
    tmp =        w[iV0]+we;      w[iV0] = tmp;
    // dx[iV1] -= dxe;
    tmp = dx[2*iV1+0]-dx0e; dx[2*iV1+0] = tmp;
    tmp = dx[2*iV1+1]-dx1e; dx[2*iV1+1] = tmp;
    // w[iV1]  +=  we;
    tmp =        w[iV1]+we;      w[iV1] = tmp;
  }
  // normalize
  for(iV=0;iV<nV;iV++)
    if((wv=w[iV])>0) {
      tmp = dx[2*iV+0]/wv; dx[2*iV+0] = tmp;
      tmp = dx[2*iV+1]/wv; dx[2*iV+1] = tmp;
    }
}

void Mesh::laplacianSmoothing
(float lambda, int n,
 bool smoothSrc, bool smoothDst, bool fixSelected) {
  _log("laplacianSmoothing() {");

  if((smoothSrc||smoothDst)==false) return;
  int      i,iV;
  float    x0,x1;

  int      nV = getNumberOfVertices();
  _log("  nV = ",nV);


  VecFloat dx(2*nV,0.0f);
  VecFloat wx(nV,0.0f);

  if(smoothSrc) {
    VecFloat& x = *_coordSrc;
    _computeLaplacian(x,dx,wx,fixSelected);
    for(iV=0;iV<nV;iV++)
      if(fixSelected==false || getSelectedVertex(iV)==false) {
        i=2*iV; x0 = x[i]+lambda*dx[i]; x[i] = x0;
        i++;    x1 = x[i]+lambda*dx[i]; x[i] = x1;
      }
  }

  if(smoothDst) {
    VecFloat& x = *_coordDst;
    _computeLaplacian(x,dx,wx,fixSelected);
    for(iV=0;iV<nV;iV++)
      if(fixSelected==false || getSelectedVertex(iV)==false) {
        i=2*iV; x0 = x[i]+lambda*dx[i]; x[i] = x0;
        i++;    x1 = x[i]+lambda*dx[i]; x[i] = x1;
      }
  }

  _log("}");
}

//////////////////////////////////////////////////////////////////////
// polylines

int Mesh::getNumberOfLines() {
  int nLines = 0;
  if(_lines==(VecInt*)0) makeLines();
  if(_lines!=(VecInt*)0) {
    for(int i=0;i<_lines->size();i++)
      if((*_lines)[i]<0)
        nLines++;
  }
  return nLines;
}

bool Mesh::hasLines() {
  return (_lines!=(VecInt*)0);
}
void Mesh::makeLines() {
  if(_lines==(VecInt*)0) _lines = new VecInt();
  _lines->clear();
  if(_edges!=(Graph*)0) {
    int nV = getNumberOfVertices();
	//int order[nV];
	std::vector<int> order(nV);
    //int link[2*nV];
	std::vector<int> link(2 * nV);
	int iV,iV0,iV1,iV2;
    for(iV=0;iV<nV;iV++) {
      order[iV] = 0; link[2*iV  ] = link[2*iV+1] = -1;
    }
    GraphEdge* e;
    GraphTraversal gt(*_edges);
    gt.start();
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      order[iV0]++;
      order[iV1]++;
      if(link[2*iV0  ]<0)
        link[2*iV0  ] = iV1;
      else if(link[2*iV0+1]<0)
        link[2*iV0+1] = iV1;
      if(link[2*iV1  ]<0)
        link[2*iV1  ] = iV0;
      else if(link[2*iV1+1]<0)
        link[2*iV1+1] = iV0;
    }
    for(iV=0;iV<nV;iV++) {
      if(order[iV]==1) {
        _lines->append(iV);
        order[iV] = 0;
        iV0 = iV;
        iV1 = link[2*iV0]; link[2*iV0] = -1;
        _lines->append(iV1);
        order[iV1]--;
        while(order[iV1]==1) {
          order[iV1]--;
          iV2 = (link[2*iV1  ]==iV0)?link[2*iV1+1]:link[2*iV1];
          _lines->append(iV2);
          order[iV2]--;
          iV0 = iV1; iV1 = iV2;
        }
        _lines->append(-1);
      }
    }
  }
}
VecInt* Mesh::getLines() {
  return _lines;
}

void Mesh::addVertices(int n) {
  // int nV = getNumberOfVertices();
  for(int i=0;i<n;i++) {
      _coordSrc->append(_xV);
      _coordSrc->append(_yV);
      _coordDst->append(_xV);
      _coordDst->append(_yV);
    if((_xV+=0.025f)>=0.975f) {
      _xV = 0.025f;
      if((_yV+=0.025f)>=0.975f) {
        _yV = 0.025f;
      }
    }
  }
}

void Mesh::addLine(int n) {

}
