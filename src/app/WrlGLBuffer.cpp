//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-13 13:21:00 taubin>
//------------------------------------------------------------------------
//
// WrlGLBuffer.cpp
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
#include <math.h>
#include "WrlGLBuffer.hpp"

//////////////////////////////////////////////////////////////////////
WrlGLBuffer::WrlGLBuffer():
  QOpenGLBuffer(),
  _type(MATERIAL),
  _nVertices(0),
  _nNormals(0),
  _nColors(0),
  _hasFaces(false),
  _hasPolylines(false),
  _hasColor(false),
  _hasNormal(false) {
}

//////////////////////////////////////////////////////////////////////
WrlGLBuffer::WrlGLBuffer(IndexedFaceSet* pIfs, QColor& materialColor):
  QOpenGLBuffer(),
  _nVertices(0),
  _nNormals(0),
  _nColors(0),
  _hasFaces(false),
  _hasPolylines(false),
  _hasColor(false),
  _hasNormal(false) {

  // std::cout << "WrlGLBuffer::WrlGLBuffer(IndexedFaceSet) {\n";

  QVector<QVector3D> m_vertices;
  QVector<QVector3D> m_normals;
  QVector<QVector3D> m_colors;

  if(pIfs==(IndexedFaceSet*)0) return;

  vector<float>& coord       = pIfs->getCoord();
  vector<int>&   coordIndex  = pIfs->getCoordIndex();

  bool           colorPerVertex = pIfs->getColorPerVertex();
  vector<float>& color       = pIfs->getColor();
  vector<int>&   colorIndex  = pIfs->getColorIndex();
  // IndexedFaceSet::Binding   cBinding    = pIfs->getColorBinding();

  bool           normalPerVertex = pIfs->getNormalPerVertex();
  vector<float>& normal      = pIfs->getNormal();
  vector<int>&   normalIndex = pIfs->getNormalIndex();
  // IndexedFaceSet::Binding   nBinding    = pIfs->getNormalBinding();

  // int         nV          = pIfs->getNumberOfCoord();
  int            nF          = pIfs->getNumberOfFaces();

  // material color values in [0.0:1.0] range
  qreal matR,matG,matB,matA;
  materialColor.getRgbF(&matR,&matG,&matB,&matA);
  QVector3D matColor(matR,matG,matB);

  _hasFaces  = (nF>0);
  _hasNormal = (normal.size()>0); // (nBinding!=IndexedFaceSet::Binding::PB_NONE);
  _hasColor  = (color.size()>0);  // cBinding!=IndexedFaceSet::Binding::PB_NONE);

  _type =
    (_hasColor)?
    ((_hasNormal)?COLOR_NORMAL:COLOR):((_hasNormal)?MATERIAL_NORMAL:MATERIAL);

  m_vertices.clear();
  m_normals.clear();
  m_colors.clear();

  if(_hasFaces) {
    // polygon mesh

    float x[3][3];
    float n[3][3];
    float c[3][3];
    int   j[3];

    int iN,iC,iV,k,h,i0,i1,iF;
    for(iF=i0=i1=0;i1<(int)coordIndex.size();i1++) {
      if(coordIndex[i1]<0) {
        // number of triangles in this face
        // nTrianglesFace = i1-i0-2;

        if(_hasNormal && normalPerVertex==false) {
          // NORMAL_PER_FACE_INDEXED or NORMAL_PER_FACE
          iN = (normalIndex.size()>0)?normalIndex[iF]:iF;
          for(h=0;h<3;h++)
            n[0][h] = n[1][h] = n[2][h] = normal[3*iN+h];
        }

        if(_hasColor && colorPerVertex==false) {
          // COLOR_PER_FACE_INDEXED or COLOR_PER_FACE
          iC = (colorIndex.size()>0)?colorIndex[iF]:iF;
          for(h=0;h<3;h++)
            c[0][h] = c[1][h] = c[2][h] = color[3*iC+h];
        }

        // triangulate face [i0:i1) on the fly and add triangles to current mesh
        for(j[0]=i0,j[1]=i0+1,j[2]=i0+2;j[2]<i1;j[1]=j[2]++) {
          // triangle [j0,j1,j2]
          for(k=0;k<3;k++) {
            // get vertex coordinates
            iV = coordIndex[j[k]];
            for(h=0;h<3;h++)
              x[k][h] = coord[3*iV+h];

            if(_hasNormal && normalPerVertex==true) {
              // NORMAL_PER_CORNER or NORNAL_PER_VERTEX
              iN = (normalIndex.size()>0)?normalIndex[j[k]]:iV;
              for(h=0;h<3;h++)
                n[k][h] = normal[3*iN+h];
            }

            if(_hasColor && colorPerVertex==true) {
              // COLOR_PER_CORNER or COLOR_PER_VERTEX
              iC = (colorIndex.size()>0)?colorIndex[j[k]]:iV;
              for(h=0;h<3;h++)
                c[k][h] = color[3*iC+h];
            }

          }

          // push values into buffers
          for(k=2;k>=0;k--) {
            m_vertices.append(QVector3D(x[k][0],x[k][1],x[k][2]));
            if(_hasNormal)
              m_normals.append(QVector3D(n[k][0],n[k][1],n[k][2]));
            if(_hasColor)
              m_colors.append(QVector3D(c[k][0],c[k][1],c[k][2]));
          }
        }

        // advance to next face
        i0 = i1+1; iF++;
      }
    }

  } else /*if(!_hasFaces)*/ {
    
    // treat as point cloud

    // assert(normalPerVertex==true);
    // assert(normalIndex.size()==0);
    // assert(normal.size()==0 || normal.size()==coord.size());

    // assert(colorPerVertex==true);
    // assert(colorIndex.size()==0);
    // assert(color.size()==0 || color.size()==coord.size());

    unsigned iV,nVertices;
    nVertices = pIfs->getNumberOfCoord();
    float x[3],n[3],c[3];
    for(iV=0;iV<nVertices;iV++) {
      x[0] = coord[3*iV  ];
      x[1] = coord[3*iV+1];
      x[2] = coord[3*iV+2];
      m_vertices.append(QVector3D(x[0],x[1],x[2]));
      if(_hasNormal) {
        n[0] = normal[3*iV  ];
        n[1] = normal[3*iV+1];
        n[2] = normal[3*iV+2];
        m_normals.append(QVector3D(n[0],n[1],n[2]));
      }
      if(_hasColor) {
        c[0] = color[3*iV  ];
        c[1] = color[3*iV+1];
        c[2] = color[3*iV+2];
        m_colors.append(QVector3D(c[0],c[1],c[2]));
      }
    }
  }

  _nVertices = m_vertices.count();
  _nNormals  = m_normals.count();
  _nColors   = m_colors.count();

  // Use a vertex buffer object.

  this->create();
  this->bind();
  QVector<GLfloat> buf;
  buf.resize(3*_nVertices+3*_nNormals+3*_nColors);

  GLfloat *p = buf.data();
  for (unsigned i = 0; i < _nVertices; ++i) {
    // vertex coordinates
    *p++ = m_vertices[i].x();
    *p++ = m_vertices[i].y();
    *p++ = m_vertices[i].z();
    // vertex normals
    if(_nNormals>0) {
      *p++ = m_normals[i].x();
      *p++ = m_normals[i].y();
      *p++ = m_normals[i].z();
    }
    // vertex colors
    if(_nColors>0) {
      *p++ = m_colors[i].x();
      *p++ = m_colors[i].y();
      *p++ = m_colors[i].z();
    }
  }
  this->allocate(buf.constData(), buf.count() * sizeof(GLfloat));
  this->release();

  // std::cout << "  _nVertices    = " << _nVertices << "\n";
  // std::cout << "  _nNormals     = " << _nNormals << "\n";
  // std::cout << "  _nColors      = " << _nColors << "\n";
  // std::cout << "  _hasFaces     = " << _hasFaces << "\n";
  // std::cout << "  _hasPolylines = " << _hasPolylines << "\n";
  // std::cout << "  _hasColor     = " << _hasColor << "\n";
  // std::cout << "  _hasNormal    = " << _hasNormal << "\n";

  // std::cout << "}\n";
}

//////////////////////////////////////////////////////////////////////
WrlGLBuffer::WrlGLBuffer(IndexedLineSet* pIls, QColor& materialColor):
  QOpenGLBuffer(),
  _nVertices(0),
  _nNormals(0),
  _nColors(0),
  _hasFaces(false),
  _hasPolylines(false),
  _hasColor(false),
  _hasNormal(false) {

  // std::cout << "WrlGLBuffer::WrlGLBuffer(IndexedLineSet) {\n";

  QVector<QVector3D> m_vertices;
  QVector<QVector3D> m_normals;
  QVector<QVector3D> m_colors;

  if(pIls==(IndexedLineSet*)0) return;

  vector<float>& coord          = pIls->getCoord();
  vector<int>&   coordIndex     = pIls->getCoordIndex();
  vector<float>& color          = pIls->getColor();
  vector<int>&   colorIndex     = pIls->getColorIndex();
  bool           colorPerVertex = pIls->getColorPerVertex();
  // int         nV             = pIls->getNumberOfCoord();
  int            nP             = pIls->getNumberOfPolylines();

  // material color values in [0.0:1.0] range
  qreal matR,matG,matB,matA;
  materialColor.getRgbF(&matR,&matG,&matB,&matA);
  QVector3D matColor(matR,matG,matB);

  _hasPolylines = (nP>0);
  _hasColor     = (color.size()>0);

  _type = (_hasColor)?COLOR:MATERIAL;

  m_vertices.clear();
  m_normals.clear();
  m_colors.clear();

  if(_hasPolylines) {

    float x[2][3];
    float c[2][3];
    int   j[2];

    int iC,iV,k,h,i0,i1,iP; // ,nPolylineEdges;
    for(iP=i0=i1=0;i1<(int)coordIndex.size();i1++) {
      if(coordIndex[i1]<0) {
        // nPolylineEdges = i1-i0-1;
        if(_hasColor && colorPerVertex==false) {
          // get color index for this polyline
          iC = (colorIndex.size()>0)?colorIndex[iP]:iP;
          // assign the same color to the two vertices
          for(h=0;h<3;h++)
            c[0][h] = c[1][h] = color[3*iC+h];
        }

        // for each edge in the polyline
        for(j[0]=i0,j[1]=i0+1;j[1]<i1;j[0]=j[1]++) {
          // edge [j0,j1]
          for(k=0;k<2;k++) {
            // get vertex coordinates
            iV = coordIndex[j[k]];
            for(h=0;h<3;h++)
              x[k][h] = coord[3*iV+h];
            // get color per vertex or per corner
            if(_hasColor && colorPerVertex==true) {
              iC = (colorIndex.size()>0)?colorIndex[j[k]]:iV;
              for(h=0;h<3;h++)
                c[k][h] = color[3*iC+h];
            }
          }

          // push values onto buffers
          for(k=1;k>=0;k--) {
            m_vertices.append(QVector3D(x[k][0],x[k][1],x[k][2]));
            if(_hasColor) m_colors.append(QVector3D(c[k][0],c[k][1],c[k][2]));
          }
        }

        // advance to next polyline
        i0 = i1+1; iP++;
      }
    }

  } else /*if(!_hasPolylines)*/ {

    // treat as point cloud

    unsigned iV,iC,nVertices;
    nVertices = pIls->getNumberOfCoord();
    float x[3];
    float c[3];
    for(iV=0;iV<nVertices;iV++) {
      x[0] = coord[3*iV  ];
      x[1] = coord[3*iV+1];
      x[2] = coord[3*iV+2];
      m_vertices.append(QVector3D(x[0],x[1],x[2]));
      if(color.size()>0) {
        iC = (colorIndex.size()>0)?colorIndex[iV]:iV;
        c[0] = color[3*iC  ];
        c[1] = color[3*iC+1];
        c[2] = color[3*iC+2];
        m_colors.append(QVector3D(c[0],c[1],c[2]));
      }
    }
  }

  _nVertices = m_vertices.count();
  _nColors   = m_colors.count();

  // Use a vertex buffer object.

  this->create();
  this->bind();
  QVector<GLfloat> buf;
  buf.resize(3*_nVertices+3*_nColors);

  GLfloat *p = buf.data();
  for (unsigned i = 0; i < _nVertices; ++i) {
    // vertex coordinates
    *p++ = m_vertices[i].x();
    *p++ = m_vertices[i].y();
    *p++ = m_vertices[i].z();
    // vertex colors
    if(_nColors>0) {
      *p++ = m_colors[i].x();
      *p++ = m_colors[i].y();
      *p++ = m_colors[i].z();
    }
  }
  this->allocate(buf.constData(), buf.count() * sizeof(GLfloat));
  this->release();

  // std::cout << "  _nVertices    = " << _nVertices << "\n";
  // std::cout << "  _nNormals     = " << _nNormals << "\n";
  // std::cout << "  _nColors      = " << _nColors << "\n";
  // std::cout << "  _hasFaces     = " << _hasFaces << "\n";
  // std::cout << "  _hasPolylines = " << _hasPolylines << "\n";
  // std::cout << "  _hasColor     = " << _hasColor << "\n";
  // std::cout << "  _hasNormal    = " << _hasNormal << "\n";

  // std::cout << "}\n";
}
