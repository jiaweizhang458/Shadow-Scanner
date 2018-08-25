//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-12-03 14:53:03 taubin>
//------------------------------------------------------------------------
//
// SceneGraphProcessor.cpp
//
// Software developed for the Fall 2015 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2015, Gabriel Taubin
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

#include <iostream>
#include "SceneGraphProcessor.h"
#include "SceneGraphTraversal.h"
#include "Shape.h"
#include "IndexedFaceSet.h"
#include "IndexedLineSet.h"
#include "Appearance.h"
#include "Material.h"
#include "IsoSurf.h"
#include "SimpleGraphMap.h"

SceneGraphProcessor::SceneGraphProcessor(SceneGraph& wrl):
  _wrl(wrl),
  _nGrid(0),
  _nPoints(0),
  _next((int*)0),
  _first((int*)0) {
}

SceneGraphProcessor::~SceneGraphProcessor() {
  _deletePartition();
}

int SceneGraphProcessor::_createPartition
(Vec3f& min, Vec3f& max, int depth, vector<float>&coord) {
  int nPointsInPartition = 0;
  _deletePartition();
  float dx = max.x-min.x;
  float dy = max.y-min.y;
  float dz = max.z-min.z;
  if(dx>0.0f && dy>0.0f && dz>0.0f) {
    _nPoints = (int)(coord.size()/3);
    if(depth<0) depth = 0;
    _nGrid = 1<<depth;
    int nCells  = _nCells();
    _next    = new int[_nPoints];
    _first   = new int[nCells];
    int iCell,iPoint,ix,iy,iz;
    float x,y,z,nG=(float)_nGrid;
    for(iCell=0;iCell<nCells;iCell++)
      _first[iCell] = -1;
    for(iPoint=0;iPoint<_nPoints;iPoint++) {  
      if((x = coord[3*iPoint  ])<min.x || x>max.x) continue;
      if((y = coord[3*iPoint+1])<min.y || y>max.y) continue;
      if((z = coord[3*iPoint+2])<min.z || z>max.z) continue;
      ix = (int)((nG*(x-min.x))/dx);
      iy = (int)((nG*(y-min.y))/dy);
      iz = (int)((nG*(z-min.z))/dz);
      iCell = ix+_nGrid*(iy+_nGrid*iz);
      _next[iPoint] = _first[iCell];
    _first[iCell] = iPoint;
    nPointsInPartition++;
    }
  }
  return nPointsInPartition;
}

void SceneGraphProcessor::_deletePartition() {
  if(_first!=(int*)0) delete [] _first;
  if(_next !=(int*)0) delete [] _next;
  _nGrid   = 0;
  _nPoints = 0;
  _next    = (int*)0;
  _first   = (int*)0;
}

void SceneGraphProcessor::normalClear() {
  _applyToIndexedFaceSet(_normalClear);
}

void SceneGraphProcessor::normalInvert() {
  _applyToIndexedFaceSet(_normalInvert);
}

void SceneGraphProcessor::computeNormalPerFace() {
  _applyToIndexedFaceSet(_computeNormalPerFace);
}

void SceneGraphProcessor::computeNormalPerVertex() {
  _applyToIndexedFaceSet(_computeNormalPerVertex);
}

void SceneGraphProcessor::computeNormalPerCorner() {
  _applyToIndexedFaceSet(_computeNormalPerCorner);
}

void SceneGraphProcessor::_applyToIndexedFaceSet(IndexedFaceSet::Operator o) {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        IndexedFaceSet& ifs = *((IndexedFaceSet*)node);
        o(ifs);
      }
    }
  }
}

void SceneGraphProcessor::_normalClear(IndexedFaceSet& ifs) {
  vector<float>& normal      = ifs.getNormal();
  vector<int>&   normalIndex = ifs.getNormalIndex();
  ifs.setNormalPerVertex(true);
  normal.clear();
  normalIndex.clear();
}

void SceneGraphProcessor::_normalInvert(IndexedFaceSet& ifs) {
  vector<float>& normal = ifs.getNormal();
  for(int i=0;i<(int)normal.size();i++)
    normal[i] = -normal[i];
}

void SceneGraphProcessor::_computeFaceNormal
(vector<float>& coord, vector<int>& coordIndex,
 int i0, int i1, Vector3d& n, bool normalize) {
  int niF,iV,i;
  Vector3d p,pi,ni,v1,v2;
  niF = i1-i0; // number of face corners
  n << 0,0,0;
  if(niF==3) { // triangle
    // triangle
    iV = coordIndex[i0];
    p << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
    iV = coordIndex[i0+1];
    v1 << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
    iV = coordIndex[i0+2];
    v2 << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
    v1 -= p;
    v2 -= p;
    n = v1.cross(v2);
    if(normalize) n.normalize();
  } else if(niF>3) { // polygon
    // compute face centroid
    p << 0,0,0;
    for(i=i0;i<i1;i++) {
      iV = coordIndex[i];
      pi << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
      p += pi;
    }
    p /= ((float)niF);
    // accumulate face normal
    iV = coordIndex[i1-1];
    v1 << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
    v1 -= p;
    for(i=i0;i<i1;i++) {
      iV = coordIndex[i];
      v2 << coord[3*iV  ],coord[3*iV+1],coord[3*iV+2];
      v2 -= p;
      ni = v1.cross(v2);
      n += ni;
      v1.swap(v2);
    }
    if(normalize) n.normalize();
  } else /* if(n<3) */{ // face with less than 3 vertices
    // throw exception ?
    // n == (0,0,0)
  }
}

void SceneGraphProcessor::_computeNormalPerFace(IndexedFaceSet& ifs) {
  if(ifs.getNormalBinding()==IndexedFaceSet::PB_PER_FACE) return;
  vector<float>& coord       = ifs.getCoord();
  vector<int>&   coordIndex  = ifs.getCoordIndex();
  vector<float>& normal      = ifs.getNormal();
  vector<int>&   normalIndex = ifs.getNormalIndex();
  ifs.setNormalPerVertex(false);
  normal.clear();
  normalIndex.clear();
  Vector3d n;
  int /*iF,*/ i0,i1;
  for(i0=i1=0;i1<(int)coordIndex.size();i1++) {
    if(coordIndex[i1]<0) {
      _computeFaceNormal(coord,coordIndex,i0,i1,n,true);
      normal.push_back((float)(n(0)));
      normal.push_back((float)(n(1)));
      normal.push_back((float)(n(2)));
      i0=i1+1; /* iF++; */
    }
  }
}

void SceneGraphProcessor::_computeNormalPerVertex(IndexedFaceSet& ifs) {
  if(ifs.getNormalBinding()==IndexedFaceSet::PB_PER_VERTEX) return;
  vector<float>& coord       = ifs.getCoord();
  vector<int>&   coordIndex  = ifs.getCoordIndex();
  vector<float>& normal      = ifs.getNormal();
  vector<int>&   normalIndex = ifs.getNormalIndex();
  ifs.setNormalPerVertex(true);
  normal.clear();
  normalIndex.clear();
  Vector3d n;
  int /*iF,*/ nV,i,i0,i1,iV;
  float x0,x1,x2;
  nV = (int)(coord.size()/3);
  // initialize accumulators
  normal.insert(normal.end(),coord.size(),0.0f);
  // accumulate face normals
  for(i0=i1=0;i1<(int)coordIndex.size();i1++) {
    if(coordIndex[i1]<0) {
      _computeFaceNormal(coord,coordIndex,i0,i1,n,false);
      // accumulate
      for(i=i0;i<i1;i++) {
        iV = coordIndex[i];
        x0 = normal[3*iV  ];
        x1 = normal[3*iV+1];
        x2 = normal[3*iV+2];
        normal[3*iV  ] = x0+((float)(n(0)));
        normal[3*iV+1] = x1+((float)(n(1)));
        normal[3*iV+2] = x2+((float)(n(2)));
      }
      i0=i1+1; /* iF++; */
    }
  }
  for(iV=0;iV<nV;iV++) {
    x0 = normal[3*iV  ];
    x1 = normal[3*iV+1];
    x2 = normal[3*iV+2];
    n << x0,x1,x2;
    n.normalize();
    normal[3*iV  ] = (float)(n(0));
    normal[3*iV+1] = (float)(n(1));
    normal[3*iV+2] = (float)(n(2));
  }
}

void SceneGraphProcessor::_computeNormalPerCorner(IndexedFaceSet& ifs) {
  if(ifs.getNormalBinding()==IndexedFaceSet::PB_PER_CORNER) return;

  vector<float>& coord       = ifs.getCoord();
  vector<int>&   coordIndex  = ifs.getCoordIndex();
  vector<float>& normal      = ifs.getNormal();
  vector<int>&   normalIndex = ifs.getNormalIndex();
  ifs.setNormalPerVertex(true);
  normal.clear();
  normalIndex.clear();

  Vector3d pP,p0,pN,vP,vN,n;
  int iF,i,i0,i1,ip,in,nFC,iV0,iVp,iVn,iN;
  for(iF=i0=i1=0;i1<(int)coordIndex.size();i1++) {
    if(coordIndex[i1]<0) {
      nFC = i1-i0; // number of face corners
      n << 0,0,0;
      if(nFC>=3) { // polygon
        for(i=i0;i<i1;i++) {
          if((ip=i-1)< i0) ip=i1-1;
          if((in=i+1)==i1) in=i0  ;

          iVp = coordIndex[ip];
          iV0 = coordIndex[i ];
          iVn = coordIndex[in];

          pP << coord[3*iVp  ],coord[3*iVp+1],coord[3*iVp+2];
          p0 << coord[3*iV0  ],coord[3*iV0+1],coord[3*iV0+2];
          pN << coord[3*iVn  ],coord[3*iVn+1],coord[3*iVn+2];

          vP = pP-p0;
          vN = pN-p0;
          n = vN.cross(vP);

          n.normalize();

          iN = (int)(normal.size()/3);
          normal.push_back((float)(n(0)));
          normal.push_back((float)(n(1)));
          normal.push_back((float)(n(2)));
          normalIndex.push_back(iN);

        }
        normalIndex.push_back(-1);

      } else /* if(nFC<3) */{ // face with less than 3 vertices
        // throw exception ?
        n << 0,0,0;
        for(i=i0;i<i1;i++) {
          iN = (int)(normal.size()/3);
          normal.push_back((float)(n(0)));
          normal.push_back((float)(n(1)));
          normal.push_back((float)(n(2)));
          normalIndex.push_back(iN);
        }
        normalIndex.push_back(-1);
      }

      i0=i1+1; iF++;
    }
  }
}

void SceneGraphProcessor::bboxAdd
(int depth, float scale, bool isCube) {
  const string name = "BOUNDING-BOX";
  Shape* shape = (Shape*)0;
  const Node*  node = _wrl.getChild(name);
  if(node==(Node*)0) {
    shape = new Shape();
    shape->setName(name);
    Appearance* appearance = new Appearance();
    shape->setAppearance(appearance);
    Material* material = new Material();
    // colors should be stored in WrlViewerData
    Color bboxColor(0.5f,0.3f,0.0f);
    material->setDiffuseColor(bboxColor);
    appearance->setMaterial(material);
    _wrl.addChild(shape);
  } else if(node->isShape()) {
    shape = (Shape*)node;
  }
  if(shape==(Shape*)0) { /* throw exception ??? */ return; }

  IndexedLineSet* ils = (IndexedLineSet*)0;
  node = shape->getGeometry();
  if(node==(Node*)0) {
    ils = new IndexedLineSet();
    shape->setGeometry(ils);
  } else if(node->isIndexedLineSet()) {
    ils = (IndexedLineSet*)node;
  }
  if(ils==(IndexedLineSet*)0) { /* throw exception ??? */ return; }

  vector<float>& coord      = ils->getCoord();
  vector<int>&   coordIndex = ils->getCoordIndex();
  vector<float>& color      = ils->getColor();
  vector<int>&   colorIndex = ils->getColorIndex();
  coord.clear();
  coordIndex.clear();
  color.clear();
  colorIndex.clear();
  ils->setColorPerVertex(true);

  _wrl.updateBBox();
  Vec3f& center = _wrl.getBBoxCenter();
  Vec3f& size   = _wrl.getBBoxSize();

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  if(isCube) {
    float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
    dx = dMax; dy = dMax; dz = dMax;
  }
  if(scale>0.0f) {
    dx *= scale; dy *= scale; dz *= scale;
  }

  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;

  if(depth==0) {
    
    // vertices
    coord.push_back(x0); coord.push_back(y0); coord.push_back(z0);
    coord.push_back(x0); coord.push_back(y0); coord.push_back(z1);
    coord.push_back(x0); coord.push_back(y1); coord.push_back(z0);
    coord.push_back(x0); coord.push_back(y1); coord.push_back(z1);
    coord.push_back(x1); coord.push_back(y0); coord.push_back(z0);
    coord.push_back(x1); coord.push_back(y0); coord.push_back(z1);
    coord.push_back(x1); coord.push_back(y1); coord.push_back(z0);
    coord.push_back(x1); coord.push_back(y1); coord.push_back(z1);
    
    // edges
    coordIndex.push_back(0); coordIndex.push_back(1); coordIndex.push_back(-1);
    coordIndex.push_back(2); coordIndex.push_back(3); coordIndex.push_back(-1);
    coordIndex.push_back(4); coordIndex.push_back(5); coordIndex.push_back(-1);
    coordIndex.push_back(6); coordIndex.push_back(7); coordIndex.push_back(-1);
    //
    coordIndex.push_back(0); coordIndex.push_back(2); coordIndex.push_back(-1);
    coordIndex.push_back(1); coordIndex.push_back(3); coordIndex.push_back(-1);
    coordIndex.push_back(4); coordIndex.push_back(6); coordIndex.push_back(-1);
    coordIndex.push_back(5); coordIndex.push_back(7); coordIndex.push_back(-1);
    // iz=0
    coordIndex.push_back(0); coordIndex.push_back(4); coordIndex.push_back(-1);
    coordIndex.push_back(1); coordIndex.push_back(5); coordIndex.push_back(-1);
    coordIndex.push_back(2); coordIndex.push_back(6); coordIndex.push_back(-1);
    coordIndex.push_back(3); coordIndex.push_back(7); coordIndex.push_back(-1);

  } else {

    int N = 1<<depth;

    // vertices
    float x,y,z;
    int ix,iy,iz,jx,jy,jz,iV0,iV1;

    for(iz=0,jz=N;iz<=N;iz++,jz--) {
      z = (((float)jz)*z0+((float)iz)*z1)/((float)N);
      for(iy=0,jy=N;iy<=N;iy++,jy--) {
        y = (((float)jy)*y0+((float)iy)*y1)/((float)N);
        for(ix=0,jx=N;ix<=N;ix++,jx--) {
          x = (((float)jx)*x0+((float)ix)*x1)/((float)N);
          coord.push_back(x); coord.push_back(y); coord.push_back(z);
        }
      }
    }

    // edges
    for(iz=0;iz<N;iz++) {
      for(iy=0;iy<=N;iy++) {
        for(ix=0;ix<=N;ix++) {
          iV0 = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          iV1 = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1));
          coordIndex.push_back(iV0);
          coordIndex.push_back(iV1);
          coordIndex.push_back(-1);
        }
      }
    }
    for(iz=0;iz<=N;iz++) {
      for(iy=0;iy<N;iy++) {
        for(ix=0;ix<=N;ix++) {
          iV0 = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          iV1 = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  ));
          coordIndex.push_back(iV0);
          coordIndex.push_back(iV1);
          coordIndex.push_back(-1);
        }
      }
    }
    for(iz=0;iz<=N;iz++) {
      for(iy=0;iy<=N;iy++) {
        for(ix=0;ix<N;ix++) {
          iV0 = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          iV1 = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  ));
          coordIndex.push_back(iV0);
          coordIndex.push_back(iV1);
          coordIndex.push_back(-1);
        }
      }
    }

  }
}

void SceneGraphProcessor::bboxRemove() {
  vector<pNode>& children = _wrl.getChildren();
  vector<pNode>::iterator i;
  for(i=children.begin();i!=children.end();i++)
    if((*i)->nameEquals("BOUNDING-BOX"))
      break;
  if(i!=children.end())
    children.erase(i);
}

void SceneGraphProcessor::edgesAdd() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  const Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape  = (Shape*)node;
      const Node* parent = shape->getParent();
      Group* group = (Group*)parent;

      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        IndexedFaceSet* ifs = (IndexedFaceSet*)node;

        shape->setShow(false);

        // compose the node name ???
        string name = "EDGES";
        node = group->getChild(name);
        if(node==(Node*)0) {
          shape = new Shape();
          shape->setName(name);
          Appearance* appearance = new Appearance();
          shape->setAppearance(appearance);
          Material* material = new Material();
          // colors should be stored in WrlViewerData
          Color edgeColor(1.0f,0.5f,0.0f);
          material->setDiffuseColor(edgeColor);
          appearance->setMaterial(material);
          group->addChild(shape);
        } else if(node->isShape()) {
          shape = (Shape*)node;
        } else /* if(node!=(Node*)0 && node->isShape()==false */ {
          // throw exception ???
        }
        if(shape==(Shape*)0) { /* throw exception ??? */ return; }

        IndexedLineSet* ils = (IndexedLineSet*)0;
        node = shape->getGeometry();
        if(node==(Node*)0) {
          ils = new IndexedLineSet();
          shape->setGeometry(ils);
        } else if(node->isIndexedLineSet()) {
          ils = (IndexedLineSet*)node;
        } else /* if(node!=(Node*)0 && node->isIndexedLineSet()==false) */ {
          // throw exception ???
        }
        
        if(ils==(IndexedLineSet*)0) { /* throw exception ??? */ return; }

        ils->clear();

        vector<float>& coordIfs      = ifs->getCoord();
        vector<int>&   coordIndexIfs = ifs->getCoordIndex();

        vector<float>& coordIls      = ils->getCoord();
        vector<int>&   coordIndexIls = ils->getCoordIndex();

        coordIls.insert(coordIls.end(),
                        coordIfs.begin(),coordIfs.end());

        int i,i0,i1,nV,iV,iV0,iV1,iF;
        nV = coordIfs.size()/3;
        for(iV=0;iV<nV;iV++) {
          coordIls.push_back(coordIfs[3*iV  ]);
          coordIls.push_back(coordIfs[3*iV+1]);
          coordIls.push_back(coordIfs[3*iV+2]);
        }

        for(iF=i0=i1=0;i1<(int)coordIndexIfs.size();i1++) {
          if(coordIndexIfs[i1]<0) {
            iV0 = coordIndexIfs[i1-1];
            for(i=i0;i<i1;i++) {
              iV1 = coordIndexIfs[i];
              coordIndexIls.push_back(iV0);
              coordIndexIls.push_back(iV1);
              coordIndexIls.push_back(-1);
              iV0 = iV1;
            }
            iF++; i0 = i1+1;
          }
        }


      }
    }
  }
}

void SceneGraphProcessor::edgesRemove() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape  = (Shape*)node;
      const Node* parent = shape->getParent();
      Group* group = (Group*)parent;
      vector<pNode>& children = group->getChildren();
      vector<pNode>::iterator i;
      do {
        for(i=children.begin();i!=children.end();i++)
          if((*i)->nameEquals("EDGES"))
            break;
        if(i!=children.end()) {
          children.erase(i);
          i=children.begin();
        }
      } while(i!=children.end());
    }
  }
}

void SceneGraphProcessor::shapeIndexedFaceSetShow() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        shape->setShow(true);
      }
    }
  }
}

void SceneGraphProcessor::shapeIndexedFaceSetHide() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        shape->setShow(false);
      }
    }
  }
}

void SceneGraphProcessor::shapeIndexedLineSetShow() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedLineSet()) {
        shape->setShow(true);
      }
    }
  }
}

void SceneGraphProcessor::shapeIndexedLineSetHide() {
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while((node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedLineSet()) {
        shape->setShow(false);
      }
    }
  }
}

bool SceneGraphProcessor::hasBBox() {
  return _wrl.getChild("BOUNDING-BOX")!=(Node*)0;
}

bool SceneGraphProcessor::_hasShapeProperty(Shape::Property p) {
  bool value = false;
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while(value==false && (node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape& shape = *(Shape*)node;
      value = p(shape);
    }
  }
  return value;
}

bool SceneGraphProcessor::_hasIndexedFaceSetProperty(IndexedFaceSet::Property p) {
  bool value = false;
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while(value==false && (node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      if(shape->hasGeometryIndexedFaceSet()) {
        IndexedFaceSet& ifs = *(IndexedFaceSet*)(shape->getGeometry());
        value = p(ifs);
      }
    }
  }
  return value;
}

bool SceneGraphProcessor::_hasIndexedLineSetProperty(IndexedLineSet::Property p) {
  bool value = false;
  SceneGraphTraversal traversal(_wrl);
  traversal.start();
  Node* node;
  while(value==false && (node=traversal.next())!=(Node*)0) {
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      if(shape->hasGeometryIndexedLineSet()) {
        IndexedLineSet& ils = *(IndexedLineSet*)(shape->getGeometry());
        value = p(ils);
      }
    }
  }
  return value;
}

bool SceneGraphProcessor::_hasFaces(IndexedFaceSet& ifs) {
  return (ifs.getNumberOfCoord()>0 && ifs.getNumberOfFaces()>0);
}

bool SceneGraphProcessor::_hasNormalNone(IndexedFaceSet& ifs) {
  return
    (ifs.getNumberOfCoord()==0 && ifs.getNumberOfFaces()==0) ||
    (ifs.getNormalBinding()==IndexedFaceSet::PB_NONE);
}

bool SceneGraphProcessor::_hasNormalPerFace(IndexedFaceSet& ifs) {
  return 
    (ifs.getNumberOfFaces()>0) &&
    (ifs.getNormalBinding()==IndexedFaceSet::PB_PER_FACE);
}

bool SceneGraphProcessor::_hasNormalPerVertex(IndexedFaceSet& ifs) {
  return
    (ifs.getNumberOfCoord()>0) &&
    (ifs.getNormalBinding()==IndexedFaceSet::PB_PER_VERTEX);
}

bool SceneGraphProcessor::_hasNormalPerCorner(IndexedFaceSet& ifs) {
  return
    (ifs.getNumberOfFaces()>0) &&
    (ifs.getNormalBinding()==IndexedFaceSet::PB_PER_CORNER);
}

bool SceneGraphProcessor::hasIndexedFaceSetFaces() {
  return _hasIndexedFaceSetProperty(_hasFaces);
}

bool SceneGraphProcessor::hasIndexedFaceSetNormalNone() {
  return _hasIndexedFaceSetProperty(_hasNormalNone);
}

bool SceneGraphProcessor::hasIndexedFaceSetNormalPerFace() {
  return _hasIndexedFaceSetProperty(_hasNormalPerFace);
}

bool SceneGraphProcessor::hasIndexedFaceSetNormalPerVertex() {
  return _hasIndexedFaceSetProperty(_hasNormalPerVertex);
}

bool SceneGraphProcessor::hasIndexedFaceSetNormalPerCorner() {
  return _hasIndexedFaceSetProperty(_hasNormalPerCorner);
}

// VRML'97
//
// If the color field is not NULL, it shall contain a Color node, and
// the colours are applied to the line(s) as follows:
//
// If colorPerVertex is FALSE:
//
// If the colorIndex field is not empty, then one colour is used for
// each polyline of the IndexedLineSet. There must be at least as many
// indices in the colorIndex field as there are polylines in the
// IndexedLineSet. If the greatest index in the colorIndex field is N,
// then there must be N+1 colours in the Color node. The colorIndex
// field must not contain any negative entries.
//
// If the colorIndex field is empty, then the colours from the Color
// node are applied to each polyline of the IndexedLineSet in
// order. There must be at least as many colours in the Color node as
// there are polylines.
//
// If colorPerVertex is TRUE:
//
// If the colorIndex field is not empty, then colours are applied to
// each vertex of the IndexedLineSet in exactly the same manner that
// the coordIndex field is used to supply coordinates for each vertex
// from the Coordinate node. The colorIndex field must contain at
// least as many indices as the coordIndex field and must contain
// end-of-polyline markers (-1) in exactly the same places as the
// coordIndex field. If the greatest index in the colorIndex field is
// N, then there must be N+1 colours in the Color node.
//
// If the colorIndex field is empty, then the coordIndex field is used
// to choose colours from the Color node. If the greatest index in the
// coordIndex field is N, then there must be N+1 colours in the Color
// node.
//
// If the color field is NULL and there is a Material defined for the
// Appearance affecting this IndexedLineSet, the emissiveColor of the
// Material shall be used to draw the lines.

 bool SceneGraphProcessor::_hasColorNone(IndexedLineSet& ils) {
  vector<float>& color         = ils.getColor();
  return (color.size()==0);
}

 bool SceneGraphProcessor::_hasColorPerVertex(IndexedLineSet& ils) {
  vector<float>& color         = ils.getColor();
  // vector<int>&   colorIndex    = ils.getColorIndex();
  bool           colorPerVerex = ils.getColorPerVertex();
  // not testing for errors, but
  // if(colorIndex.size()==0)
  //   we should have color.size()/3 == ils.getNumberOfCoord()
  // else
  //   we should have colorIndex.size() == ils.getNumberOfCoord()
  return color.size()>0 && colorPerVerex==false;
}

 bool SceneGraphProcessor::_hasColorPerPolyline(IndexedLineSet& ils) {
  vector<float>& color         = ils.getColor();
  // vector<int>&   colorIndex    = ils.getColorIndex();
  bool           colorPerVerex = ils.getColorPerVertex();
  // not testing for errors, but
  // if(colorIndex.size()==0)
  //   we should have color.size()/3 == ils.getNumberOfPolylines()
  // else
  //   we should have colorIndex.size() == ils.getNumberOfPolylines()
  return color.size()>0 && colorPerVerex==false;
}

bool SceneGraphProcessor::hasIndexedLineSetColorNone() {
  return _hasIndexedLineSetProperty(_hasColorNone);
}

bool SceneGraphProcessor::hasIndexedLineSetColorPerVertex() {
  return _hasIndexedLineSetProperty(_hasColorPerVertex);
}

bool SceneGraphProcessor::hasIndexedLineSetColorPerPolyline() {
  return _hasIndexedLineSetProperty(_hasColorPerPolyline);
}

 bool SceneGraphProcessor::_hasEdges(Shape& shape) {
   return shape.nameEquals("EDGES");
}

 bool SceneGraphProcessor::_hasIndexedFaceSetShown(Shape& shape) {
  return shape.hasGeometryIndexedFaceSet() && shape.getShow()==true;
}

 bool SceneGraphProcessor::_hasIndexedFaceSetHidden(Shape& shape) {
  return shape.hasGeometryIndexedFaceSet() && shape.getShow()==false;
}

 bool SceneGraphProcessor::_hasIndexedLineSetShown(Shape& shape) {
  return shape.hasGeometryIndexedLineSet() && shape.getShow()==true;
}

 bool SceneGraphProcessor::_hasIndexedLineSetHidden(Shape& shape) {
  return shape.hasGeometryIndexedLineSet() && shape.getShow()==false;
}

bool SceneGraphProcessor::hasEdges() {
  return _hasShapeProperty(_hasEdges);
}

bool SceneGraphProcessor::hasIndexedFaceSetShown() {
  return _hasShapeProperty(_hasIndexedFaceSetShown);
}

bool SceneGraphProcessor::hasIndexedFaceSetHidden() {
  return _hasShapeProperty(_hasIndexedFaceSetHidden);
}

bool SceneGraphProcessor::hasIndexedLineSetShown() {
  return _hasShapeProperty(_hasIndexedLineSetShown);
}

bool SceneGraphProcessor::hasIndexedLineSetHidden() {
  return _hasShapeProperty(_hasIndexedLineSetHidden);
}

void SceneGraphProcessor::removeSceneGraphChild(const string& name) {
  vector<pNode>& children = _wrl.getChildren();
  vector<pNode>::iterator i;
  for(i=children.begin();i!=children.end();i++)
    if((*i)->nameEquals(name))
      break;
  if(i!=children.end())
    children.erase(i);
}

void SceneGraphProcessor::pointsRemove() {
  removeSceneGraphChild("POINTS");
}

void SceneGraphProcessor::surfaceRemove() {
  removeSceneGraphChild("SURFACE");
}

IndexedFaceSet* SceneGraphProcessor::_getNamedShapeIFS
(const string& name, bool create) {
  IndexedFaceSet* ifs = (IndexedFaceSet*)0;
  Node* node = _wrl.getChild(name);
  if(node!=(Node*)0 && node->isShape()) {
    Shape* shape = (Shape*)node;
    node = shape->getGeometry();
    if(node!=(Node*)0 && node->isIndexedFaceSet()) {
      ifs = (IndexedFaceSet*)node;
    }
  }
  if(ifs==(IndexedFaceSet*)0 && create) {
    Shape* shape = new Shape();
    shape->setName(name);
    _wrl.addChild(shape);
    Appearance* appearance = new Appearance();
    shape->setAppearance(appearance);
    Material* material = new Material();
    appearance->setMaterial(material);
    ifs = new IndexedFaceSet();
    shape->setGeometry(ifs);
  }
  return ifs;
}

//////////////////////////////////////////////////////////////////////
void eigenFit(const vector<float>& coordPoints,
              const Vec3f& min, const Vec3f& max, Vec4f& f) {

  char str[256];
  std::cerr << "eigenFit() {" <<endl;

  int nPoints = (int)((coordPoints.size())/3);
  float x0 = min.x, x1 = max.x, dx = x1-x0;
  float y0 = min.y, y1 = max.y, dy = y1-y0;
  float z0 = min.z, z1 = max.z, dz = z1-z0;
  float dMax = dx; if(dy>dMax) dMax = dy; if(dz>dMax) dMax = dz;

  // compute the mean of the points contained in the 
  double x,y,z;
  double xMean = 0.0f;
  double yMean = 0.0f;
  double zMean = 0.0f;
  int   nMean = 0;
  for(int iP=0;iP<nPoints;iP++) {
    x = (double)(coordPoints[3*iP  ]);
    y = (double)(coordPoints[3*iP+1]);
    z = (double)(coordPoints[3*iP+2]);
    if(x0<=x && x<=x1 && y0<=y && y<=y1 && z0<=z && z<=z1) {
      xMean += x;
      yMean += y;
      zMean += z;
      nMean++;
    }
  }

  std::cerr << "  nMean = " << nMean << endl;
  if(nMean==0) {
    // throw exception ??
    return;
  }

  xMean /= ((double)nMean);
  yMean /= ((double)nMean);
  zMean /= ((double)nMean);

  std::cerr << "  pMean = [" <<endl;
  sprintf(str,"    %12.6f",xMean); std::cerr << str << endl;
  sprintf(str,"    %12.6f",yMean); std::cerr << str << endl;
  sprintf(str,"    %12.6f",zMean); std::cerr << str << endl;
  std::cerr << "  ]" <<endl;

  // compute the scatter matrix
  double dxp,dyp,dzp;
  double M00=0.0,M01=0.0,M02=0.0,M11=0.0,M12=0.0,M22=0.0;
  for(int iP=0;iP<nPoints;iP++) {
    x = (double)(coordPoints[3*iP  ]);
    y = (double)(coordPoints[3*iP+1]);
    z = (double)(coordPoints[3*iP+2]);
    if(x0<=x && x<=x1 && y0<=y && y<=y1 && z0<=z && z<=z1) {
      dxp = (x-xMean)/dMax;
      dyp = (y-yMean)/dMax;
      dzp = (z-zMean)/dMax;
      M00 += dxp*dxp; M01 += dxp*dyp; M02 += dxp*dzp;
                      M11 += dyp*dyp; M12 += dyp*dzp;
                                      M22 += dzp*dzp;
    }
  }

  double dMean = (double)nMean;
  M00 /= dMean; M01 /= dMean; M02 /= dMean;
                M11 /= dMean; M12 /= dMean;
                              M22 /= dMean;
  Matrix3d M;
  M << M00,M01,M02,M01,M11,M12,M02,M12,M22;

  // double dMean = (double)nMean;
  // M(0,0) /= dMean; M(0,1) /= dMean; M(0,2) /= dMean;
  // M(1,0) /= dMean; M(1,1) /= dMean; M(1,2) /= dMean;
  // M(2,0) /= dMean; M(2,1) /= dMean; M(2,2) /= dMean;

  std::cerr << "  M = [" << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",M(0,0),M(0,1),M(0,2)); std::cerr << str << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",M(1,0),M(1,1),M(1,2)); std::cerr << str << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",M(2,0),M(2,1),M(2,2)); std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  SelfAdjointEigenSolver<Matrix3d> eigensolver(M);

  if(eigensolver.info() != Success) {
    // trow exception ?
    // abort();
    return;
  }

  Vector3d L(eigensolver.eigenvalues());
  Matrix3d E(eigensolver.eigenvectors());

  std::cerr << "  eigenvalues(M)"<< endl;
  std::cerr << "  L = [" << endl;
  sprintf(str,"    %12.6f",L(0)); std::cerr << str << endl;
  sprintf(str,"    %12.6f",L(1)); std::cerr << str << endl;
  sprintf(str,"    %12.6f",L(2)); std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  std::cerr << "  eigenvectors(M)"<< endl;
  std::cerr << "  E = [" << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",E(0,0),E(0,1),E(0,2)); std::cerr << str << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",E(1,0),E(1,1),E(1,2)); std::cerr << str << endl;
  sprintf(str,"    %12.6f %12.6f %12.6f",E(2,0),E(2,1),E(2,2)); std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  // L(0)                   minimum eigenvalue
  // (E(0,0),E(1,0),E(2,0)) minimum eigenvector

  f.x =  (float)(E(0,0));
  f.y =  (float)(E(1,0));
  f.z =  (float)(E(2,0));
  f.w = -(float)(E(0,0)*xMean+E(1,0)*yMean+E(2,0)*zMean);

  std::cerr << "  f = [" << endl;
  sprintf(str,"    %12.6f",f.x);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",f.y);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",f.z);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",f.w);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  std::cerr << "}" <<endl;
}

//////////////////////////////////////////////////////////////////////
void meanFit(const vector<float>& coordPoints,
             const vector<float>& normalPoints,
             const Vec3f& min, const Vec3f& max, Vec4f& f) {

  int nPoints = (int)((coordPoints.size())/3);
  float x0 = min.x, x1 = max.x, dx = x1-x0;
  float y0 = min.y, y1 = max.y, dy = y1-y0;
  float z0 = min.z, z1 = max.z, dz = z1-z0;
  float dMax = dx; if(dy>dMax) dMax = dy; if(dz>dMax) dMax = dz;

  // compute the mean of the points contained in the 
  double x,y,z,nn;
  double xMean = 0.0f;
  double yMean = 0.0f;
  double zMean = 0.0f;
  double nxMean = 0.0f;
  double nyMean = 0.0f;
  double nzMean = 0.0f;
  int    nMean = 0;
  for(int iP=0;iP<nPoints;iP++) {
    x = (double)(coordPoints[3*iP  ]);
    y = (double)(coordPoints[3*iP+1]);
    z = (double)(coordPoints[3*iP+2]);
    if(x0<=x && x<=x1 && y0<=y && y<=y1 && z0<=z && z<=z1) {
      xMean += x;
      yMean += y;
      zMean += z;
      x = (double)(normalPoints[3*iP  ]);
      y = (double)(normalPoints[3*iP+1]);
      z = (double)(normalPoints[3*iP+2]);
      nxMean += x;
      nyMean += y;
      nzMean += z;
      nMean++;
    }
  }

  if(nMean==0) {
    // throw exception ??
    return;
  }

  // normalize the point mean
  xMean /= ((double)nMean);
  yMean /= ((double)nMean);
  zMean /= ((double)nMean);
  // normalize the normal mean to unit length
  nn = nxMean*nxMean+nyMean*nyMean+nzMean*nzMean;
  if(nn>0.0) {
    nn = sqrt(nn); nxMean/=nn; nyMean/=nn; nzMean/=nn;
  }
  // set the linear function coefficients
  f.x =  (float)(nxMean);
  f.y =  (float)(nyMean);
  f.z =  (float)(nzMean);
  f.w = -(float)(nxMean*xMean+nyMean*yMean+nzMean*zMean);
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::fitSinglePlane
(const Vec3f& center, const Vec3f& size,
 const float scale, const bool isCube, Vec4f& f) {

  // make sure that the bounding box is not empty
  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) {
    // throw exception ?
    return;
  }

  char str[256];
  std::cerr << "SceneGraphProcessor::fitSinglePlane() {" << endl;

  // find the input point set in the scene graph
  IndexedFaceSet* points  = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;
  const vector<float>& coordPoints  = points->getCoord();
  const vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);
    
  std::cerr << "  nPoints = " << nPoints << endl;

  // compute the coordinates of the bounding box corners
  float dx=size.x/2.0f, dy=size.y/2.0f, dz=size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(isCube) { dx = dy = dz = dMax; }
  if(scale>0.0f) { dx *= scale; dy *= scale; dz *= scale; }
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);
  float x,y,z;
  Vec3f v[8]; // bbox corner coordinates
  for(int i=0;i<8;i++) {
    v[i].z = z = (((i>>0)&0x1)==0)?z0:z1;
    v[i].y = y = (((i>>1)&0x1)==0)?y0:y1;
    v[i].x = x = (((i>>2)&0x1)==0)?x0:x1;
  }

  std::cerr << "  bboxMin = [" << endl;
  sprintf(str,"    %12.6f",x0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z0);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;
  std::cerr << "  bboxMax = [" << endl;
  sprintf(str,"    %12.6f",x1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z1);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  // fit a plane to the points contained in the bounding box
  // eigenFit(coordPoints,normalPoints,min,max,f);
  meanFit(coordPoints,normalPoints,min,max,f);

  // find or create the output surface in the scene graph 
  IndexedFaceSet* plane = _getNamedShapeIFS("SURFACE",true);
  plane->clear();
  vector<float>& coordIfs = plane->getCoord();

  // evaluate the linear function at bounding box corners
  float F[8]; // function values at bbox corners
  bool  b[8]; // function is positive or negative ?
  for(int i=0;i<8;i++) {
    b[i] = (F[i] = x*f.x+y*f.y+z*f.z+f.w)<0.0f;
  }

  std::cerr << "//                6 ----- 7 = (x1,y1,z1)" << endl;
  std::cerr << "//               /|      /|             " << endl;
  std::cerr << "//              4 ----- 5 |             " << endl;
  std::cerr << "//              | |     | |             " << endl;
  std::cerr << "//              | 2 ----| 3             " << endl;
  std::cerr << "//              |/      |/              " << endl;
  std::cerr << "// (x0,y0,z0) = 0 ----- 1               " << endl;

  std::cerr << "  b = [" << endl;
  std::cerr << "    " << b[0] << endl;
  std::cerr << "    " << b[1] << endl;
  std::cerr << "    " << b[2] << endl;
  std::cerr << "    " << b[3] << endl;
  std::cerr << "    " << b[4] << endl;
  std::cerr << "    " << b[5] << endl;
  std::cerr << "    " << b[6] << endl;
  std::cerr << "    " << b[7] << endl;
  std::cerr << "  ]" << endl;

  //////////////////////////////////////////////////////////////////////
  //
  //    vertices      //    edges                 //    faces
  //      6-----7     //        [6]---11---[7]    //        1
  //     /|    /|     //        /|         /|     //        | 3
  //    4-----5 |     //       6 2        7 3     //        |/
  //    | 2---|-3     //      /  |       /  |     //    4---+---5
  //    |/    |/      //    [4]---10---[5]  |     //       /|
  //    0-----1       //     |   |      |   |     //      2 |
  //                  //     |  [2]--9--|--[3]    //        0
  //                  //     0  /       1  /      //
  //                  //     | 4        | 5       //
  //                  //     |/         |/        //
  //                  //    [0]---8----[1]        //
  //

  const int (*edge)[2] = IsoSurf::getEdgeTable();

  // compute the isovertex coordinates
  float tj,tk;
  int   iE[12],iV,i,j,k;
  for(i=0;i<12;i++) {
    iV   = -1;
    j    = edge[i][0];
    k    = edge[i][1];
    if(b[j]!=b[k]) {
      // isvertex index
      iV = (int)((coordIfs.size()/3));
      // isovertex coordinates
      tk = F[j]/(F[j]-F[k]);
      tj = F[k]/(F[k]-F[j]);
      x  = tj*v[j].x+tk*v[k].x;
      y  = tj*v[j].y+tk*v[k].y;
      z  = tj*v[j].z+tk*v[k].z;
      coordIfs.push_back(x);
      coordIfs.push_back(y);
      coordIfs.push_back(z);
    }
    iE[i] = iV;
  }
  std::cerr << "  edge to isovertex table" << endl;
  std::cerr << "  iE = [" << endl;
  for(i=0;i<12;i++) {
    sprintf(str,"    %2d (%2d, %2d) -> %3d",i,
            edge[i][0],edge[i][1],iE[i]);
    std::cerr << str << endl;
  }
  std::cerr << "  ]" << endl;

  // create isosurface faces
  vector<int>& coordIndex = plane->getCoordIndex(); // coordIndex.size()==0
  int nFaces = IsoSurf::makeCellFaces(b,iE,coordIndex);
  std::cerr << "  nFaces = " << nFaces << endl;

  // save face normal
  plane->setNormalPerVertex(false);
  vector<float>& normal = plane->getNormal();
  // normal.size()==0 here
  normal.push_back(f.x);
  normal.push_back(f.y);
  normal.push_back(f.z);

  std::cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::fitMultiplePlanes
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool isCube,
 vector<float>& f) {

  char str[256];
  std::cerr << "SceneGraphProcessor::fitMultiplePlanes() {" << endl;

  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;
  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  IndexedFaceSet* surface = _getNamedShapeIFS("SURFACE",true);
  surface->clear();

  f.clear();

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) {
    // throw exception ?
    return;
  }

  std::cerr << "  computing bounding box" <<  endl;

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(isCube) {
    dx = dMax; dy = dMax; dz = dMax;
  }
  if(scale>0.0f) {
    dx *= scale; dy *= scale; dz *= scale;
  }
  float x,y,z;
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  std::cerr << "  bboxMin = [" << endl;
  sprintf(str,"    %12.6f",x0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z0);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;
  std::cerr << "  bboxMax = [" << endl;
  sprintf(str,"    %12.6f",x1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z1);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  int iCell,iPoint,ix,iy,iz;

  // create a partition of the points as an array of linked lists

  std::cerr << "  creating partition" <<  endl;
  _createPartition(min,max,depth,coordPoints);
  std::cerr << "  depth  = " << depth <<  endl;
  std::cerr << "  nGrid  = " << _nGrid <<  endl;
  std::cerr << "  nCells = " << _nCells() <<  endl;

  std::cerr << "  processing cells {" << endl;

  // IndexedFaceSet* surface = _getNamedShapeIFS("SURFACE",true);
  // surface->clear();
  vector<float>& coordIfs      = surface->getCoord();
  vector<int>&   coordIndexIfs = surface->getCoordIndex();
  vector<float>& normalIfs     = surface->getNormal();
  surface->setNormalPerVertex(false);

  Vec3f v[8]; // bbox corner coordinates
  float F[8]; // function values at bbox corners
  bool  b[8]; // function is positive or negative ?

  float tj,tk;
  int   iE[12],iV,i,j,k;

  const int (*edgeTable)[2] = IsoSurf::getEdgeTable();

  // fit a plane to each non-empty cell
  Vec4f fCell;
  Vec3f minCell,maxCell,nCell;
  vector<float> coordCell;
  vector<float> normalCell;
  int nonEmptyCells = 0;
  int nFacesCell    = 0;

  int N = _nGrid;
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);
        if((iPoint=_first[iCell])>=0) {
          nonEmptyCells++;

          // std::cerr << "    cell[" << iCell << "] {" << endl;
          // std::cerr << "      (" << ix << "," << iy << "," << iz << ")" << endl;

          // build cell coord and normal array
          coordCell.clear();
          normalCell.clear();
          // initialize cell mean normal vector
          nCell.x=nCell.y=nCell.z;
          for(;iPoint>=0;iPoint=_next[iPoint]) {
            //
            x = coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y = coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z = coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
            // acumulate cell mean normal vector
            nCell.x += x; nCell.y += y; nCell.z += z;
          }
          // std::cerr << "      nPoints = " << (coordCell.size()/3) << endl;
          if(nPoints<=0) continue;

          // normalize cell mean normal vector
          nCell.x /= ((float)nPoints);
          nCell.y /= ((float)nPoints);
          nCell.z /= ((float)nPoints);

          // fit linear function
          // eigenFit(coordCell,normalCell,minCell,maxCell,fCell);
          meanFit(coordCell,normalCell,minCell,maxCell,fCell);
          if(fCell.x*nCell.x+fCell.y*nCell.y+fCell.z*nCell.z<0.0f) {
            fCell.x = -fCell.x; fCell.y = -fCell.y;
            fCell.z = -fCell.z; fCell.w = -fCell.w;
          }

          // save linear function
          f.push_back(fCell.x);
          f.push_back(fCell.y);
          f.push_back(fCell.z);
          f.push_back(fCell.w);

          // compute isosurface within cell

          // get the 8 cube corner coordinates and function values
          for(i=0;i<8;i++) {
            v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            b[i] = (F[i] = x*fCell.x+y*fCell.y+z*fCell.z+fCell.w)<0.0f;
          }

          // for each of the 12 edges of the cube
          for(i=0;i<12;i++) {
            iV   = -1; // isovertex index associated to this edge
            j    = edgeTable[i][0]; // first  edge cube vertex
            k    = edgeTable[i][1]; // second edge cube vertex
            if(b[j]!=b[k]) {
              // get a new isovertex index
              iV = (int)((coordIfs.size()/3));
              // compute the isovertex coordinates
              tk = F[j]/(F[j]-F[k]);
              tj = F[k]/(F[k]-F[j]);
              x  = tj*v[j].x+tk*v[k].x;
              y  = tj*v[j].y+tk*v[k].y;
              z  = tj*v[j].z+tk*v[k].z;
              // save the isovertex coordinates
              coordIfs.push_back(x);
              coordIfs.push_back(y);
              coordIfs.push_back(z);
            }
            // save the isovertex index
            iE[i] = iV;
          }

          nFacesCell = IsoSurf::makeCellFaces(b,iE,coordIndexIfs);
          // std::cerr << "      nFacesCell = " << nFacesCell << endl;

          normalIfs.push_back(fCell.x);
          normalIfs.push_back(fCell.y);
          normalIfs.push_back(fCell.z);

          // std::cerr << "    }" << endl;
        }
      }
    }
  }

  std::cerr << "  }" << endl;
  std::cerr << "  nonEmptyCells = " << nonEmptyCells << endl;
  std::cerr << "  nFacesCell    = " << nFacesCell    << endl;

  _deletePartition();

  std::cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::fitContinuous
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool isCube) {
  
  char str[256];
  std::cerr << "SceneGraphProcessor::fitContinuous() {" << endl;
  
  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;
  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) {
    // throw exception ?
    return;
  }

  std::cerr << "  computing bounding box" <<  endl;

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(isCube) {
    dx = dMax; dy = dMax; dz = dMax;
  }
  if(scale>0.0f) {
    dx *= scale; dy *= scale; dz *= scale;
  }
  float x,y,z;
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  std::cerr << "  bboxMin = [" << endl;
  sprintf(str,"    %12.6f",x0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y0);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z0);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;
  std::cerr << "  bboxMax = [" << endl;
  sprintf(str,"    %12.6f",x1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",y1);  std::cerr << str << endl;
  sprintf(str,"    %12.6f",z1);  std::cerr << str << endl;
  std::cerr << "  ]" << endl;

  int iCell,iPoint,ix,iy,iz,jx,jy,jz;

  // create a partition of the points as an array of linked lists

  std::cerr << "  creating partition" <<  endl;
  _createPartition(min,max,depth,coordPoints);
  std::cerr << "  depth  = " << depth <<  endl;
  std::cerr << "  nGrid  = " << _nGrid <<  endl;
  std::cerr << "  nCells = " << _nCells() <<  endl;

  // initialize fGrid and wGrid
  vector<float> fGrid;
  vector<float> wGrid;
  int nGridVertices = (_nGrid+1)*(_nGrid+1)*(_nGrid+1);
  fGrid.clear();
  fGrid.insert(fGrid.end(),nGridVertices,0.0f);
  wGrid.clear();
  wGrid.insert(wGrid.end(),nGridVertices,0.0f);

  Vec4f f;
  Vec3f minCell,maxCell,nCell;
  vector<float> coordCell;
  vector<float> normalCell;
  int nonEmptyCells = 0;
  int nFacesCell    = 0;

  std::cerr << "  accumulating grid vertex funtion values {" << endl;

  int iV,N = _nGrid;
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);
        if((iPoint=_first[iCell])>=0) {
          nonEmptyCells++;

          std::cerr << "    cell[" << iCell << "] {" << endl;
          std::cerr << "      (" << ix << "," << iy << "," << iz << ")" << endl;

          // build cell coord array

          coordCell.clear();
          normalCell.clear();
          nCell.x=nCell.y=nCell.z;
          for(;iPoint>=0;iPoint=_next[iPoint]) {
            //
            x = coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y = coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z = coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
            nCell.x += x; nCell.y += y; nCell.z += z;
          }
          std::cerr << "      nPoints = " << (coordCell.size()/3) << endl;
          if(nPoints<=0) continue;
          nCell.x /= ((float)nPoints);
          nCell.y /= ((float)nPoints);
          nCell.z /= ((float)nPoints);

          // eigenFit(coordCell,normalCell,minCell,maxCell,f);
          meanFit(coordCell,normalCell,minCell,maxCell,f);
          if(f.x*nCell.x+f.y*nCell.y+f.z*nCell.z<0.0f) {
            f.x = -f.x; f.y = -f.y;
            f.z = -f.z; f.w = -f.w;
          }

          // accumulate
          iV = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          fGrid[iV] += (f.x*minCell.x+f.y*minCell.y+f.z*minCell.z+f.w);
          wGrid[iV] += 1.0f;
          
          iV = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1));
          fGrid[iV] += (f.x*minCell.x+f.y*minCell.y+f.z*maxCell.z+f.w);
          wGrid[iV] += 1.0f;

          iV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  ));
          fGrid[iV] += (f.x*minCell.x+f.y*maxCell.y+f.z*minCell.z+f.w);
          wGrid[iV] += 1.0f;

          iV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz+1));
          fGrid[iV] += (f.x*minCell.x+f.y*maxCell.y+f.z*maxCell.z+f.w);
          wGrid[iV] += 1.0f;
          
          iV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  ));
          fGrid[iV] += (f.x*maxCell.x+f.y*minCell.y+f.z*minCell.z+f.w);
          wGrid[iV] += 1.0f;

          iV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz+1));
          fGrid[iV] += (f.x*maxCell.x+f.y*minCell.y+f.z*maxCell.z+f.w);
          wGrid[iV] += 1.0f;

          iV = (ix+1)+(N+1)*((iy+1)+(N+1)*(iz  ));
          fGrid[iV] += (f.x*maxCell.x+f.y*maxCell.y+f.z*minCell.z+f.w);
          wGrid[iV] += 1.0f;

          iV = (ix+1)+(N+1)*((iy+1)+(N+1)*(iz+1));
          fGrid[iV] += (f.x*maxCell.x+f.y*maxCell.y+f.z*maxCell.z+f.w);
          wGrid[iV] += 1.0f;

          std::cerr << "    }" << endl;
        }
      }
    }
  }

  std::cerr << "  nonEmptyCells = " << nonEmptyCells << endl;
  std::cerr << "  nFacesCell    = " << nFacesCell << endl;

  std::cerr << "  }" << endl;

  std::cerr << "  normalizing {" << endl;

  // normalize
  for(iV=0;iV<nGridVertices;iV++)
    if(wGrid[iV]>0.0f)
      fGrid[iV] /= wGrid[iV];

  std::cerr << "  }" << endl;

  // create output surface

  IndexedFaceSet* surface = _getNamedShapeIFS("SURFACE",true);
  surface->clear();
  vector<float>& coordIfs      = surface->getCoord();
  vector<int>&   coordIndexIfs = surface->getCoordIndex();
  // vector<float>& normalIfs     = surface->getNormal();
  surface->setNormalPerVertex(false);

  Vec3f v[8]; // bbox corner coordinates
  float F[8]; // function values at bbox corners
  bool  b[8]; // function is positive or negative ?
  float tj,tk;
  int   iE[12],i,j,k;
  const int (*edgeTable)[2] = IsoSurf::getEdgeTable();

  std::cerr << "  computing isosurface {" << endl;

  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);
        if((iPoint=_first[iCell])>=0) {

          for(i=0;i<8;i++) {
            v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            jz = iz+((i>>0)&0x1);
            jy = iy+((i>>1)&0x1);
            jx = ix+((i>>2)&0x1);
            iV = jx+(N+1)*(jy+(N+1)*jz);
            b[i] = ((F[i]=fGrid[iV])<0.0f);
          }

          for(i=0;i<12;i++) {
            iV   = -1;
            j    = edgeTable[i][0];
            k    = edgeTable[i][1];
            if(b[j]!=b[k]) {
              // isvertex index
              iV = (int)((coordIfs.size()/3));
              // isovertex coordinates
              tk = F[j]/(F[j]-F[k]);
              tj = F[k]/(F[k]-F[j]);
              x  = tj*v[j].x+tk*v[k].x;
              y  = tj*v[j].y+tk*v[k].y;
              z  = tj*v[j].z+tk*v[k].z;
              coordIfs.push_back(x);
              coordIfs.push_back(y);
              coordIfs.push_back(z);
            }
            iE[i] = iV;
          }

          nFacesCell = IsoSurf::makeCellFaces(b,iE,coordIndexIfs);
        }
      }
    }
  }

  std::cerr << "  }" << endl;

  // add normal per face
  _computeNormalPerFace(*surface);

  _deletePartition();

  std::cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::fitWatertight
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool isCube,
  vector<float>& fGrid) {

  // char str[256];
  std::cerr << "SceneGraphProcessor::fitWatertight() {" << endl;

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) return;
  
  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;

  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  std::cerr << "  computing bounding box" <<  endl;

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(isCube ) { dx = dMax; dy = dMax; dz = dMax; }
  if(scale>0.0f) { dx *= scale; dy *= scale; dz *= scale; }
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  int iCell,iPoint,ix,iy,iz; //,jx,jy,jz;
  
  // create a partition of the points as an array of linked lists
  
  _createPartition(min,max,depth,coordPoints);

  // initialize fGrid and wGrid
  vector<float> wGrid;
  int nGridVertices = (_nGrid+1)*(_nGrid+1)*(_nGrid+1);
  fGrid.clear();
  fGrid.insert(fGrid.end(),nGridVertices,0.0f);
  wGrid.clear();
  wGrid.insert(wGrid.end(),nGridVertices,0.0f);

  float x,y,z;
  Vec4f fCell;
  Vec3f minCell,maxCell,nCell;
  vector<float> coordCell;
  vector<float> normalCell;
  int nonEmptyCells = 0;
    int jx,jy,jz;

  std::cerr << "  accumulating grid vertex funtion values {" << endl;

  int iV,N = _nGrid;
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);

        if((iPoint=_first[iCell])>=0) {
          nonEmptyCells++;

          // build cell coord array
          coordCell.clear();
          normalCell.clear();
          nCell.x=nCell.y=nCell.z;
          for(;iPoint>=0;iPoint=_next[iPoint]) {
            //
            x = coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y = coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z = coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
            nCell.x += x; nCell.y += y; nCell.z += z;
          }

          if(nPoints<=0) continue;
          nCell.x /= ((float)nPoints);
          nCell.y /= ((float)nPoints);
          nCell.z /= ((float)nPoints);

          meanFit(coordCell,normalCell,minCell,maxCell,fCell);

          // accumulate
          for(int i=0;i<8;i++) {
            z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            jz = iz+((i>>0)&0x1);
            jy = iy+((i>>1)&0x1);
            jx = ix+((i>>2)&0x1);
            iV = jx+(N+1)*(jy+(N+1)*jz);
            fGrid[iV] +=
              (fCell.x*x+fCell.y*y+fCell.z*z+fCell.w);
            wGrid[iV] += 1.0f;
          }
        }
      }
    }
  }
  std::cerr << "  nonEmptyCells = " << nonEmptyCells << endl;
  std::cerr << "  }" << endl;

  std::cerr << "  normalizing {" << endl;

  // normalize
  vector<int> src;
  vector<int> dst;
  for(iV=0;iV<nGridVertices;iV++)
    if(wGrid[iV]>0.0f) {
      fGrid[iV] /= wGrid[iV];
      wGrid[iV] = -2.0f; // to indicate original vertices
      src.push_back(iV);
    }

  std::cerr << "  }" << endl;

  // extend vertex function values to all vertices ???

  int iV0,iV1;
  vector<int> I;
  float fV0;
  while(src.size()>0) {

    // for each vertex in the wavefront
    while(src.size()>0) {
      iV0 = iV = src.back(); src.pop_back(); // wGrid[iV0]<0
      ix = iV%(N+1); iV/=(N+1);
      iy = iV%(N+1); iV/=(N+1);
      iz = iV%(N+1);
      fV0 = fGrid[iV0];
      
      if(  iz<N) I.push_back((ix  )+(N+1)*((iy  )+(N+1)*(iz+1)));
      if(0<iz  ) I.push_back((ix  )+(N+1)*((iy  )+(N+1)*(iz-1)));
      if(  iy<N) I.push_back((ix  )+(N+1)*((iy+1)+(N+1)*(iz  )));
      if(0<iy  ) I.push_back((ix  )+(N+1)*((iy-1)+(N+1)*(iz  )));
      if(  ix<N) I.push_back((ix+1)+(N+1)*((iy  )+(N+1)*(iz  )));
      if(0<ix  ) I.push_back((ix-1)+(N+1)*((iy  )+(N+1)*(iz  )));
      
      while(I.size()>0) {
        iV1 = I.back(); I.pop_back();
        // wGrid[iV0]<0.0f
        if(wGrid[iV1]>=0.0f) {
          if(wGrid[iV1]==0.0f) dst.push_back(iV1);
          fGrid[iV1] += fV0;
          wGrid[iV1] += 1.0f;
        }
      }
    }

    // normalize new function values and create new wavefront
    while(dst.size()>0) {
      iV1 = dst.back(); dst.pop_back();
      // if(wGrid[iV1]>0.0f) {
      fGrid[iV1] /= wGrid[iV1];
      wGrid[iV1] = -1.0f;
      src.push_back(iV1);
      // }
    } // while(dst.size()>0)

  } // while(source.size()>0)

  // create output surface
  computeIsosurface(center,size,depth,scale,isCube,fGrid);

  // we no longer need the point partition
  _deletePartition();

  std::cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::optimalCGHard
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool cube,
 vector<float>& fGrid /* input & output */) {

  // char str[256];
  cerr << "SceneGraphProcessor::optimalCGHard() {" << endl;

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) return;
  
  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;

  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(cube      ) { dx = dMax; dy = dMax; dz = dMax; }
  if(scale>0.0f) { dx *= scale; dy *= scale; dz *= scale; }
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  _createPartition(min,max,depth,coordPoints);
  int N = 1<<depth;
  int nGridVertices = (N+1)*(N+1)*(N+1);
  if((int)fGrid.size()!=nGridVertices) return;

  int iCell,iPoint,ix,iy,iz,i,j,k,iV,jV,kV;

  cerr << "  initialize function values at corners of occupied cells {" << endl;

  Vec4f f;
  float x,y,z;
  Vec3f v[8]; // bbox corner coordinates
  vector<float> coordCell;
  vector<float> normalCell;
  Vec3f minCell,maxCell,nCell;
  vector<float> wGrid;
  wGrid.insert(wGrid.end(),nGridVertices,0.0f);
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);

        if((iPoint=_first[iCell])>=0) {
          coordCell.clear();
          normalCell.clear();
          for(nPoints=0;iPoint>=0;iPoint=_next[iPoint]) {
            x =  coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y =  coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z =  coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
          }

          // fit
          meanFit(coordCell,normalCell,minCell,maxCell,f);
          
          // accumulate
          for(int i=0;i<8;i++) {
            v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            iV = (ix+((i>>2)&0x1))+(N+1)*((iy+((i>>1)&0x1))+(N+1)*(iz+(i&0x1)));
            if(wGrid[iV]==0.0f) fGrid[iV] = 0.0f;
            fGrid[iV] += (f.x*x+f.y*y+f.z*z+f.w);
            wGrid[iV] += 1.0f;
          }
        }

      }
    }
  }

  // normalize
  for(iV=0;iV<nGridVertices;iV++)
    if(wGrid[iV]>0.0f)
      fGrid[iV] /= wGrid[iV];

  cerr << "  }" << endl;

  cerr << "  identifying constrained variables {" << endl;
  //
  vector<int> gridVertex;
  gridVertex.insert(gridVertex.end(),nGridVertices,-1);
  vector<int> iVConstrained;
  vector<int> iVFree;
  for(iCell=iz=0;iz<N;iz++) {
    for(iy=0;iy<N;iy++) {
      for(ix=0;ix<N;ix++,iCell++) {
        if(_first[iCell]>=0) { // if cell is not empty
          for(i=0;i<8;i++) { // for each corner of cell
            iV = (ix+((i>>2)&0x1))+(N+1)*((iy+((i>>1)&0x1))+(N+1)*(iz+(i&0x1)));
            if(gridVertex[iV]<0) { // if first visit
              gridVertex[iV] = (int)(iVConstrained.size()); // save the vertex index
              iVConstrained.push_back(iV);
            }
          }
        }
      }
    }
  }
  //
  for(iV=0;iV<nGridVertices;iV++) {
    if((i=gridVertex[iV])<0) { // if still free
      gridVertex[iV] = (int)(iVFree.size());
      iVFree.push_back(iV);
    } else /* if(iVConstrained[i]==iV) */ {
      gridVertex[iV] = -i-1;
    }
  }
  //
  cerr << "    " << iVConstrained.size() << " constrained variables " << endl;
  cerr << "    " << iVFree.size()        << " free variables        " << endl;
  cerr << "  }" << endl;

  int n = (int)(iVFree.size());
  if(n<=0) return;

  cerr << "  iterative linear solver {" << endl;

  VectorXd X(n),B(n),D(n); // these vectors are not initialized
  B = VectorXd::Zero(n);
  SparseMatrix<double> A(n,n); // this matrix is implicitly initialized to 0's
  A.reserve(VectorXi::Constant(n,8)); // each grid vertex may have up to 8 neighbors

  // fill A and b
  for(iz=0;iz<=N;iz++) {
    for(iy=0;iy<=N;iy++) {
      for(ix=0;ix<=N;ix++) {
        jV = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
        j  = gridVertex[jV]; // free if j>=0
        for(i=0;i<3;i++) {
          if((i==0 && ix==N)||(i==1 && iy==N)||(i==2 && iz==N)) continue;
          kV = jV;
          switch(i) {
          case 0: kV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  )); break;
          case 1: kV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  )); break;
          case 2: kV = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1)); break;
          }
          k = gridVertex[kV]; // free if k>=0
          // for each edge (j,k)
          if(j>=0 && k>=0) { // jV and kV free
            A.coeffRef(j,j) += 1.0; A.coeffRef(j,k) -= 1.0;
            A.coeffRef(k,j) -= 1.0; A.coeffRef(k,k) += 1.0;
          } else if(j>=0) { // jV free kV constrained
            A.coeffRef(j,j) += 1.0; B(j) += fGrid[kV];
          } else if(k>=0) { // jV constrained kV free
            A.coeffRef(k,k) += 1.0; B(k) += fGrid[jV];
          } // nothing to do if both jV and kV are constrained
        }
      }
    }
  }

  ConjugateGradient<SparseMatrix<double> > cg;
  cg.compute(A);
  if(cg.info()!=Success) {
    cerr << "    decomposition failed "<< endl;
  } else {

    // fill x with initial guess
    for(i=0;i<(int)iVFree.size();i++) {
      iV = iVFree[i]; X(i) = fGrid[iV];
    }
    
    // x = cg.solve(b);
    X = cg.solveWithGuess(B,X);
    
    // transfer result back
    for(i=0;i<(int)iVFree.size();i++) {
      iV = iVFree[i]; fGrid[iV] = (float)(X(i));
    }
    
    cerr << "    " << cg.iterations() << " iterations"<< endl;
    cerr << "    " << cg.error() << " estimated error"<< endl;
  }

  cerr << "  }" << endl;

  // update the output surface
  computeIsosurface(center,size,depth,scale,cube,fGrid);

  // we no longer need the point partition
  _deletePartition();

  cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::optimalCGSoft
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool cube,
 vector<float>& fGrid /* input & output */) {

  // char str[256];
  cerr << "SceneGraphProcessor::optimalCGHard() {" << endl;

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) return;
  
  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;

  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(cube      ) { dx = dMax; dy = dMax; dz = dMax; }
  if(scale>0.0f) { dx *= scale; dy *= scale; dz *= scale; }
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  _createPartition(min,max,depth,coordPoints);
  int N = 1<<depth;
  int nGridVertices = (N+1)*(N+1)*(N+1);
  if((int)fGrid.size()!=nGridVertices) return;

  int iCell,iPoint,ix,iy,iz,i,iV,jV,kV;
  
  cerr << "  initialize function values at corners of occupied cells {" << endl;

  Vec4f f;
  float x,y,z;
  Vec3f v[8]; // bbox corner coordinates
  vector<float> coordCell;
  vector<float> normalCell;
  Vec3f minCell,maxCell,nCell;
  vector<float> wGrid;
  wGrid.insert(wGrid.end(),nGridVertices,0.0f);
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);

        if((iPoint=_first[iCell])>=0) {
          coordCell.clear();
          normalCell.clear();
          for(nPoints=0;iPoint>=0;iPoint=_next[iPoint]) {
            x =  coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y =  coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z =  coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
          }

          // fit
          meanFit(coordCell,normalCell,minCell,maxCell,f);
          
          // accumulate
          for(int i=0;i<8;i++) {
            v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            iV = (ix+((i>>2)&0x1))+(N+1)*((iy+((i>>1)&0x1))+(N+1)*(iz+(i&0x1)));
            if(wGrid[iV]==0.0f) fGrid[iV] = 0.0f;
            fGrid[iV] += (f.x*x+f.y*y+f.z*z+f.w);
            wGrid[iV] += 1.0f;
          }
        }

      }
    }
  }

  // normalize
  for(iV=0;iV<nGridVertices;iV++)
    if(wGrid[iV]>0.0f)
      fGrid[iV] /= wGrid[iV];

  cerr << "    " << nGridVertices << " free variables        " << endl;
  cerr << "  }" << endl;

    
  cerr << "  iterative linear solver {" << endl;

  int n = nGridVertices;
  VectorXd X(n),B(n),D(n); // these vectors are not initialized
  B = VectorXd::Zero(n);

  SparseMatrix<double> A(n,n); // this matrix is implicitly initialized to 0's
  A.reserve(VectorXi::Constant(n,8)); // each grid vertex may have up to 8 neighbors

  for(iV=0;iV<n;iV++) {
    if(wGrid[iV]>0.0f) {
      B(iV) = fGrid[iV];
      A.coeffRef(iV,iV) += 1.0;
    }
  }

  double lambda = 0.05;

  // fill A and b
  for(iz=0;iz<=N;iz++) {
    for(iy=0;iy<=N;iy++) {
      for(ix=0;ix<=N;ix++) {
        jV = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
        for(i=0;i<3;i++) {
          if((i==0 && ix==N)||(i==1 && iy==N)||(i==2 && iz==N)) continue;
          kV = jV;
          switch(i) {
          case 0: kV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  )); break;
          case 1: kV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  )); break;
          case 2: kV = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1)); break;
          }
          A.coeffRef(jV,jV) += lambda; A.coeffRef(jV,kV) -= lambda;
          A.coeffRef(kV,jV) -= lambda; A.coeffRef(kV,kV) += lambda;
        }
      }
    }
  }

  ConjugateGradient<SparseMatrix<double> > cg;
  cg.compute(A);
  if(cg.info()!=Success) {
    cerr << "    decomposition failed "<< endl;
  } else {

    // fill x with initial guess
    for(iV=0;iV<n;iV++) {
      X(iV) = fGrid[iV];
    }
    
    // x = cg.solve(b);
    X = cg.solveWithGuess(B,X);
    
    // transfer result back
    for(iV=0;iV<n;iV++) {
      fGrid[iV] = (float)(X(iV));
    }
    
    cerr << "    " << cg.iterations() << " iterations"<< endl;
    cerr << "    " << cg.error() << " estimated error"<< endl;
  }

  cerr << "  }" << endl;

  // update the output surface

  computeIsosurface(center,size,depth,scale,cube,fGrid);

  // we no longer need the point partition

  _deletePartition();

  cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::optimalJacobi
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool cube,
 vector<float>& fGrid /* input & output */) {

  // char str[256];
  cerr << "SceneGraphProcessor::optimalJacobi() {" << endl;

  if(size.x<=0.0f || size.y<=0.0f || size.z<=0.0f || scale<=0.0f) return;
  
  IndexedFaceSet* points = _getNamedShapeIFS("POINTS",false);
  if(points==(IndexedFaceSet*)0) return;
  if(points->getNormalBinding()!=IndexedFaceSet::PB_PER_VERTEX) return;

  vector<float>& coordPoints = points->getCoord();
  vector<float>& normalPoints = points->getNormal();
  int nPoints = (int)(coordPoints.size()/3);

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(cube      ) { dx = dMax; dy = dMax; dz = dMax; }
  if(scale>0.0f) { dx *= scale; dy *= scale; dz *= scale; }
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  _createPartition(min,max,depth,coordPoints);
  int N = 1<<depth;
  int nGridVertices = (N+1)*(N+1)*(N+1);
  if((int)fGrid.size()!=nGridVertices) return;

  int iCell,iPoint,ix,iy,iz,i,iV,jV,kV;
  
  cerr << "  initialize function values at corners of occupied cells {" << endl;

  Vec4f fCell;
  float x,y,z;
  Vec3f v[8]; // bbox corner coordinates
  vector<float> coordCell;
  vector<float> normalCell;
  Vec3f minCell,maxCell,nCell;
  vector<float> wGrid;
  wGrid.insert(wGrid.end(),nGridVertices,0.0f);
  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);

        if((iPoint=_first[iCell])>=0) {
          coordCell.clear();
          normalCell.clear();
          for(nPoints=0;iPoint>=0;iPoint=_next[iPoint]) {
            x =  coordPoints[3*iPoint  ]; coordCell.push_back(x);
            y =  coordPoints[3*iPoint+1]; coordCell.push_back(y);
            z =  coordPoints[3*iPoint+2]; coordCell.push_back(z);
            //
            x = normalPoints[3*iPoint  ]; normalCell.push_back(x);
            y = normalPoints[3*iPoint+1]; normalCell.push_back(y);
            z = normalPoints[3*iPoint+2]; normalCell.push_back(z);
          }

          // fit
          meanFit(coordCell,normalCell,minCell,maxCell,fCell);
          
          // accumulate
          for(int i=0;i<8;i++) {
            v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
            v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
            v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
            iV = (ix+((i>>2)&0x1))+(N+1)*((iy+((i>>1)&0x1))+(N+1)*(iz+(i&0x1)));
            if(wGrid[iV]==0.0f) fGrid[iV] = 0.0f;
            fGrid[iV] += (fCell.x*x+fCell.y*y+fCell.z*z+fCell.w);
            wGrid[iV] += 1.0f;
          }
        }

      }
    }
  }

  // normalize
  for(iV=0;iV<nGridVertices;iV++)
    if(wGrid[iV]>0.0f)
      fGrid[iV] /= wGrid[iV];

  cerr << "    " << nGridVertices << " free variables        " << endl;
  cerr << "  }" << endl;

  int    n      = nGridVertices;
  int    nIter  = 20;
  float  lambda = 0.10f;
  float  mu     = 0.5f;
  vector<float> f;
  vector<float> df;
  vector<float> wf;
  float fErr,wErr;

  // initialize result
  f.insert(f.end(),fGrid.begin(),fGrid.end());

  cerr << "  iterative Jacobi solver {" << endl;

  for(int iIter=0;iIter<nIter;iIter++) {

    // zero accumulators
    df.clear();
    df.insert(df.end(),n,0.0f);
    wf.clear();
    wf.insert(wf.end(),n,0.0f);

    // accumulate soft constraints
    for(iV=0;iV<n;iV++) {
      if(wGrid[iV]>0.0f) {
        df[iV] += (fGrid[iV]-f[iV]);
        wf[iV] += 1.0f;
      }
    }

    // accumulate Laplacian
    for(iz=0;iz<=N;iz++) {
      for(iy=0;iy<=N;iy++) {
        for(ix=0;ix<=N;ix++) {
          jV = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          for(i=0;i<3;i++) {
            if((i==0 && ix==N)||(i==1 && iy==N)||(i==2 && iz==N)) continue;
            kV = jV;
            switch(i) {
            case 0: kV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  )); break;
            case 1: kV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  )); break;
            case 2: kV = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1)); break;
            }
            df[jV] += lambda*(f[kV]-f[jV]);
            wf[jV] += lambda;
            df[kV] += lambda*(f[jV]-f[kV]);
            wf[kV] += lambda;
          }
        }
      }
    }

    // normalize
    for(iV=0;iV<n;iV++)
      df[iV] /= wf[iV];

    // update
    for(iV=0;iV<n;iV++)
      f[iV] += mu*df[iV];
    
    // measure error
    fErr = wErr = 0.0f;
    for(iV=0;iV<n;iV++)
      if(wGrid[iV]>0.0f) {
        fErr += (fGrid[iV]-f[iV])*(fGrid[iV]-f[iV]);
        wErr += 1.0f;
      }
    for(iz=0;iz<=N;iz++) {
      for(iy=0;iy<=N;iy++) {
        for(ix=0;ix<=N;ix++) {
          jV = (ix  )+(N+1)*((iy  )+(N+1)*(iz  ));
          for(i=0;i<3;i++) {
            if((i==0 && ix==N)||(i==1 && iy==N)||(i==2 && iz==N)) continue;
            kV = jV;
            switch(i) {
            case 0: kV = (ix+1)+(N+1)*((iy  )+(N+1)*(iz  )); break;
            case 1: kV = (ix  )+(N+1)*((iy+1)+(N+1)*(iz  )); break;
            case 2: kV = (ix  )+(N+1)*((iy  )+(N+1)*(iz+1)); break;
            }
            fErr += lambda*(f[kV]-f[jV])*(f[kV]-f[jV]);
            wErr += lambda;
          }
        }
      }
    }
    fErr /= wErr;

    cerr << "    err = " << fErr << endl;
  }

  // save result
  fGrid.clear();
  fGrid.insert(fGrid.end(),f.begin(),f.end());

  cerr << "  }" << endl;

  // update the output surface

  computeIsosurface(center,size,depth,scale,cube,fGrid);

  // we no longer need the point partition

  _deletePartition();

  cerr << "}" << endl;
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::multiGridFiner
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool cube,
 vector<float>& fGridIn /* input & output */) {
  if(depth<10) {
    
    int nGridIn         = 1<<depth;
    // int nGridInCells    = nGridIn*nGridIn*nGridIn;
    // int nGridInVertices = (nGridIn+1)*(nGridIn+1)*(nGridIn+1);

    // assert(fGridIn.size()==nGridVertices);

    int nGridOut         = 1<<(depth+1);
    // int nGridOutCells    = nGridOut*nGridOut*nGridOut;
    int nGridOutVertices = (nGridOut+1)*(nGridOut+1)*(nGridOut+1);

    vector<float> fGridOut;
    fGridOut.insert(fGridOut.end(),nGridOutVertices,0.0f);

    int iV0,iV1,iV2,iV3,iV4,iV5,iV6,iV7,iV8,i,j,k;

    // copy each input vertex value onto corresponding output vertex value 
    for(k=0;k<=nGridIn;k++) {
      for(j=0;j<=nGridIn;j++) {
        for(i=0;i<=nGridIn;i++) {
          iV0 = (i  )+(nGridIn+1)*((j  )+(nGridIn+1)*(k  ));
          // vertex
          iV1 = (2*i)+(nGridOut+1)*((2*j)+(nGridOut+1)*(2*k));
          fGridOut[iV1] = fGridIn[iV0];
          // edges
          // [iV0] --- [iV2] --- [iV1]
          if(k<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV2 = (2*i  )+(nGridOut+1)*((2*j  )+(nGridOut+1)*(2*k+1));            
            fGridOut[iV2] = (fGridIn[iV0]+fGridIn[iV1])/2.0f;
          }
          if(j<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV2 = (2*i  )+(nGridOut+1)*((2*j+1)+(nGridOut+1)*(2*k  ));
            fGridOut[iV2] = (fGridIn[iV0]+fGridIn[iV1])/2.0f;
          }
          if(i<nGridIn) {
            iV1 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k  ));
            iV2 = (2*i+1)+(nGridOut+1)*((2*j  )+(nGridOut+1)*(2*k  ));
            fGridOut[iV2] = (fGridIn[iV0]+fGridIn[iV1])/2.0f;
          }
          // faces
          // [iV0] ------- [iV1]
          //   |    [iV4]    |
          // [iV2] ------- [iV3]
          if(k<nGridIn &&  j<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV2 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV3 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k+1));
            iV4 = (2*i  )+(nGridOut+1)*((2*j+1)+(nGridOut+1)*(2*k+1));            
            fGridOut[iV4] =
              (fGridIn[iV0]+fGridIn[iV1]+fGridIn[iV2]+fGridIn[iV3])/4.0f;
          }
          if(k<nGridIn &&  i<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV2 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k  ));
            iV3 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV4 = (2*i+1)+(nGridOut+1)*((2*j  )+(nGridOut+1)*(2*k+1));            
            fGridOut[iV4] =
              (fGridIn[iV0]+fGridIn[iV1]+fGridIn[iV2]+fGridIn[iV3])/4.0f;
          }
          if(j<nGridIn &&  i<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV2 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k  ));
            iV3 = (  i+1)+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV4 = (2*i+1)+(nGridOut+1)*((2*j+1)+(nGridOut+1)*(2*k  ));
            fGridOut[iV4] =
              (fGridIn[iV0]+fGridIn[iV1]+fGridIn[iV2]+fGridIn[iV3])/4.0f;
          }
          // cell
          if(k<nGridIn &&  j<nGridIn && i<nGridIn) {
            iV1 = (  i  )+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV2 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV3 = (  i  )+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k+1));
            iV4 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k  ));
            iV5 = (  i+1)+(nGridIn +1)*((  j  )+(nGridIn +1)*(  k+1));
            iV6 = (  i+1)+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k  ));
            iV7 = (  i+1)+(nGridIn +1)*((  j+1)+(nGridIn +1)*(  k+1));
            iV8 = (2*i+1)+(nGridOut+1)*((2*j+1)+(nGridOut+1)*(2*k+1));
            fGridOut[iV8] =
              (fGridIn[iV0]+fGridIn[iV1]+fGridIn[iV2]+fGridIn[iV3]+
               fGridIn[iV4]+fGridIn[iV5]+fGridIn[iV6]+fGridIn[iV7])/8.0f;
          }
        }
      }
    }
    
    // create output surface
    computeIsosurface(center,size,depth+1,scale,cube,fGridOut);
    fGridIn.swap(fGridOut);
  }
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::multiGridCoarser
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool cube,
 vector<float>& fGridIn /* input & output */) {
  if(depth>1) {
    
    int nGridIn         = 1<<depth;
    // int nGridInCells    = nGridIn*nGridIn*nGridIn;
    // int nGridInVertices = (nGridIn+1)*(nGridIn+1)*(nGridIn+1);

    // assert(fGridIn.size()==nGridVertices);

    int nGridOut         = 1<<(depth-1);
    // int nGridOutCells    = nGridOut*nGridOut*nGridOut;
    int nGridOutVertices = (nGridOut+1)*(nGridOut+1)*(nGridOut+1);

    vector<float> fGridOut;
    fGridOut.insert(fGridOut.end(),nGridOutVertices,0.0f);

    int iV0,iV1,i,j,k;

    // copy each input vertex value onto corresponding output vertex value 
    for(k=0;k<=nGridOut;k++) {
      for(j=0;j<=nGridOut;j++) {
        for(i=0;i<=nGridOut;i++) {
          iV1 = (i  )+(nGridOut+1)*((j  )+(nGridOut+1)*(k  ));
          // vertex
          iV0 = (2*i)+(nGridIn+1)*((2*j)+(nGridIn+1)*(2*k));
          fGridOut[iV1] = fGridIn[iV0];
        }
      }
    }
    
    // create output surface
    computeIsosurface(center,size,depth-1,scale,cube,fGridOut);
    fGridIn.swap(fGridOut);
  }
}

//////////////////////////////////////////////////////////////////////
void SceneGraphProcessor::computeIsosurface
(const Vec3f& center, const Vec3f& size,
 const int depth, const float scale, const bool isCube,
  vector<float>& fGrid) {
  
  // char str[256];
  std::cerr << "SceneGraphProcessor::computeIsosurface() {" << endl;

  float dx = size.x/2.0f;
  float dy = size.y/2.0f;
  float dz = size.z/2.0f;
  float dMax = dx; if(dy>dMax) dMax=dy; if(dz>dMax) dMax=dz;
  if(isCube) {
    dx = dMax; dy = dMax; dz = dMax;
  }
  if(scale>0.0f) {
    dx *= scale; dy *= scale; dz *= scale;
  }
  float x,y,z;
  float x0 = center.x-dx; float y0 = center.y-dy; float z0 = center.z-dz;
  float x1 = center.x+dx; float y1 = center.y+dy; float z1 = center.z+dz;
  Vec3f min(x0,y0,z0);
  Vec3f max(x1,y1,z1);

  int iCell,ix,iy,iz,jx,jy,jz;

  // create output surface

  IndexedFaceSet*   surface       = _getNamedShapeIFS("SURFACE",true);
  surface->clear();
  vector<float>&    coordIfs      = surface->getCoord();
  vector<int>&      coordIndexIfs = surface->getCoordIndex();
  // vector<float>& normalIfs     = surface->getNormal();
  surface->setNormalPerVertex(false);

  int   iVB[8]; // bbox corner vertex indices
  Vec3f v[8];   // bbox corner coordinates
  float F[8];   // function values at bbox corners
  bool  b[8];   // function is positive or negative ?
  float tj,tk;
  int   iE[12],i,j,k,iV,N = 1<<depth;
  // (int[2])*
  const int (*edgeTable)[2] = IsoSurf::getEdgeTable();
  
  SimpleGraphMap graph; // stores map from grid edges to isovertices

  Vec4f f;
  Vec3f minCell,maxCell,nCell;
  vector<float> coordCell;
  vector<float> normalCell;
  int nFacesCell    = 0;

  std::cerr << "  computing isosurface {" << endl;

  for(iCell=iz=0;iz<N;iz++) {
    minCell.z = (((float)(N-iz  ))*z0+((float)(iz  ))*z1)/((float)N);
    maxCell.z = (((float)(N-iz-1))*z0+((float)(iz+1))*z1)/((float)N);
    for(iy=0;iy<N;iy++) {
      minCell.y = (((float)(N-iy  ))*y0+((float)(iy  ))*y1)/((float)N);
      maxCell.y = (((float)(N-iy-1))*y0+((float)(iy+1))*y1)/((float)N);
      for(ix=0;ix<N;ix++,iCell++) {
        minCell.x = (((float)(N-ix  ))*x0+((float)(ix  ))*x1)/((float)N);
        maxCell.x = (((float)(N-ix-1))*x0+((float)(ix+1))*x1)/((float)N);
        
        for(i=0;i<8;i++) {
          v[i].z = z = (((i>>0)&0x1)==0)?minCell.z:maxCell.z;
          v[i].y = y = (((i>>1)&0x1)==0)?minCell.y:maxCell.y;
          v[i].x = x = (((i>>2)&0x1)==0)?minCell.x:maxCell.x;
          jz = iz+((i>>0)&0x1);
          jy = iy+((i>>1)&0x1);
          jx = ix+((i>>2)&0x1);
          iVB[i] = iV = jx+(N+1)*(jy+(N+1)*jz);
          b[i] = ((F[i]=fGrid[iV])<0.0f);
        }
        
        for(i=0;i<12;i++) {
          iV   = -1;
          j    = edgeTable[i][0];
          k    = edgeTable[i][1];
          if(b[j]!=b[k]) {
            // look for the index of the isovertex associated with this grid edge
            iV = graph.get(iVB[j],iVB[k]);
            if(iV<0) { // need to create a new vertex
              iV = (int)((coordIfs.size()/3));
              // isovertex coordinates
              tk = F[j]/(F[j]-F[k]);
              tj = F[k]/(F[k]-F[j]);
              x  = tj*v[j].x+tk*v[k].x;
              y  = tj*v[j].y+tk*v[k].y;
              z  = tj*v[j].z+tk*v[k].z;
              coordIfs.push_back(x);
              coordIfs.push_back(y);
              coordIfs.push_back(z);
              // save the association
              graph.insert(iVB[j],iVB[k],iV);
            }
            
          }
          iE[i] = iV;
        }
        
        nFacesCell = IsoSurf::makeCellFaces(b,iE,coordIndexIfs);
      }
    }
  }

  std::cerr << "  }" << endl;

  // add normal per face
  _computeNormalPerFace(*surface);

  std::cerr << "}" << endl;
}
