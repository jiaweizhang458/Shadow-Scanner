//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-04-30 02:00:50 taubin>
//------------------------------------------------------------------------
//
// ImgDraw.cpp
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
#include <util/Mesh.hpp>
#include <util/GraphPlanar.hpp>
#include <util/GraphEdge.hpp>
#include <util/GraphTraversal.hpp>
#include "ImgDraw.hpp"

ImgDraw::ImgDraw(Img& img):
  _img(img),
  _bgColor(0xff555555),
  _fgColor(0xffffffff),
  _edgeColor(0x7fff7f),
  _edgeThickness(0.0f),
  _vertexColor(0xff7f7f),
  _vertexRadius(2.0f) {
}

void ImgDraw::setBgColor(int value)           { _bgColor        = value; }
void ImgDraw::setFgColor(int value)           { _fgColor        = value; }
void ImgDraw::setEdgeColor(int value)         { _edgeColor      = value; }
void ImgDraw::setEdgeThickness(float value)   { _edgeThickness  = value; }
void ImgDraw::setVertexColor(int value)       { _vertexColor    = value; }
void ImgDraw::setVertexRadius(float value)    { _vertexRadius   = value; }

void ImgDraw::edge(int x0, int y0, int x1, int y1) {
  edge(_img,x0,y0,x1,y1,_edgeColor,_edgeThickness);
}
  
void ImgDraw::vertex(int x0, int y0) {
  vertex(_img,x0,y0,_vertexColor,_vertexRadius);
}

void ImgDraw::edge
(Img& img, int x0, int y0, int x1, int y1, int color, float thickness) {
  if(thickness<0.0f) thickness = 0.0f;
  // int th     = (int)ceil(thickness);
  int width  = img.getWidth();
  int height = img.getHeight();
  // compute bbox
  int xmin,xmax,ymin,ymax;
  if(x0<x1) { xmin=x0; xmax=x1; } else { xmin=x1; xmax=x0; }
  if(y0<y1) { ymin=y0; ymax=y1; } else { ymin=y1; ymax=y0; }
  // return if bbox does not intersect screen
  if(xmax<0 || xmin>=width || ymax<0 || ymin>=height) return;
    
  int xside = xmax-xmin; // +1???
  int yside = ymax-ymin; // +1???

  if(ymax>=height) ymax = height-1;
  if(xmax>= width) xmax =  width-1;

  int x,dx,y,dy;

  int mdx=0,mdy=0;
  if(yside<=xside) {
    if(x0<=x1) {
      x  = x0; dx = 1;
      y  = y0; dy = (y1>y0)?1:-1;
    } else { // x0>x1
      x  = x1; dx = 1;
      y  = y1; dy = (y0>y1)?1:-1;
    }
    while(x<=xmax) {
        if(x>=0 && 0<=y&&y<height) {
          if(thickness<=0) {
            img.set(x,y,color);
          } else {
            vertex(img,x,y,color,thickness);
          }
        }
      x+=dx;
      mdy += yside; if((mdy<<1)>xside) { y+=dy; mdy -= xside; }
    }
  } else { // if(xside<yside)
    if(y0<=y1) {
      y = y0; dy = 1;
      x = x0; dx = (x1>x0)?1:-1;
    } else { // y0>y1
      y = y1; dy = 1;
      x = x1; dx = (x0>x1)?1:-1;
    }
    while(y<=ymax) {
      if(y>=0 && 0<=x&&x<width) {
        if(thickness<=0)
          img.set(x,y,color);
        else
          vertex(img,x,y,color,thickness);
      }
      y+=dy;
      mdx += xside; if((mdx<<1)>yside) { x+=dx; mdx -= yside; }
    }
  }
}

void ImgDraw::vertex
(Img& img, int x0, int y0, int color, float radius) {
  if(radius<0.0f) radius = 0.0f;
  int width  = img.getWidth();
  int height = img.getHeight();
  int   r1     = (int)ceil(radius);
  float r2     = radius*radius;
  int x,y,dx2,dy2;
  for(x=x0-r1;x<=x0+r1;x++) {
    if(x<0 || x>=width) continue;
    dx2 = x-x0; dx2 *= dx2;
    for(y=y0-r1;y<=y0+r1;y++) {
      if(y<0 || y>=height) continue;
      dy2 = y-y0; dy2 *= dy2;
      if((float)(dx2+dy2)>r2) continue;
      img.set(x,y,color);
    }
  }
}

void ImgDraw::points
(Img& img, VecFloat& points, int color, float radius) {
  if(img.getWidth()<=0 || img.getHeight()<=0 || radius<=0.0f) return;
  int nV = points.size()/2;
  if(nV<=0) return;
  float w = (float)img.getWidth();
  float h = (float)img.getHeight();
  int iV,x,y;
  for(iV=0;iV<nV;iV++) {
    x = (int)(0.5f+w*points[2*iV  ]);
    y = (int)(0.5f+h*points[2*iV+1]);
    vertex(img,x,y,color,radius);
  }
}

void ImgDraw::points
(Img& img, VecFloat& points, VecInt& color, float radius) {
  if(img.getWidth()<=0 || img.getHeight()<=0 || radius<=0.0f) return;
  int nV = points.size()/2;
  if(nV<=0 || color.size()<nV) return;
  float w = (float)img.getWidth();
  float h = (float)img.getHeight();
  int iV,x,y;
  for(iV=0;iV<nV;iV++) {
    x = (int)(0.5f+w*points[2*iV  ]);
    y = (int)(0.5f+h*points[2*iV+1]);
    vertex(img,x,y,color[iV],radius);
  }
}

void ImgDraw::mesh
(Img& img, Mesh& mesh, int color, float thickness, float radius) {
  float w = (float)img.getWidth();
  float h = (float)img.getHeight();

  if(thickness>0 && mesh.getNumberOfEdges()>0) { // draw edges
    VecFloat& coord = *mesh.getCoordSrc();
    GraphTraversal gt(*mesh.getEdges());
    gt.start();
    int   iV0,iV1,x0,y0,x1,y1;
    GraphEdge* e;
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      x0  = (int)(0.5f+w*coord[2*iV0  ]);
      y0  = (int)(0.5f+h*coord[2*iV0+1]);
      x1  = (int)(0.5f+w*coord[2*iV1  ]);
      y1  = (int)(0.5f+h*coord[2*iV1+1]);
      edge(img,x0,y0,x1,y1,color,thickness);
    }

  }
  if(radius>0 && mesh.getNumberOfVertices()>0) { // draw vertices
    VecFloat& coord = *mesh.getCoordSrc();
    int nV = coord.size()/2;
    int iV,x,y;
    for(iV=0;iV<nV;iV++) {
      x = (int)(0.5f+w*coord[2*iV  ]);
      y = (int)(0.5f+h*coord[2*iV+1]);
      vertex(img,x,y,color,radius);
    }
  }
}

void ImgDraw::graphPlanar
(Img& img, GraphPlanar& graphPlanar, int color, float thickness, float radius) {
  float w = (float)img.getWidth();
  float h = (float)img.getHeight();
  
  if(thickness>0 && graphPlanar.getNumberOfEdges()>0) { // draw edges
    VecFloat& coord = graphPlanar.getCoord();
    GraphTraversal gt(graphPlanar);
    gt.start();
    int   iV0,iV1,x0,y0,x1,y1;
    GraphEdge* e;
    while((e=gt.next())!=(GraphEdge*)0) {
      iV0 = e->getVertex(0);
      iV1 = e->getVertex(1);
      x0  = (int)(0.5f+w*coord[2*iV0  ]);
      y0  = (int)(0.5f+h*coord[2*iV0+1]);
      x1  = (int)(0.5f+w*coord[2*iV1  ]);
      y1  = (int)(0.5f+h*coord[2*iV1+1]);
      edge(img,x0,y0,x1,y1,color,thickness);
    }

  }
  if(radius>0 && graphPlanar.getNumberOfCoord()>0) { // draw vertices
    VecFloat& coord = graphPlanar.getCoord();
    int nV = coord.size()/2;
    int iV,x,y;
    for(iV=0;iV<nV;iV++) {
      x = (int)(0.5f+w*coord[2*iV  ]);
      y = (int)(0.5f+h*coord[2*iV+1]);
      vertex(img,x,y,color,radius);
    }
  }

}
