//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 15:09:41 taubin>
//------------------------------------------------------------------------
//
// Vec.cpp
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

#include "Vec.hpp"

int VecBit::MIN_ARRAY_LENGTH = 16;

byte VecBit::_mask0[] = {
  (byte)0x01, (byte)0x02, (byte)0x04,  (byte)0x08,
  (byte)0x10, (byte)0x20, (byte)0x40,  (byte)0x80
};

byte VecBit::_mask1[] = {
  (byte)0xfe, (byte)0xfd, (byte)0xfb,  (byte)0xf7,
  (byte)0xef, (byte)0xdf, (byte)0xbf,  (byte)0x7f 
};

void VecBit::getBits(byte b, bool* bits) {
  for(int i=0;i<8;i++)
    bits[i] = ((b&_mask0[i])!=0);
}

bool* VecBit::getBits(byte b) {
  bool* bits = new bool[8];
  getBits(b,bits);
  return bits;
}

VecBit::VecBit():
  _nBits(0),
  _bytes() {
  reserve(8*MIN_ARRAY_LENGTH);
}  

VecBit::~VecBit() {
}

VecBit::VecBit(int nBitsReserve) {
  if(nBitsReserve<8*MIN_ARRAY_LENGTH) nBitsReserve = 8*MIN_ARRAY_LENGTH;
  reserve(nBitsReserve);
}
  
int VecBit::getNumberOfBits()
{ return _nBits; }

int VecBit::capacity()
{ return _bytes.size()*8; }
  
void VecBit::clear() {
  int nBytes = _bytes.size();
  for(int i=0;i<nBytes;i++)
    _bytes[i] = 0x00;
  _nBits = 0;
}  

byte VecBit::get8(int k)
{ return _bytes[k]; }

void VecBit::set8(int k, byte value)
{ _bytes[k] = value; }

bool VecBit::get(int j)
{ return ((_bytes[(j/8)]&_mask0[(j%8)])!=0); }  

void VecBit::set(int j, bool value) {
  int k=j/8, l=j%8;
  _bytes[k]&=_mask1[l];
  if(value) _bytes[k] |= _mask0[l];
}  

bool VecBit::getBack()
{ return get(_nBits-1); }  

bool VecBit::getFront()
{ return get(0); }  

void VecBit::popBack()
{ if(_nBits>0) _nBits--; }  

void VecBit::popBack(int n)
{ while(n-->0) popBack(); }

bool VecBit::getPopBack() {
  bool v = getBack(); popBack(); return v;
}

void VecBit::pushBack(VecBool* vecValue) {
  if(vecValue!=(VecBool*)0)
    for(int i=0;i<vecValue->size();i++)
      pushBack((*vecValue)[i]);
}

void VecBit::pushBack(bool value) {
  if(_nBits>=_bytes.size()*8) reserve(_nBits+1);
  set(_nBits,value);
  _nBits++;
}

void VecBit::pushBack(byte b) {
  for(int i=0;i<8;i++)
    pushBack((b&_mask0[i])!=0);
}

void VecBit::pushBackN(int n, bool value) {
  if(n>0) {
    reserve(_nBits+n);
    for(int i=0;i<n;i++)
      pushBack(value);
  }
}  

void VecBit::reserve(int nBits) {
  if(nBits>capacity()) {
    int nBytes = (nBits+7)/8;
    if(nBytes<MIN_ARRAY_LENGTH) nBytes = MIN_ARRAY_LENGTH;
    int nBytesNew = MIN_ARRAY_LENGTH;
    while(nBytesNew<nBytes) nBytesNew *= 2;
    while(_bytes.size()<nBytesNew)
      _bytes.append(-1);
  }
} 

int VecBit::size()
{ return _nBits; }  

void VecBit::swap(VecBit& otherVecBit) {
  int    nBits = _nBits;
  _nBits = otherVecBit._nBits;
  otherVecBit._nBits = nBits;
  _bytes.swap(otherVecBit._bytes);
}
