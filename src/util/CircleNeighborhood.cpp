//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-28 21:02:40 taubin>
//------------------------------------------------------------------------
//
// CircleNeighborhood.cpp
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
#include "CircleNeighborhood.hpp"
#include "Heap.hpp"

VecInt CircleNeighborhood::_values;

void CircleNeighborhood::initialize(int R) {

  if(R<=0) R = 0;

  VecInt  tmp;
  Heap    H;
  int     iKey,i,j,k,r2,R2=R*R;
  float   fKey;

  for(i=-R;i<=R;i++) {
    for(j=-R;j<=R;j++) {
      if((r2=i*i+j*j)<=R2) {
        fKey = (float)sqrt((float)r2);
        iKey = tmp.size()/2;
          tmp.append(i);
          tmp.append(j);
        H.add(fKey,iKey);
      }
    }
  }

  while(H.length()>0) {
    k    = H.delMin();
    fKey = H.getLastFKey();
    iKey = H.getLastIKey();
    i = tmp[2*iKey  ];
    j = tmp[2*iKey+1];
      _values.append(i);
      _values.append(j);
  }
}

int CircleNeighborhood::length() {
  return _values.size()/2;
}
  
VecInt& CircleNeighborhood::getValues() {
  return _values;
}
  
int CircleNeighborhood::getI(int n) {
  return (n<0 || n>=_values.size()/2)?-1:_values[2*n  ];
}
  
int CircleNeighborhood::getJ(int n) {
  return (n<0 || n>=_values.size()/2)?-1:_values[2*n+1];
}

