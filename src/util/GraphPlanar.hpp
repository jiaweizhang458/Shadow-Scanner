//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-26 15:50:06 taubin>
//------------------------------------------------------------------------
//
// GraphPlanar.hpp
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

#ifndef _GraphPlanar_hpp_
#define _GraphPlanar_hpp_

#include <iostream>
#include "Graph.hpp"

class Polylines;

class GraphPlanar: public Graph {

public: // static

  static QString HEADER;

private: // static

  static void _log(QString s) {
    std::cerr << "GraphPlanar      | " << s.toUtf8().constData() << std::endl;
  }

protected:

  VecFloat  _coord;
  VecInt    _edgeColor;
  VecInt    _vertexColor;

public:

            GraphPlanar(bool oriented=false);
            GraphPlanar(Polylines& polylines);

  VecFloat& getCoord();
  void      reset(bool oriented);
  void      reset();
  int       getNumberOfCoord();
  void      pushBackCoord(float x);
  int       pushBackCoord(float x, float y);
  void      pushBackCoord(float x, float y, int n);
  int       setCoord(int i, float x, float y);
  void      setEdgeColor(VecInt eColor);
  VecInt&   getEdgeColor();
  void      setVertexColor(VecInt eColor);
  VecInt&   getVertexColor();
  void      lowPass(float lambda, float kappa, int nSmooth);
  void      laplacianSmooth
  (float lambda0, float lambda1, int nSmooth, VecInt* valence = (VecInt*)0);
  void      softConstrainedSmooth
  (float descentStep, float smoothing, int steps);
  void      anchoredSmooth
  (float lambda, int steps);
  void      deleteVertices(VecInt& vDelete);
  void      deleteSharpVertices(float max_sin_angle);

  void      write(QString filename);

};

#endif // _GraphEdge_hpp_
