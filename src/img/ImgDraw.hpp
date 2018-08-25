//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-04-30 01:56:35 taubin>
//------------------------------------------------------------------------
//
// ImgDraw.hpp
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

#ifndef _ImgDraw_hpp_
#define _ImgDraw_hpp_

#include "Img.hpp"

class Mesh;
class GraphPlanar;

class ImgDraw {

private:

  Img&         _img;
  int          _bgColor;
  int          _fgColor;
  int          _edgeColor;
  float        _edgeThickness;
  int          _vertexColor;
  float        _vertexRadius;

public:

  ImgDraw(Img& img);

  void setBgColor(int value);
  void setFgColor(int value);
  void setEdgeColor(int value);
  void setEdgeThickness(float value);
  void setVertexColor(int value);
  void setVertexRadius(float value);
  void edge(int x0, int y0, int x1, int y1);
  void vertex(int x0, int y0);

  static void edge
  (Img& img, int x0, int y0, int x1, int y1, int color, float thickness);
  static void vertex
  (Img& img, int x0, int y0, int color, float radius);
  static void points
  (Img& img, VecFloat& points, int color, float radius);
  static void points
  (Img& img, VecFloat& points, VecInt& color, float radius);
  static void mesh
  (Img& img, Mesh& mesh, int color, float thickness, float radius);
  static void graphPlanar
  (Img& img, GraphPlanar& graphPlanar, int color, float thickness, float radius);

};

#endif // _ImgDraw_hpp_
