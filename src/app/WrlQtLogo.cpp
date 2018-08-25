//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 11:27:52 taubin>
//------------------------------------------------------------------------
//
// QtLogo.cpp
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

#include <math.h>
#include "WrlQtLogo.hpp"

#include "wrl/Shape.h"
#include "wrl/Appearance.h"
#include "wrl/Material.h"

//////////////////////////////////////////////////////////////////////
WrlQtLogo::WrlQtLogo():
  SceneGraph(),
  _pIfs((IndexedFaceSet*)0) {

  Shape* shape = new Shape();
  std::string name = "QtLogo";
  shape->setName(name);
  addChild(shape);
  Appearance* appearance = new Appearance();
  shape->setAppearance(appearance);
  Material* material = new Material();
  Color diffuseColor(1.0f,0.6f,0.3f); // (225, 150, 75);
  material->setDiffuseColor(diffuseColor);
  appearance->setMaterial(material);
  _pIfs = new IndexedFaceSet();
  shape->setGeometry(_pIfs);

  _pIfs->setNormalPerVertex(false);

  qreal x1 = +0.06f;
  qreal y1 = -0.14f;
  qreal x2 = +0.14f;
  qreal y2 = -0.06f;
  qreal x3 = +0.08f;
  qreal y3 = +0.00f;
  qreal x4 = +0.30f;
  qreal y4 = +0.22f;

  newQuad(x1, y1, x2, y2, y2, x2, y1, x1);
  newQuad(x3, y3, x4, y4, y4, x4, y3, x3);

  newExtrude(x1, y1, x2, y2);
  newExtrude(x2, y2, y2, x2);
  newExtrude(y2, x2, y1, x1);
  newExtrude(y1, x1, x1, y1);
  newExtrude(x3, y3, x4, y4);
  newExtrude(x4, y4, y4, x4);
  newExtrude(y4, x4, y3, x3);

  const qreal Pi = 3.14159f;
  const int NumSectors = 100;

  for (int i = 0; i < NumSectors; ++i) {
    qreal angle1 = (i * 2 * Pi) / NumSectors;
    qreal x5 = 0.30 * sin(angle1);
    qreal y5 = 0.30 * cos(angle1);
    qreal x6 = 0.20 * sin(angle1);
    qreal y6 = 0.20 * cos(angle1);

    qreal angle2 = ((i + 1) * 2 * Pi) / NumSectors;
    qreal x7 = 0.20 * sin(angle2);
    qreal y7 = 0.20 * cos(angle2);
    qreal x8 = 0.30 * sin(angle2);
    qreal y8 = 0.30 * cos(angle2);

    newQuad(x5, y5, x6, y6, x7, y7, x8, y8);

    newExtrude(x6, y6, x7, y7);
    newExtrude(x8, y8, x5, y5);
  }

  vector<float>& coord = _pIfs->getCoord();
  for (unsigned i = 0;i < coord.size();i++)
    coord[i] *= 2.0f;

  updateBBox();
}

int WrlQtLogo::newVertex(float x, float y, float z) {
  vector<float>& coord  = _pIfs->getCoord();
  int iV = coord.size()/3;
  coord.push_back(x); coord.push_back(y); coord.push_back(z);
  return iV;
}

int WrlQtLogo::newNormal(float nx, float ny, float nz) {
  vector<float>& normal = _pIfs->getNormal();
  int iN = normal.size()/3;
  normal.push_back(nx); normal.push_back(ny); normal.push_back(nz);
  return iN;
}

void WrlQtLogo::newTriangle(int i0, int i1, int i2) {
  vector<int>& coordIndex = _pIfs->getCoordIndex();
  coordIndex.push_back(i0);
  coordIndex.push_back(i1);
  coordIndex.push_back(i2);
  coordIndex.push_back(-1);
}


//////////////////////////////////////////////////////////////////////
void WrlQtLogo::newQuad(qreal x1, qreal y1, qreal x2, qreal y2,
                             qreal x3, qreal y3, qreal x4, qreal y4) {

  int i1m = newVertex(x1, y1, -0.05f);
  int i2m = newVertex(x2, y2, -0.05f);
  int i3m = newVertex(x3, y3, -0.05f);
  int i4m = newVertex(x4, y4, -0.05f);

  QVector3D v1(x2 - x1, y2 - y1, 0.0f);
  QVector3D v2(x4 - x1, y4 - y1, 0.0f);
  QVector3D nm = QVector3D::normal(v1, v2);

  newTriangle(i1m,i2m,i4m);
  newNormal(nm.x(),nm.y(),nm.z());
  newTriangle(i3m,i4m,i2m);
  newNormal(nm.x(),nm.y(),nm.z());

  int i4p = newVertex(x4, y4, 0.05f);
  int i3p = newVertex(x3, y3, 0.05f);
  int i2p = newVertex(x2, y2, 0.05f);
  int i1p = newVertex(x1, y1, 0.05f);

  QVector3D v3(x2 - x4, y2 - y4, 0.0f);
  QVector3D v4(x1 - x4, y1 - y4, 0.0f);
  QVector3D np = QVector3D::normal(v3, v4);

  newTriangle(i4p,i2p,i1p);
  newNormal(np.x(),np.y(),np.z());
  newTriangle(i2p,i4p,i3p);
  newNormal(np.x(),np.y(),np.z());
}

//////////////////////////////////////////////////////////////////////
void WrlQtLogo::newExtrude(qreal x1, qreal y1, qreal x2, qreal y2) {
  int i1p = newVertex(x1, y1, +0.05f);
  int i1m = newVertex(x1, y1, -0.05f);
  int i2p = newVertex(x2, y2, +0.05f);
  int i2m = newVertex(x2, y2, -0.05f);

  QVector3D v1(x2 - x1, y2 - y1, 0.0f);
  QVector3D v2(0.0f, 0.0f, -0.1f);
  QVector3D n = QVector3D::normal(v1, v2);

  newTriangle(i1p,i2p,i1m);
  newNormal(n.x(),n.y(),n.z());
  newTriangle(i2m,i1m,i2p);
  newNormal(n.x(),n.y(),n.z());
}
