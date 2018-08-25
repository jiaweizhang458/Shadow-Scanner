//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-14 11:07:22 taubin>
//------------------------------------------------------------------------
//
// WrlViewerData.hpp
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

#ifndef _WRL_VIEWER_DATA_HPP_
#define _WRL_VIEWER_DATA_HPP_

#include "wrl/SceneGraph.h"

class WrlViewerData {
public:
  WrlViewerData();
  ~WrlViewerData();

  void           setSceneGraph(SceneGraph* pWrl);

  SceneGraph*    getSceneGraph()
  { return _pWrl; }
  vector<float>& getFunctionVertices()
  { return _functionVertices; }
  vector<float>& getFunctionCells()
  { return _functionCells; }
  int            getBBoxDepth()
  { return _bboxDepth; }
  void           setBBoxDepth(int d)
  { _bboxDepth = (d<0)?0:(d>10)?10:d; }
  bool           getBBoxCube()
  { return _bboxCube; }
  void           setBBoxCube(bool value)
  { _bboxCube = value; }
  bool           getBBoxOccupied()
  { return _bboxOccupied; }
  void           setBBoxOccupied(bool value)
  { _bboxOccupied = value; }
  float          getBBoxScale()
  { return   _bboxScale; }
  void           setBBoxScale(float scale)
  { _bboxScale = (scale<0.0f)?0.0f:scale; }

private:

  SceneGraph*   _pWrl;
  vector<float> _functionVertices;
  vector<float> _functionCells;
  int           _bboxDepth;
  bool          _bboxCube;
  bool          _bboxOccupied;
  float         _bboxScale;

};

#endif /* _WRL_VIEWER_DATA_H_ */
