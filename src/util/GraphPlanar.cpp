//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-30 13:18:41 taubin>
//------------------------------------------------------------------------
//
// GraphPlanar.cpp
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
#include <QFile>
#include <QTextStream>
#include "GraphPlanar.hpp"
#include "GraphTraversal.hpp"
#include "Polylines.hpp"
#include <vector>

QString GraphPlanar::HEADER = "#GraphPlanar V1.0";

GraphPlanar::GraphPlanar(bool oriented):
  Graph(oriented),
  _coord(),
  _edgeColor(),
  _vertexColor() {
}

GraphPlanar::GraphPlanar(Polylines& polylines):
  Graph(false),
  _coord(),
  _edgeColor(),
  _vertexColor() {

  VecFloat& coord = polylines.getCoord();
  VecInt&   first = polylines.getFirst();
  VecInt&   next  = polylines.getNext();

  int nPolylines = first.size(); // polylines.getNumberOfPolylines();

  int iP,iVf,iV0,iV1;

  _coord << coord;

  for(iP=0;iP<nPolylines;iP++) {
    iVf = first[iP]; // polylines.first(iP);
    // for(iV0=iVf;(iV1=polylines.next(iV0))>=0;iV0=iV1) {
    for(iV0=iVf;(iV1=next[iV0])>=0;iV0=iV1) {
      insertEdge(iV0,iV1);
      if(iV1==iVf) break;
    }
  }
}

void GraphPlanar::reset(bool oriented) {
  Graph::reset(oriented);
  _coord.clear();
}

void GraphPlanar::reset() {
  reset(false);
}

VecFloat& GraphPlanar::getCoord() {
  return _coord;
}

int GraphPlanar::getNumberOfCoord() {
  return _coord.size()/2;
}
void GraphPlanar::pushBackCoord(float x) {
  _coord.append(x);
}

int GraphPlanar::pushBackCoord(float x, float y) {
  int indx = _coord.size()/2;
  _coord.append(x);
  _coord.append(y);
  return indx;
}

void GraphPlanar::pushBackCoord(float x, float y, int n) {
  for(int i=n;i>0;i--) {
    _coord.append(x);
    _coord.append(y);
  }
}

int GraphPlanar::setCoord(int i, float x, float y) {
  if(0<=i && 2*i+1<_coord.size()) {
    _coord[2*i  ] = x;
    _coord[2*i+1] = y;
  } else {
    i = -1;
  }
  return i;
}

void GraphPlanar::setEdgeColor(VecInt eColor) {
  _edgeColor = eColor;
}
VecInt& GraphPlanar::getEdgeColor() {
  return _edgeColor;
}

void GraphPlanar::setVertexColor(VecInt eColor) {
  _vertexColor = eColor;
}
VecInt& GraphPlanar::getVertexColor() {
  return _vertexColor;
}

void GraphPlanar::lowPass(float lambda, float kappa, int nSmooth) {
  float lambda0 = lambda;
  float lambda1 = 1.0f/(kappa-1.0f/lambda);
  laplacianSmooth(lambda0,lambda1,nSmooth);
}

//////////////////////////////////////////////////////////////////////
void GraphPlanar::laplacianSmooth
(float lambda0, float lambda1, int nSmooth, VecInt* valence) {

  // _log("laplacianSmooth() {");
  // _log("  lambda0 = "+lambda0);
  // _log("  lambda1 = "+lambda1);
  // _log("  nSmooth = "+nSmooth);

  int         nV     = getNumberOfCoord();
  VecFloat&   coord  = getCoord();
  //float dCoord[2*nV];
  //float wCoord[nV];
  std::vector<float> dCoord(2 * nV);
  std::vector<float> wCoord(nV);
  float       dx0,dx1,x0,x1,w,lambda;
  int         step,iV,iV0,iV1;
  GraphEdge*   e;
  bool        withValenceConstraints = (valence!=(VecInt*)0 && valence->size()==nV);
  GraphTraversal t(*this);

  // _log("  withConstraints = "+withValenceConstraints);

  for(step=0;step<nSmooth;step++) {
    // acumulate displacement
      for(int i=0;i<2*nV;i++) dCoord[i] = 0.0f;
      for(int i=0;i<  nV;i++) wCoord[i] = 0.0f;
      
    t.start();
    while((e=t.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      dx0 = coord[2*iV1  ]-coord[2*iV0  ];
      dx1 = coord[2*iV1+1]-coord[2*iV0+1];
      w   = 1.0f;
      if(!withValenceConstraints || (*valence)[iV0]==2) {
        dCoord[2*iV0  ] += w*dx0;
        dCoord[2*iV0+1] += w*dx1;
        wCoord[iV0]     += w    ;
      }
      if(!withValenceConstraints || (*valence)[iV1]==2) {
        dCoord[2*iV1  ] -= w*dx0;
        dCoord[2*iV1+1] -= w*dx1;
        wCoord[iV1]     += w    ;
      }
    }
    // normalize and apply displacement
    lambda = (step%2==0)?lambda0:lambda1;
    for(iV=0;iV<nV;iV++)
      if((w=wCoord[iV])>0.0f) {
        dx0 = lambda*(dCoord[2*iV  ]/w);
        dx1 = lambda*(dCoord[2*iV+1]/w);
        x0 = coord[2*iV  ]+dx0;
        x1 = coord[2*iV+1]+dx1;
        coord[2*iV  ] = x0;
        coord[2*iV+1] = x1;
      }
  }

  // _log("}");
}

//////////////////////////////////////////////////////////////////////
void GraphPlanar::softConstrainedSmooth
(float descentStep, float smoothing, int steps) {

  // _log("softConstrainedSmooth() {");

  if(descentStep<0.0f)
    descentStep = 0.0f;
  else if(descentStep>1.0f)
    descentStep =1.0f;
  if(smoothing<0.0f)
    smoothing = 0.0f;
  else
    if(smoothing>1.0f)
      smoothing =1.0f;

  // _log("  descentStep  = "+descentStep);   // 0<descentStep<1
  // _log("  smoothing    = "+smoothing);      // 0<=smoothing<=1
  // _log("  steps        = "+steps); // steps>=0

  int         nV     = getNumberOfCoord();
  VecFloat    coord0 = getCoord();
  //float coord[2*nV];
  //float dCoord[2*nV];
  //float wCoord[nV];
  //float eCoord[nV];
  std::vector<float> coord(2 * nV);
  std::vector<float> dCoord(2 * nV);
  std::vector<float> wCoord(nV);
  std::vector<float> eCoord(nV);
  float       dx0,dx1,w,wMin,wMax;
  int         step,iV,iV0,iV1,nConstrained;

  GraphEdge*  e;
  GraphTraversal t(*this);
    
  for(iV=0;iV<nV;iV++) {
    coord[2*iV  ] = coord0[2*iV  ];
    coord[2*iV+1] = coord0[2*iV+1];
  }

  for(step=0;step<steps;step++) {

    // initialize displacements from soft constraints
    wMin = wMax = 0.0f;
    for(iV=0;iV<nV;iV++) {
      dx0 = coord0[2*iV  ]-coord[2*iV  ];
      dx1 = coord0[2*iV+1]-coord[2*iV+1];
      w = (float)sqrt(dx0*dx0+dx1*dx1);
      if(wMax==0) {
        wMin = wMax = w;
      } else {
        if(w>wMax) wMax = w;
        if(w<wMin) wMin = w;
      }
      eCoord[iV] = w;
      w = 1.0f-smoothing;
      dCoord[2*iV  ] = w*dx0;
      dCoord[2*iV+1] = w*dx1;
      wCoord[iV]     = w; // 1.0f;
    }

    if(wMax>wMin) {
      // reset vertex position to initial position
      nConstrained = 0;
      for(iV=0;iV<nV;iV++) {
        if((wMax-eCoord[iV])/(wMax-wMin)<0.1f) {
          coord[2*iV  ] = coord0[2*iV  ];
          coord[2*iV+1] = coord0[2*iV+1];
          nConstrained++;
        }
      }
    }

    // accumulate smoothing term
    t.start();
    while((e=t.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      dx0 = coord[2*iV1  ]-coord[2*iV0  ];
      dx1 = coord[2*iV1+1]-coord[2*iV0+1];
      w   = smoothing;
      dCoord[2*iV0  ] += w*dx0;
      dCoord[2*iV0+1] += w*dx1;
      wCoord[iV0]     += w    ;
      dCoord[2*iV1  ] -= w*dx0;
      dCoord[2*iV1+1] -= w*dx1;
      wCoord[iV1]     += w    ;
    }

    for(iV=0;iV<nV;iV++)
      if((w=wCoord[iV])>0.0f) {
        coord[2*iV  ] += descentStep*(dCoord[2*iV  ]/w);
        coord[2*iV+1] += descentStep*(dCoord[2*iV+1]/w);
      }
  }
    
  for(iV=0;iV<nV;iV++) {
    coord0[2*iV  ] = coord[2*iV  ];
    coord0[2*iV+1] = coord[2*iV+1];
  }

  // _log("}");
}

//////////////////////////////////////////////////////////////////////
void GraphPlanar::anchoredSmooth(float lambda, int steps) {
  _log("anchoredSmooth() {");

  if(lambda<0.0f)
    lambda = 0.0f;
  else if(lambda>1.0f)
    lambda =1.0f;

  if(steps<=1) steps = 1;

  // _log(QString("  lambda = %1").arg(lambda));
  // _log(QString("  steps  = %1").arg(steps));

  int         nV     = getNumberOfCoord();
  int         nE     = getNumberOfEdges();

  VecFloat&   coord  = _coord; // just a new name 
  VecFloat    coord0; // make a copy
  coord0.append(_coord);

  VecFloat    dCoord(2*nV,0.0f); // displacement vectors
  VecFloat    wCoord(nV,0.0f);   // weights

  float d1x0,d1x1,d2x0,d2x1,w,a,b,descentStep;

  int step,iV,iV0,iV1;

  GraphEdge*     e;
  GraphTraversal gt(*this);

  for(step=0;step<steps;step++) {

    // zero accumulators
    for(iV=0;iV<nV;iV++) {
      dCoord[2*iV  ] = 0.0f;
      dCoord[2*iV+1] = 0.0f;
      wCoord[iV    ] = 0.0f;
    }

    // accumulate data term
    for(iV=0;iV<nV;iV++) {
      d1x0 = coord0[2*iV  ]-coord[2*iV  ];
      d1x1 = coord0[2*iV+1]-coord[2*iV+1];
      w              = (1.0f-lambda)/((float)nV);;
      dCoord[2*iV  ] = w*d1x0;
      dCoord[2*iV+1] = w*d1x1;
      wCoord[iV]     = w; // 1.0f;
    }

    // accumulate smoothing term
    gt.start();
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      d1x0 = coord[2*iV1  ]-coord[2*iV0  ];
      d1x1 = coord[2*iV1+1]-coord[2*iV0+1];
      w   = lambda/((float)nE);
      dCoord[2*iV0  ] += w*d1x0;
      dCoord[2*iV0+1] += w*d1x1;
      wCoord[iV0]     += w    ;
      dCoord[2*iV1  ] -= w*d1x0;
      dCoord[2*iV1+1] -= w*d1x1;
      wCoord[iV1]     += w    ;
    }

    // normalize descent vector
    for(iV=0;iV<nV;iV++) {
      if((w=wCoord[iV])>0.0f) {
        dCoord[2*iV  ] /= w;
        dCoord[2*iV+1] /= w;
      }
    }

    // compute optimal descent step
    a = b = descentStep = 0.0f;

    // accumulate a
    for(iV=0;iV<nV;iV++) {
      d1x0 = coord[2*iV  ]-coord0[2*iV  ];
      d1x1 = coord[2*iV+1]-coord0[2*iV+1];
      w = (1.0f-lambda)/((float)nV);
      a += w*(dCoord[2*iV  ]*d1x0+dCoord[2*iV+1]*d1x1);
    }
    gt.start();
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      d2x0 = dCoord[2*iV1  ]-dCoord[2*iV0  ];
      d2x1 = dCoord[2*iV1+1]-dCoord[2*iV0+1];
      d1x0  = coord[2*iV1  ]-coord[2*iV0  ];
      d1x1  = coord[2*iV1+1]-coord[2*iV0+1];
      w   = lambda/((float)nE);
      a  += w*(d1x0*d2x0+d1x1*d2x1);
    }

    // accumulate b
    for(iV=0;iV<nV;iV++) {
      d1x0 = dCoord[2*iV  ];
      d1x1 = dCoord[2*iV+1];
      w   = (1.0f-lambda)/((float)nV);
      b  += w*(d1x0*d1x0+d1x1*d1x1);
    }
    gt.start();
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      d2x0 = dCoord[2*iV1  ]-dCoord[2*iV0  ];
      d2x1 = dCoord[2*iV1+1]-dCoord[2*iV0+1];
      w   = lambda/((float)nE);
      b  += w*(d2x0*d2x0+d2x1*d2x1);
    }

    if(b>0.0f) {
      // apply optimal descent step
      descentStep = -a/b;
      for(iV=0;iV<nV;iV++)
        if((w=wCoord[iV])>0.0f) {
          d1x0 = dCoord[2*iV  ];
          d1x1 = dCoord[2*iV+1];
          coord[2*iV  ] += descentStep*d1x0;
          coord[2*iV+1] += descentStep*d1x1;
        }
    }
  }

  // _log("} anchoredSmooth()");
}

//////////////////////////////////////////////////////////////////////
void GraphPlanar::deleteVertices(VecInt& vDelete) {
  _log("deleteVertices() {");
  int nV = getNumberOfVertices();
  int nE = getNumberOfEdges();

  // _log(QString("  nV = %1").arg(nV));
  // _log(QString("  nE = %1").arg(nE));
  // _log(QString("  vDelete.size() = %1").arg(vDelete.size()));

  // _log(QString("  nVertexColor = %1").arg(_vertexColor.size()));
  // _log(QString("  nEdgeColor   = %1").arg(_edgeColor.size()));

  VecInt vMap(nV,0);
  int i,iV,iV0,iV1,iV0n,iV1n,iE;
  for(i=0;i<vDelete.size();i++) {
    iV = vDelete[i];
    if(0<=iV && iV<nV)
      vMap[iV] = -1;
  }

  int nDelete = 0;
  for(iV=0;iV<nV;iV++)
    if(vMap[iV]<0)
      nDelete++;
  // _log(QString("  nDelete = %1").arg(nDelete));

  // also delete all disconnected vertices
  VecInt order(nV,0);
  GraphEdge* e;
  GraphTraversal gt(*this);
  while((e=gt.next())!=(GraphEdge*)0) {
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);
    if(vMap[iV0]>=0 && vMap[iV1]>=0) {
      order[iV0]++;
      order[iV1]++;
    }
  }

  int nOrderDelete = 0;
  for(iV=0;iV<nV;iV++)
    if(order[iV]==0 || order[iV]>=3) {
      vMap[iV] = -1;
      nOrderDelete++;
    }
  // _log(QString("  nOrderDelete = %1").arg(nOrderDelete));

  nDelete = 0;
  for(iV=0;iV<nV;iV++)
    if(vMap[iV]<0)
      nDelete++;
  // _log(QString("  nDelete = %1").arg(nDelete));

  float    x,y;
  VecFloat newCoord;
  VecInt   newVertexColor;
  for(iV=0;iV<nV;iV++) {
    if(vMap[iV]<0) continue;
    vMap[iV] = newCoord.size()/2;
    x = _coord[2*iV  ];
    y = _coord[2*iV+1];
    newCoord.append(x);
    newCoord.append(y);
    if(_vertexColor.size()==nV) {
      i = _vertexColor[iV];
      newVertexColor.append(i);
    }
  }

  // _log(QString("  nVnew = %1").arg(newCoord.size()/2));

  int nVn = newVertexColor.size()/2;
  Graph newGraph(nVn,false);
  VecInt newEdgeColor;

  gt.start();
  while((e=gt.next())!=(GraphEdge*)0) {
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);
    iE = e->getIndex();
    i = (_edgeColor.size()==nE)?_edgeColor[iE]:-1;
    if((iV0n=vMap[iV0])>=0 && (iV1n=vMap[iV1])>=0) {
      newGraph.insertEdge(iV0n,iV1n);
      if(_edgeColor.size()==nE) newEdgeColor.append(i);
    }
  }
  newGraph.enumerateEdges();

  // _log(QString("  nEnew = %1").arg(newGraph.getNumberOfEdges()));

  newCoord.swap(_coord);
  newEdgeColor.swap(_edgeColor);
  newVertexColor.swap(_vertexColor);
  newGraph.swap(*this);

  // _log("} deleteVertices()");
}

//////////////////////////////////////////////////////////////////////
void GraphPlanar::deleteSharpVertices(float max_sin_angle) {
  // _log("deleteSharpVertices() {");

  if(max_sin_angle<-1.0f)
    max_sin_angle = -1.0f;
  else if(max_sin_angle>1.0f)
    max_sin_angle = 1.0f;

  // _log(QString("  max_sin_angle = %1").arg(max_sin_angle));

  enumerateEdges();

  // TODO Mon Apr 25 22:00:04 2016
  // temporary ...
  _edgeColor.clear();
  _vertexColor.clear();

  int   nV,iV,iV0,iV1,iVn,n0,n1,n2,n3,n4;
  float x,x0,x1,dx,dx0,dx1,y,y0,y1,dy,dy0,dy1,dn;

  nV = getNumberOfCoord();
  VecFloat&  coord = _coord; // getCoord();
  VecInt     order(nV,0);
  VecInt     neighb(2*nV,-1);
  GraphEdge* e;
  GraphTraversal gt(*this);
  while((e=gt.next())!=(GraphEdge*)0) {
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);

    x0  = coord[2*iV0  ];
    y0  = coord[2*iV0+1];
    x1  = coord[2*iV1  ];
    y1  = coord[2*iV1+1];
    dx  = x1-x0;
    dy  = y1-y0;
    dn  = dx*dx+dy*dy;
    if(dn==0.0f) {
      std::cerr << "   dn("<<iV0<<","<<iV1<<")=0\n";
    }

    order[iV0]++;
    if(neighb[2*iV0  ]<0) {
      neighb[2*iV0  ] = iV1;
    } else if(neighb[2*iV0+1]<0) {
      neighb[2*iV0+1] = iV1;
    } else {
      // ignore
    }

    order[iV1]++;
    if(neighb[2*iV1  ]<0) {
      neighb[2*iV1  ] = iV0;
    } else if(neighb[2*iV1+1]<0) {
      neighb[2*iV1+1] = iV0;
    } else {
      // ignore
    }
  }

  // erase non-regular vertex links; just in case
  for(iV=0;iV<nV;iV++) {
    if(order[iV]!=2) {
      neighb[2*iV  ] = -1;
      neighb[2*iV+1] = -1;
    }
  }

  for(n0=n1=n2=n3=n4=iV=0;iV<nV;iV++) {
    switch(order[iV]) {
    case 0: n0++; break;
    case 1: n1++; break;
    case 2: n2++; break;
    case 3: n3++; break;
    case 4: n4++; break;
    }
  }
  std::cerr << "  nV = "<< nV <<"\n";
  std::cerr << "  n0 = "<< n0 <<"\n";
  std::cerr << "  n1 = "<< n1 <<"\n";
  std::cerr << "  n2 = "<< n2 <<"\n";
  std::cerr << "  n3 = "<< n3 <<"\n";
  std::cerr << "  n4 = "<< n4 <<"\n";

  float fKey,fKeyMin=1.0f,fKeyMax=-1.0f;

  // forward mapping from old to new vertices
  VecInt vFMap(nV,-1);
  // inverse mapping from new to old vertices
  VecInt vIMap;
  VecInt vDelete;
  
  for(iV=0;iV<nV;iV++) {
    x = coord[2*iV  ];
    y = coord[2*iV+1];
    if(order[iV]==2) {
        
      iV0 = neighb[2*iV  ];
      x0  = coord[2*iV0  ];
      y0  = coord[2*iV0+1];

      iV1 = neighb[2*iV+1];
      x1  = coord[2*iV1  ];
      y1  = coord[2*iV1+1];

      dx0 = x0-x;
      dy0 = y0-y;
      if((dn = dx0*dx0+dy0*dy0)>0.0f) {
        dn = sqrt(dn); dx0 /= dn; dy0 /= dn;
      }

      dx1 = x1-x;
      dy1 = y1-y;
      if((dn = dx1*dx1+dy1*dy1)>0.0f) {
        dn = sqrt(dn); dx1 /= dn; dy1 /= dn;
      }

      // fKey = sin(alpha)^2
      fKey = 1.0f+(dx0*dx1+dy0*dy1);
      if(fKey<0.0f) fKey = 0.0f;
        
      if(fKey<fKeyMin) fKeyMin = fKey;
      if(fKey>fKeyMax) fKeyMax = fKey;

      if(fKey>max_sin_angle) {
        vDelete.append(iV);
      } else {
        iVn = vIMap.size();
        vFMap[iV] = iVn;
        vIMap.append(iV);
      }

      // ...

    }
  }

  int nVnew = vIMap.size();
      
  std::cerr << "  fKeyMin  = "<< fKeyMin << " \n";
  std::cerr << "  fKeyMax  = "<< fKeyMax << " \n";
  std::cerr << "  nVold    = "<< nV << " \n";
  std::cerr << "  nVnew    = "<< nVnew << " \n";
  std::cerr << "  nDeleted = "<< vDelete.size() << " \n";

  deleteVertices(vDelete);

  // _log("} deleteSharpVertices()");
}

// format compatible with JImgShop
//////////////////////////////////////////////////////////////////////
void GraphPlanar::write(QString filename) {

  QFile file(filename);
  file.open(QIODevice::WriteOnly);
  QTextStream out(&file);

  out << QString("#GraphPlanar V1.0\n");

  out << QString("coord [\n");
  int nCoord = _coord.size()/2;
  for(int i=0;i<nCoord;i++) {
    out <<
      QString(" %1 %2\n").arg(_coord[2*i  ]).arg(_coord[2*i+1]);
  }
  out << QString("]\n");

  out << QString("edges [\n");
  GraphEdge* e;
  GraphTraversal gt(*this);
  while((e=gt.next())!=(GraphEdge*)0) {
    out <<
      QString(" %1 %2\n").arg(e->getVertex(0)).arg(e->getVertex(1));
  }
  out << QString("]\n");
      
  file.close();
}
