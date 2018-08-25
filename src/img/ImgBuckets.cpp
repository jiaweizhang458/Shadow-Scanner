//------------------------------------------------------------------------
// Time-stamp: <2016-03-27 18:36:59 taubin>
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

#include "ImgBuckets.hpp"

ImgBuckets::ImgBuckets():
  _width(0),
  _height(0),
  _first((int*)0),
  _next(),
  _bucket(),
  _outside(-1) {
}

ImgBuckets::ImgBuckets(int width, int height):
  _width(0),
  _height(0),
  _first((int*)0),
  _next(),
  _bucket(),
  _outside(-1) {
  reset(width,height);
}
  
int  ImgBuckets::getHeight() {
  return _height;
}
int  ImgBuckets::getWidth() {
  return _width;
}
int  ImgBuckets::getNumberOfElements() {
  return _next.size();
}

void ImgBuckets::reset(int width, int height) {
  if(width==_width && height==_height)
    erase();
  else if(width>0 && height>0) {
    _width   = width;
    _height  = height;
    _first   = new int[_width*_height];
    int n = _width*_height;
    for(int i=0;i<n;i++) _first[i] = -1;
    _next.clear();
    _bucket.clear();
    _outside = -1;
  }
}

int ImgBuckets::getOutside() {
  return _outside;
}

int ImgBuckets::getFirst(float x, float y, bool normalized) {
  if(normalized) { x *= ((float)_width); y *= ((float)_height); }
  return getFirst((int)x,(int)y);
}

int ImgBuckets::getFirst(int col, int row) {
  return
    (col>=0 && col<_width && row>=0 && row<_height)?_first[col+row*_width]:-1;
}

int ImgBuckets::getNext(int iV) {
  return (0<=iV && iV<_next.size())?_next[iV]:-1;
}

int ImgBuckets::getBucket(int iV) {
  return (0<=iV && iV<_bucket.size())?_bucket[iV]:-1;
}

void ImgBuckets::erase() { // keeps the same _width & _height
  if(_width>0 && _height>0) {
    int n = _width*_height;
    for(int i=0;i<n;i++) _first[i] = -1;
    _next.clear();
    _bucket.clear();
    _outside = -1;
  }
}

void ImgBuckets::remove(int iV) {
  int j,iP,iN;
  if(iV>=0 && iV<_bucket.size() && (j=_bucket[iV])>=0) {
    if((iP=_first[j])==iV) { // iV is the first element of the list
      // pop iV from top of list
      iN = _next[iV];
      _first[j] = iN;
    } else { // iV is not the first element of the list
      // find iP & iN so that iP->iV->iN
      while((iN=_next[iP])!=iV) iP = iN;
      iN = _next[iV];
      // unlink iV
      _next[iP]=iN;
    }
    // reset iV pointers
    _next[iV]=-1;
    _bucket[iV]=-1;
  }
}

void ImgBuckets::set(int col, int row, int iV) {
  if(iV>=0) {
    if(iV>=_next.size()) {
      while(_next.size()<=iV) _next.append(-1);
      while(_bucket.size()<=iV) _bucket.append(-1);
    } else {
      remove(iV);
    }
    if(col<0 || col>=_width || row<0 || row>=_height) {
      // push i onto the _outside list
      _next[iV]=_outside; // _next[i] = _outside;
      // _bucket[iV]=-1;
      _outside = iV;
    } else {
      // push i onto the _first[col][row] list
      int j = col+row*_width;
      _next[iV]=_first[j]; // _next[i] = _first[col][row];
      _bucket[iV]=j;
      _first[j] = iV;
    }
  }
}

void ImgBuckets::init(int nV) {
  erase();
    for(int i=0;i<nV;i++) {
    _next.append(-1);
    _bucket.append(-1);
  }
}
void ImgBuckets::addElement() {
  _next.append(-1);
  _bucket.append(-1);
}

void ImgBuckets::initColRow(VecInt& colRow) {
  erase();
  if(colRow.size()>0) {
    int nV,iV,col,row;
    nV = colRow.size()/2;
    for(iV=0;iV<nV;iV++) {
      col = colRow[2*iV  ];
      row = colRow[2*iV+1];
      set(col,row,iV);
    }
  }
}

void ImgBuckets::initCoord(VecFloat& coord, int D, bool normalized) {
  if(coord.size()>0 && D>=2) {
    int n = _width*_height;
    for(int i=0;i<n;i++) _first[i] = -1;
    _next.clear();
    _bucket.clear();
    _outside = -1;
    int nV,iV,row,col;
    float x,y;
    nV = coord.size()/D;
    for(iV=0;iV<nV;iV++) {
      x = coord[D*iV  ];
      y = coord[D*iV+1];
      if(normalized) {
        x *= ((float)_width);
        y *= ((float)_height);
      }
      col = (int)x;
      row = (int)y;
      set(col,row,iV);
    }
  }
}

void ImgBuckets::initCoord2(VecFloat& coord, bool normalized) {
  initCoord(coord,2,normalized);
}

void ImgBuckets::initCoord3(VecFloat& coord, bool normalized) {
  initCoord(coord,3,normalized);
}


