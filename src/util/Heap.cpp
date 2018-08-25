//------------------------------------------------------------------------
//  Time-stamp: <2016-03-27 17:21:19 taubin>
//------------------------------------------------------------------------
//
// Heap.cpp
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

#include "Heap.hpp"

int Heap::LEFT_SON(int i)  { return 2*(i)+1; }
int Heap::RIGHT_SON(int i) { return 2*(i)+2; }

Heap::Heap():
  _last(0),
  _fKey(),
  _iKey(),
  _perm(),
  _invp() {
}

void Heap::reset() {
  _last = 0;
  _fKey.clear();
  _iKey.clear();
  _perm.clear();
  _invp.clear();
}

void Heap::swap(Heap& H) {
  int last = _last; _last = H._last; H._last = last;
  _fKey.swap(H._fKey);
  _iKey.swap(H._iKey);
  _perm.swap(H._perm);
  _invp.swap(H._invp);
}

// public int length() { return _n(); }
int Heap::length() { return _last; }

void Heap::del(int indx) {
  if(0<=indx && indx<_n())
    _del(_perm[indx]);
}

// indx is the value returned by add(fKey)
int Heap::replaceKey(int indx, float fKey, int iKey) {
  // System.err.println("replaceKey("+indx+","+fKey+","+iKey+")");
  del(indx);
  indx = add(fKey,iKey);
  // System.err.println("  indx="+indx);
  return indx;
}
  
// call getLastKey() after delMin() to get the key value
int Heap::delMin() {
  if(_last<=0) return -1;
  _del(0);
  return _invp[_last];   // returns indx
}

// call getLastKey() after getMin() to get the key value
int   Heap::getMin()      { return     _invp[0]; } // returns indx
float Heap::getLastFKey() { return _fKey[_last]; }
int   Heap::getLastIKey() { return _iKey[_last]; }

// returns indx
int Heap::add(float fKey, int iKey) {
  int last = _last++;
  if(last>=_n()) {
    int n = _n();
    _fKey.append(fKey);
    _iKey.append(iKey);
    _perm.append(n);
    _invp.append(n);
  } else {
    _fKey[last] = fKey;
    _iKey[last] = iKey;
  }
  int indx = _invp[last]; // _perm.get(indx)==last
  _up();
  return indx;
}
  
void Heap::build() {
  for(int i=0;i<_n();i++) {
    _invp[i]=i; _perm[i]=i;
  }
  for(_last=0;++_last<_n();_up());
}
  
void Heap::sort() {
  build();
  while(delMin()>=0);
}

void Heap::_switch(int i, int j) {
  if(i!=j && i<_n() && j<_n()) {
    float key = _fKey[i]; _fKey[i]=_fKey[j]; _fKey[j]=key;
    int   inx = _iKey[i]; _iKey[i]=_iKey[j]; _iKey[j]=inx;
    int   pos = _invp[i]; _invp[i]=_invp[j]; _invp[j]=pos;
    _perm[_invp[i]]=i;
    _perm[_invp[j]]=j;
  }
}
  
void Heap::_down(int father) {
  if(0<=father && father<_last-1) {
    int left,right,child;
    while((left=LEFT_SON(father))<_last) {
      child = father;
      if(_less(left,child))
        child = left;
      if((right=RIGHT_SON(father))<_last && _less(right,child))
        child = right;
      if(child==father)
        break;
      _switch(father,child);
      father = child;
    }
  }
}
  
void Heap::_up() {
  int father;
  int child = _last-1;
  while(child>0 && !_less((father=(child-1)/2),child))
    { _switch(father,child); child = father; }
}

int Heap::_n() { return _perm.size(); }

// lexicographical order
bool Heap::_less(int i, int j) {
  // return (_fKey[i]<_fKey[j])?1:0;
  return
    ((_fKey[i]< _fKey[j]                             ) ||
     (_fKey[i]==_fKey[j] && _invp[i]<_invp[j]));
}
  
void Heap::_del(int i) {
  if(0<=i && i<_last) {
    _last--; _switch(i,_last); _down(i);
  }
}

