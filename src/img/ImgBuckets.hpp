//------------------------------------------------------------------------
// Time-stamp: <2016-03-27 18:46:40 taubin>
// Copyright (c) 2013-2016, Gabriel Taubin, Brown University
// All rights reserved.
//
// ImgBuckets.hpp
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
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GABRIEL
// TAUBIN OR BROWN UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _ImgBuckets_hpp_
#define _ImgBuckets_hpp_

#include <util/Vec.hpp>

class ImgBuckets {

private:

  int    _width;
  int    _height;
  int*  _first;
  VecInt _next;
  VecInt _bucket;
  int    _outside;

public:

  ImgBuckets();
  ImgBuckets(int width, int height);
  
  int  getHeight();
  int  getWidth();
  int  getNumberOfElements();

  void reset(int width, int height);
  int  getOutside();
  int  getFirst(float x, float y, bool normalized);
  int  getFirst(int col, int row);
  int  getNext(int iV);
  int  getBucket(int iV);
  void erase();
  void remove(int iV);
  void set(int col, int row, int iV);
  void init(int nV);
  void addElement();
  void initColRow(VecInt& colRow);
  void initCoord(VecFloat& coord, int D, bool normalized);
  void initCoord2(VecFloat& coord, bool normalized);
  void initCoord3(VecFloat& coord, bool normalized);

};

#endif //_ImgBuckets_hpp_


