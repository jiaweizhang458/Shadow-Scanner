//------------------------------------------------------------------------
// Time-stamp: <2016-03-27 17:59:32 taubin>
// Copyright (c) 2013, Gabriel Taubin, Brown University
// All rights reserved.
//
// PartitionLists.java
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

#include "PartitionLists.hpp"

PartitionLists::PartitionLists(int nElements):
  Partition(),
  _beg(),
  _end(),
  _next() {
  reset(nElements);
}

void PartitionLists::addElement() {
  int i = _father.size();
  Partition::addElement();
  _beg.append(i);
  _end.append(i);
  _next.append(-1);
}

void PartitionLists::reset(int nElements) {
  _beg.clear();
  _end.clear();
  _next.clear();
  if(nElements>0) {
    Partition::reset(nElements);
    for(int i=0;i<_nElements;i++) {
      _beg.append(i); _end.append(i); _next.append(-1);
    }
  }
}

int PartitionLists::join(int i, int j) {
  int Ci = Partition::find(i);
  int Cj = Partition::find(j);
  int Ck = Ci;
  if(Ci!=Cj) {
    if(_size[Ci]>=_size[Cj]) {
      _next[_end[Ci]] = _beg[Cj];
      _end[Ci] = _end[Cj];
      _beg[Cj] = -1;
      _end[Cj] = -1;
    } else {
      _next[_end[Cj]] = _beg[Ci];
      _end[Cj] = _end[Ci];
      _beg[Ci] = -1;
      _end[Ci] = -1;
    }
    Ck = Partition::join(Ci,Cj);
  }
  return Ck;
}

// split a component into singletons
void PartitionLists::split(int i) {
  int j,k;
  i = find(i); // find the root of the component
  _nParts += (_size[i]-1); // fix the number of parts
  // split the tree into singletons
  for(j=_beg[i];j>=0;k=_next[j],_next[j]=-1,j=k) {
    _size[j]=1; _father[j]=j; _beg[j]=j; _end[j]=j;
  }
}

int PartitionLists::beg(int i)  { return  _beg[i]; }
int PartitionLists::end(int i)  { return  _end[i]; }
int PartitionLists::next(int i) { return _next[i]; }

int PartitionLists::length(int i) {
  int len = 0;
  for(int j=_beg[i];j>=0;j=_next[j]) len++;
  return len;
}
