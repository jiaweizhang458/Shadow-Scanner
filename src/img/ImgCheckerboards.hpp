// Software developed for the Spring 2016 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
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
// DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO NOR GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _ImgCheckerboards_hpp_
#define _ImgCheckerboards_hpp_

#include <iostream>
#include <QString>

#include <math.h>
// #include "Image.hpp"
// #include "ImageBuffer.hpp"
#include "ImgArgb.hpp"
#include "ImgFloat.hpp"
#include "ImgBuckets.hpp"
#include <util/GraphPlanar.hpp>
#include <util/Polylines.hpp>
#include <util/Mesh.hpp>
#include <util/PartitionLists.hpp>

class ImgCheckerboards {

private: // static

  static void _log(QString s) {
    // std::cerr << "ImgCheckerboards | " << s.toUtf8().constData()
    //           << "\n";
  }
  static void _log(QString s, int v) {
    // std::cerr << "ImgCheckerboards | " << s.toUtf8().constData()
    //           << v <<"\n";
  }

public:

  static ImgArgb* boxFilter
  (Img&            srcImg,
   int             radius);

  static int boxFilterThreshold
  (Img&            srcImg,
   int             nMorphology,
   PartitionLists& P);

  static ImgArgb* makeSegmentedImg
  (PartitionLists& P,
   int             iBlack,
   Img&            img);

  static ImgArgb* segment1
  (Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow);

  static void segment1
  (Img&            srcImg,
   float           maxEdgeWeight,
   int             nSegments,
   PartitionLists& P);

  static ImgArgb* segment2
  (Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow);

  static void segment2
  (Img&            srcImg,
   float           maxEdgeWeight,
   int             nSegments,
   PartitionLists& P);

  static ImgArgb* segment3
  (Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow);

  static void segment3
  (Img&            srcImg,
   float           maxEdgeWeight,
   int             nSegments,
   PartitionLists& P);

  static ImgArgb* segment6
  (Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow);

  static ImgArgb* segment11
  (Img& srcImg, float maxEdgeWeight, int nMinSegments, int fromRow, int toRow,
   GraphPlanar& edges);

  static ImgArgb* segment12
  (Img& srcImg, float maxEdgeWeight, int nMinSegments,
   int fromRow, int toRow, int cbRows, int cbCols, Polylines& polylines);

  static Img* detect
  (Img&            srcImg,
   GraphPlanar&    graphP,
   GraphPlanar&    graphD,
   Mesh&           mesh,
   int             background,
   bool            paintVertexDiscs,
   bool            paintBoundaryDiscs,
   bool            drawBorderTriangles);

  static ImgFloat* xFloatEdgesI(Img& srcImg);
  static ImgFloat* yFloatEdgesI(Img& srcImg);
  static ImgFloat* sobelFloatEdgesI(Img& srcImg);
  static ImgFloat* sobelFloatEdgesIx(Img& srcImg);
  static ImgFloat* sobelFloatEdgesIy(Img& srcImg);

private:

  static int    RED;
  static int    WHITE;
  static int    BLACK;
  static int    BLUE;

  static float _wMinSobel;
  static float _wMaxSobel;

  static float _randomColorBase; // 56.0f;
  static float _randomColorScale;

  static float _minScale;
  static float _maxScale;

  static float _random(); // [0:1] range

  static int   _makeRandomColor();

  static float _max(float a, float b) {
    return (a>b)?(0.65f*b+0.35f*a):(0.35f*a+0.65f*b);
  }
  static float _min(float a, float b) {
    return (a<b)?(0.65f*b+0.35f*a):(0.35f*a+0.65f*b);
  }
  static float _dist(float x0, float y0, float x1, float y1) {
    return (float)sqrt((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
  }

  static void _regularQuadMeshFilter
  (VecFloat& coord, VecInt& coordIndex);

  static void _createQuads
  (Img& srcImg, int nMorphology, PartitionLists& P,
   VecFloat& coord, VecInt& coordIndex);

  static void _clusterBoundaryEdgeCenter
  (ImgBuckets& buckets, VecFloat& coord,
   VecInt& coordIndex, Img& segmented,
   bool paintVertexDiscs);

  static void _clusterBoundaryVertices
  (ImgBuckets& buckets, VecFloat& coord,
   VecInt& coordIndex, Img& segmented,
   bool paintVertexDiscs);

  static void _drawBorderTriangles
  (Mesh& mesh, GraphPlanar& graphD);

  static void _clusterVertices
  (VecFloat& coord, VecInt& coordIndex, PartitionLists& P);

  static void _dualMeshFilter
  (VecFloat& coord, VecInt& coordIndex);

  static ImgFloat* _sobelStrength(Img& srcImg);

  static void _computeFaceCenters
  (float fWidth, float fHeight,
   VecFloat& coord, VecInt& coordIndex, VecFloat& coordFace);

  static float _quadness19
  (int N, PartitionLists& P, int i, ImgFloat& strength, float coord[/*8*/]);

  static void _removeUnusedVertices
  (VecFloat& coord, VecInt& coordIndex);

  static void _setMinScale(float value);
  static void _setMaxScale(float value);
  static float _scale(float value);

  static ImgFloat* _coreFloatEdgesI(Img& srcImg, int mode);

  static bool _areSimilarSizes(int size0, int size1, float factor);

};

#endif //_ImgCheckerboards_hpp_
