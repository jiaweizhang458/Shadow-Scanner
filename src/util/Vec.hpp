//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-27 15:09:02 taubin>
//------------------------------------------------------------------------
//
// Vec.hpp
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

#ifndef _Vec_hpp_
#define _Vec_hpp_

#include <QVector>

typedef QVector<bool>   VecBool;
typedef QVector<int>    VecInt;
typedef QVector<float>  VecFloat;
typedef QVector<double> VecDouble;
typedef unsigned char   byte;            
typedef QVector<byte>   VecByte;

class VecBit {

private: // static

  static int MIN_ARRAY_LENGTH;
  static byte _mask0[];
  static byte _mask1[];

public: // static

  static void  getBits(byte b, bool* bits);
  static bool* getBits(byte b);

private:
  
  int     _nBits;
  VecByte _bytes;
  
public:

       VecBit();
       VecBit(int nBitsReserve);
      ~VecBit();
  
  int  getNumberOfBits();
  int  capacity();
  void clear();  
  byte get8(int k);
  void set8(int k, byte value);
  bool get(int j);
  void set(int j, bool value);  
  bool getBack();
  bool getFront();
  void popBack();
  void popBack(int n);
  bool getPopBack();
  void pushBack(VecBool* vecValue);
  void pushBack(bool value);
  void pushBack(byte b);
  void pushBackN(int n, bool value);  
  void reserve(int nBits); 
  int  size();
  void swap(VecBit& otherVecBit);
};

#endif //_Vec_hpp_
