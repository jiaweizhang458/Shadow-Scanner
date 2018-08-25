//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-28 08:47:24 taubin>
//------------------------------------------------------------------------
//
// Polylines.hpp
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

#ifndef _Polylines_hpp_
#define _Polylines_hpp_

#include <iostream>
#include <QString>
#include "Vec.hpp"

class Polylines {

private: // static

  static void _log(QString s) {
    std::cerr << "Polylines      | " << s.toUtf8().constData() << std::endl;
  }

protected:

  VecFloat  _coord;
  VecInt    _first;
  VecInt    _next;

public:

            Polylines();

  void      reset();
  void      swap(Polylines& polylines);

  VecFloat& getCoord();
  VecInt&   getFirst();
  VecInt&   getNext();

  bool      resetFirst();

  int       getNumberOfVertices();
  int       getNumberOfPolylines();
  int       getNumberOfEdges();
  int       pushBackCoord(float x, float y, int nxt=-1);
  int       pushBackFirst(int iV);
  int       setNext(int iV0, int iV1);

  int       first(int iP);
  int       next(int iV);

  void      convertToDual();

};

#endif // _Polylines_hpp_
