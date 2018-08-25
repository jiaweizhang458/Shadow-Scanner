//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 09:50:51 taubin>
//------------------------------------------------------------------------
//
// StaticRotation.h
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

#ifndef _ROTATION_H_
#define _ROTATION_H_

class StaticRotation {

public:

  static void rotate
  (float* r /*[4]*/, float* x /*[3]*/, float* y /*[3]*/);

  static void vectorToMatrix
  (float ang_deg, float u0, float u1, float u2, float* R /*[16]*/);

  static void vectorToMatrix
  (float* r /*[4]*/, float* R /*[16]*/);

  static void matrixToVector
  (float* R /*[16]*/, float* r /*[4]*/);

  static void multiplyMatrices
  (float* A /*[16]*/, float* B /*[16]*/, float* C /* C[16]=A*B */);

  static void multiplyMatricesLeft
  (float* A /*[16]*/, float* B /*[16]*/);

  static void vectorMultiplyLeft
  (float ang_deg, float u0, float u1, float u2, float* r /*[4]*/);

  static void crossProduct
  (double* x /*[3]*/, double* y /*[3]*/, double* z /*[3]*/);

};

#endif /* _ROTATION_H_ */
