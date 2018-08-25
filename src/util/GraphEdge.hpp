//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 17:45:39 taubin>
//------------------------------------------------------------------------
//
// GraphEdge.hpp
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

#ifndef _GraphEdge_hpp_
#define _GraphEdge_hpp_

class GraphEdge {

private:

  int   _v0;
  int   _v1;
  int   _indx;
  int   _next;
             
public:

  GraphEdge(int v0, int v1, int indx, int next)
  { _v0=v0; _v1=v1; _indx=indx; _next=next; }

  GraphEdge(int v0, int v1, int indx)
  { _v0=v0; _v1=v1; _indx=indx; _next=-1;}

  GraphEdge(int v0, int v1)
  { _v0=v0; _v1=v1; _indx=-1; _next=-1; }

  GraphEdge(VecInt vI, int j)
  { set(vI,j); }
  
  int getVertex(int i)
  { return (i==0)?_v0:_v1; }
  
  int getIndex()
  { return _indx; }

  void set(VecInt vI, int j) {
    if(j+4<=vI.size()) {
      _v0   = vI[j  ];
      _v1   = vI[j+1];
      _indx = vI[j+2];
      _next = vI[j+3];
    } else {
      _v0   = -1;
      _v1   = -1;
      _indx = -1;
      _next = -1;
    }
  }
  
  void setIndex(int indx)
  { _indx = indx; }
  
  int getNext()
  { return _next; }
  
  void setNext(int next)
  { _next = next; }
  
  int getOtherVertex(int vi)
  { return (_v0==vi)?_v1:(_v1==vi)?_v0:-1; }
  
  bool isVertex(int vi)
  { return (_v0==vi||_v1==vi)?true:false; }

};

#endif // _GraphEdge_hpp_
