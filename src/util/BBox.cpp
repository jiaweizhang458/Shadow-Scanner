//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 09:49:39 taubin>
//------------------------------------------------------------------------
//
// BBox.cpp
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

#include <math.h>
#include "BBox.h"

BBox::~BBox() {
  if(_min   !=(float*)0) delete [] _min;
  if(_max   !=(float*)0) delete [] _max;
 }

BBox::BBox(const int d, const vector<float>& v, const bool cube):
  _d(0),_min((float*)0),_max((float*)0),_step(0.0f) {
  if(d>0) {
    _d     = d;
    _min    = new float[d];
    _max    = new float[d];
    float* center = new float[d];
    int i,j;
    int nV = (int)(v.size()/d);
    if(nV>0) {
      for(i=0;i<d;i++) {
        float vji = v[i];
        _min[i]    = vji;
        _max[i]    = vji;
      }
      for(j=1;j<nV;j++)
        for(i=0;i<d;i++) {
          float vji = v[d*j+i];
          if(vji<_min[i]) _min[i] = vji;
          if(vji>_max[i]) _max[i] = vji;
        }
      for(i=0;i<d;i++)
        center[i] = (_min[i]+_max[i])/2;
    }
    
    if(cube) {
      float halfSide = 0.0f;
      for(i=0;i<d;i++) {
        float hsi = (_max[i]-_min[i])/2.0f;
        if(hsi>halfSide) halfSide = hsi;
      }
      for(i=0;i<d;i++) {
        _min[i] = center[i] - halfSide;
        _max[i] = center[i] + halfSide;
      }
    }
    
    // make sure the box has nonzero volume
    float minSide = 0.0f;
    for(i=0;i<d;i++) {
      float sideI = (_max[i]-_min[i])/2;
      if(sideI>0.0f && (minSide==0.0f || sideI<minSide))
        minSide = sideI;
    }
    if(minSide==0.0f) minSide = 1.0f;
    float minHalfSide = minSide*0.05f;
    for(i=0;i<d;i++)
      if(_max[i]==_min[i]) {
        _min[i] -= minHalfSide;
        _max[i] += minHalfSide;
      }
    
    _step = getSide();
	delete [] center;
  }
}

BBox::BBox(const int d):
  _d(0),_min((float*)0),_max((float*)0),_step(0.0f) {
  if(d>0) {
    _d      = d;
    _min    = new float[d];
    _max    = new float[d];
    
    int i;
    
    for(i=0;i<d;i++) {
      _min[i]    = 0.0f;
      _max[i]    = 1.0f;
    }
    
    // make sure the box has nonzero volume
    float minSide = 0.0f;
    for(i=0;i<d;i++) {
      float sideI = (_max[i]-_min[i])/2;
      if(sideI>0.0f && (minSide==0.0f || sideI<minSide))
        minSide = sideI;
    }
    if(minSide==0.0f) minSide = 1.0f;
    float minHalfSide = minSide*0.05f;
    for(i=0;i<d;i++)
      if(_max[i]==_min[i]) {
        _min[i] -= minHalfSide;
        _max[i] += minHalfSide;
      }
    
    _step = getSide();
  }
}

int BBox::getDimension() const {
  return _d;
}
  
float* BBox::getMax() const {
  return _max;
}  

float BBox::getMax(const int i) const {
  return _max[i];
}

float* BBox::getMin() const {
  return _min;
}

float BBox::getMin(const int i) const {
  return _min[i];
}  

float BBox::getCenter(const int i) const {
  int d = getDimension();
  int j =(i<0 )?0:(i>=d)?d-1:i;
  return (_max[j]+_min[j])/2.0f;
}
  
float BBox::getSide() const {
  float side = 0.0f;
  for(int i=0;i<_d;i++) {
    float side_i = _max[i]-_min[i];
    if(side_i>side) side = side_i;
  }
  return side;
}

float BBox::getSide(const int i) const {
  int j = (i<0)?0:(i>=_d)?_d-1:i;
  return _max[j]-_min[j];
}

float BBox::getMaxSide() const {
  float maxSide = 0.0f;
  int d = getDimension();
  if(d>0) {
    float side_i;
    maxSide = getSide(0);
    for(int i=1;i<d;i++) {
      if((side_i=getSide(i))>maxSide)
        maxSide = side_i;
    }
  }
  return maxSide;
}

void BBox::setMax(const float* value /*[3]*/) {
  for(int i=0;i<3;i++)
    _max[i] = value[i];
}  

void BBox::setMin(const float* value /*[3]*/) {
  for(int i=0;i<3;i++)
    _min[i] = value[i];
}

float BBox::getDiameter() const {
  float diam2 = 0.0f;
  int   d = getDimension();
  if(d>0) {
    float side_i;
    for(int i=0;i<d;i++) {
      side_i = getSide(i);
      diam2 += side_i*side_i;
    }
  }
  return (diam2>0.0f)?(float)sqrt(diam2):0.0f;
}
