//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-05-03 12:02:08 taubin>
//------------------------------------------------------------------------
//
// LineDetector6.hpp
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

#ifndef _LineDetector6_hpp_
#define _LineDetector6_hpp_

#include <iostream>
#include <util/Vec.hpp>
#include <img/Img.hpp>
#include "Polylines.hpp"

class LineDetector6 {

public: // static

private: // static

  static void _log(QString s) {
    // std::cerr << "LineDetector6 | " << s.toUtf8().constData() << std::endl;
  }

protected:

  Img&         _maskedChecherboardImg;
  Polylines&   _polylines;
  VecInt       _histogram;
  int          _checkerboardRows;
  int          _checkerboardCols;
  int          _rowMin;
  int          _rowMax;

public:

  LineDetector6(Img& maskedChecherboardImg,
                Polylines& polylines,
                VecFloat&  checkerboardCorners,
                VecInt&    checkerboardCornersColor,
                bool extendToBorder = false,
                int checkerboardRows=1, int checkerboardCols=1,
                int rowMin=0, int rowMax=0);

private:

  int testLineFit
  (const int iVL0, const int iVL1,
   const float nx, const float ny, const float nd,
   float& max_dist, float& mean_dist);

  int testLineFit
  (const VecInt& vertex,
   const float nx, const float ny, const float nd,
   float& max_dist, float& mean_dist);
  
  int fitLineFixedEnds
  (const int iVL0, const int iVL1,
   float& nx, float& ny, float& nd,
   float& max_dist, float& mean_dist);
  
  int fitLineLeastSquares
  (const int iVL0, const int iVL1,
   float& nx, float& ny, float& nd, float& max_dist, float& mean_dist);

  int fitLineLeastSquares
  (const VecInt& vertex,
   float& nx, float& ny, float& nd, float& max_dist, float& mean_dist);

  void extendSegmentToBorder(float& xx0, float& yy0, float& xx1, float& yy1);

};

#endif // _LineDetector6_hpp_
