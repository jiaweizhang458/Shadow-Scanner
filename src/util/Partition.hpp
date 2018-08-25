//------------------------------------------------------------------------
// Time-stamp: <2016-04-28 11:28:18 taubin>
// Copyright (c) 2013, Gabriel Taubin, Brown University
// All rights reserved.
//
// Partition.hpp
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

#ifndef _Partition_hpp_
#define _Partition_hpp_

#include "Vec.hpp"

class Partition {

protected:

  int    _nElements;
  int       _nParts;
  VecInt    _father;
  VecInt      _size;
  int*   _partFirst;
  int*   _partIndex;

public:

        Partition(int nElements=0);

  void  addElement();
  void  reset(int nElements);
  int   size(const int i);
  int   getNumberOfParts();
  int   getNumberOfElements();
  int   find(int i);
  int   join(const int i, const int j);
  void  _end();
  void  makeParts();
  int   getNumberOfElementsInPart(const int i);
  int*  getPart(const int i);
  void  getPart(const int i, VecInt& part);

};

#endif // _Partition_hpp_
