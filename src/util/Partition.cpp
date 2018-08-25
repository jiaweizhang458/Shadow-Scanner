//------------------------------------------------------------------------
// Time-stamp: <2016-04-28 11:28:25 taubin>
// Copyright (c) 2013, Gabriel Taubin, Brown University
// All rights reserved.
//
// Partition.cpp
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

#include "Partition.hpp"

Partition::Partition(int nElements):
  _nElements(0),
  _nParts(0),
  _father(),
  _size(),
  _partFirst((int*)0),
  _partIndex((int*)0) {
  if(nElements>0) reset(nElements);
}

void Partition::addElement() {
  int i = _father.size();
  _father.append(i);
  _size.append(1);
  _nElements++;
  _nParts++;
}

void Partition::reset(int nElements) {
  _nElements = 0;
  _nParts    = 0;
  _father.clear();
  _size.clear();
  if(nElements>0) {
    _nElements = nElements;
    _nParts    = _nElements;
    for(int i=0;i<_nElements;i++) {
      _size.append(1);
      _father.append(i);
    }
  }
  // what about _partFirst & _partIndex ???

  if(_partFirst!=(int*)0) {
    delete [] _partFirst;
    _partFirst = (int*)0;
  }
  if(_partIndex!=(int*)0) {
    delete [] _partIndex;
    _partIndex = (int*)0;
  }
}
  
// assumes i>=0 && i<_nElements
int Partition::size(const int i)     { return   _size[i]; }
int Partition::getNumberOfParts()    { return    _nParts; }
int Partition::getNumberOfElements() { return _nElements; }

// assumes 0<=i && i<_nElements
int Partition::find(int i) {
  int Ri,Fi;
  // traverse path
  // for(Ri=i;_father[Ri]!=Ri;Ri=_father[Ri]);
  for(Ri=i;_father[Ri]!=Ri;Ri=_father[Ri]);
  // compress path
  // while(i!=Ri) { Fi = _father[i]; _father[i] = Ri; i = Fi; }
  while(i!=Ri) { Fi = _father[i]; _father[i]=Ri; i = Fi; }
  return i;
}

// assumes 0<=i && i<_nElements && 0<=j && j<_nElements
int Partition::join(const int i, const int j) {
  int Ci  = find(i);
  int Cj  = find(j);
  int Ck = Ci;
  if(Ci!=Cj) {
    _nParts--;
    // if(_size[Ci]>=_size[Cj]) { // make Ci the root
    //   Ck = _father[Cj] = Ci; _size[Ci] += _size[Cj]; _size[Cj] = 0;
    // } else { // make Cj the root
    //   Ck = _father[Ci] = Cj; _size[Cj] += _size[Ci]; _size[Ci] = 0;
    // }
    if(_size[Ci]>=_size[Cj]) { // make Ci the root
      _father[Cj]=Ci; Ck = Ci; _size[Ci]+=_size[Cj]; _size[Cj]=0;
    } else { // make Cj the root
      _father[Ci]=Cj; Ck = Ci; _size[Cj]+=_size[Ci]; _size[Ci]=0;
    }
  }
  return Ck;
}

void Partition::_end() {
  int* min = new int[_nElements];
  int i;
  for(i=0;i<_nElements;i++) {
    min[i] = -1; min[find(i)] = find(i);
  }
  for(i=0;i<_nElements;i++)
    if(i<min[find(i)]) min[find(i)] = i;
  for(i=0;i<_nElements;i++)
    if(_size[i]>0 && min[i]<find(i)) {
      _father[i]    =  min[i];
      _father[min[i]] = min[i];
      _size[min[i]] = _size[i];
      _size[i]        = 0;
    }
}

void Partition::makeParts() {
  _end();
  int nP = getNumberOfParts();

  if(_partFirst!=(int*)0) delete [] _partFirst;
  _partFirst = new int[nP+1];
  if(_partIndex!=(int*)0) delete [] _partIndex;
  _partIndex = new int[_nElements];

  int i,j;

  int* part = new int[_nElements];
  for(i=0;i<_nElements;i++)
    part[i] = -1;
  for(j=i=0;i<_nElements;i++)
    if(_size[i]>0)
      part[i] = j++;
    
  for(i=0;i<=nP;i++)
    _partFirst[i] = 0;
  for(i=0;i<_nElements;i++)
    _partFirst[part[find(i)]+1]++;
  for(i=0;i<nP;i++)
    _partFirst[i+1] += _partFirst[i];
    
  int* indx = new int[nP];
  for(i=0;i<nP;i++)
    indx[i] = 0;
    
  for(i=0;i<_nElements;i++) {
    int ip = part[find(i)];
    _partIndex[_partFirst[ip]+indx[ip]] = i;
    indx[ip]++;
  }
}

int Partition::getNumberOfElementsInPart(const int i) {
  int n = 0;
  if(i>=0 && i<getNumberOfParts()) {
    if(_partFirst==(int*)0 || _partIndex==(int*)0) makeParts();
    n = _partFirst[i+1]-_partFirst[i];
  }
  return n;
}
  
int* Partition::getPart(const int i) {
  int* part = (int*)0;
  if(i>=0 && i<getNumberOfParts()) {
    if(_partFirst==(int*)0 || _partIndex==(int*)0) makeParts();
    int n    = _partFirst[i+1]-_partFirst[i];
    part = new int[n];
    for(int j=0;j<n;j++)
      part[j] = _partIndex[_partFirst[i]+j];
  }
  return part;
}
  
void Partition::getPart(const int i, VecInt& part) {
  part.clear();
  if(i>=0 && i<getNumberOfParts()) {
    if(_partFirst==(int*)0 || _partIndex==(int*)0) makeParts();
    int n    = _partFirst[i+1]-_partFirst[i];
    part.reserve(n);
    for(int j=0;j<n;j++)
      part.append(_partIndex[_partFirst[i]+j]);
  }
}

