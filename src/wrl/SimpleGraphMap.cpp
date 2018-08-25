//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-14 10:56:47 taubin>
//------------------------------------------------------------------------
//
// SimpleGraphMap.cpp
//
// Software developed for the Fall 2012 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2012, Gabriel Taubin
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

// #include <iostream>
#include "SimpleGraphMap.h"

SimpleGraphMap::SimpleGraphMap() {
}
SimpleGraphMap::~SimpleGraphMap() {
}

void SimpleGraphMap::clear() {
  _first.clear();
  _data.clear();
}

int SimpleGraphMap:: insert(int jV, int kV, int value) {
  int retValue = -1;
  if(jV>=0 && kV>=0 && jV!=kV) {
    if((retValue=get(jV,kV))<0) {
      int i,hV;
      // make sure that jV<kV
      if(jV>kV) { hV=jV; jV=kV; kV=hV; }
      while(jV>=(int)_first.size())
        _first.push_back(-1);
      i = (int)(_data.size());
      _data.push_back(kV); // vertex index
      _data.push_back(_first[jV]); // next
      _data.push_back(value);
      _first[jV] = i;
      retValue = value;
    }
  }
  // cerr << "insert("<< jV <<","<< kV <<","<< value <<") = "<< retValue << endl;
  return retValue;
}

int SimpleGraphMap::get(int jV, int kV) const {
  int value = -1;
  if(jV>=0 && kV>=0 && jV!=kV) {
    int hV,i;
    if(jV>kV) { hV=jV; jV=kV; kV=hV; }    
    if(jV<(int)_first.size()) {
      for(i=_first[jV];i>=0 && (hV=_data[i])>=0;i=_data[i+1]) {
        if(hV==kV) { value = _data[i+2]; break; }
      }
    }
  }
  // cerr << "get("<< jV <<","<< kV <<") = "<< value << endl;
  return value;
}
