//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 09:50:41 taubin>
//------------------------------------------------------------------------
//
// StaticRotation.cpp
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

// #ifdef _WINDOWS
// #define _USE_MATH_DEFINES
// #endif
#include <math.h>
#ifndef M_PI
#define M_PI 3.141592654
#endif
#include "StaticRotation.h"

  //////////////////////////////////////////////////////////////////////////////////
  //
  // computes
  // y = rotation of angle=r[0] (in degrees)
  // of vector x around vector u=(r[1],r[2],r[3])
  //

void StaticRotation::rotate(float* r /*[4]*/, float* x /*[3]*/, float* y /*[3]*/) {
  // vectorToMatrix(r,R);
  // y = R*x;
  float ang,nu,u0,u1,u2,c,s,ux,v0,v1,v2,w0,w1,w2;
  // rotation angle in radians
  ang = (float)M_PI*r[0]/180.0f;
  // normalize axis of rotation vector to unit length
  u0 = r[1]; u1 = r[2]; u2 = r[3];
  nu = u0*u0+u1*u1+u2*u2;
  if(nu>0.0f) {
    nu = sqrt(u0*u0+u1*u1+u2*u2);
    u0 /= nu; u1 /= nu; u2 /= nu;
    // sin and cos of the angle of rotation
    c = cos(ang); s = sin(ang);
    // inner product of u and x
    ux = u0*x[0]+u1*x[1]+u2*x[2];
    // orthogonal projection of x
    v0 = x[0]-ux*u0; v1 = x[1]-ux*u1; v2 = x[2]-ux*u2;
    // third orthogonal vector
    w0 = u1*v2-u2*v1; w1 = u2*v0-u0*v2; w2 = u0*v1-u1*v0;
    // compose the rotated vector
    y[0] = ux*u0+c*v0+s*w0;
    y[1] = ux*u1+c*v1+s*w1;
    y[2] = ux*u2+c*v2+s*w2;
  } else /* if(n<=0.0f) */ {
    // u==0 => rotation==identity
    y[0] = x[0]; y[1] = x[1]; y[2] = x[2];
  }
}

void StaticRotation::vectorToMatrix
(float ang_deg, float u0, float u1, float u2, float* R /*[16]*/) {
  float ang,nu,c,s,d;
  // rotation angle in radians
  ang = (float)M_PI*ang_deg/180.0f;
  // normalize axis of rotation vector to unit length
  nu = u0*u0+u1*u1+u2*u2;
  if(nu>0.0f) {
    nu = sqrt(u0*u0+u1*u1+u2*u2);
    u0 /= nu; u1 /= nu; u2 /= nu;
    // sin and cos of the angle of rotation
    c = cos(ang); s = sin(ang); d = 1.0f-c;
    // column 0
    R[ 0] = u0*u0*d+c;
    R[ 1] = u1*u0*d+u2*s;
    R[ 2] = u0*u2*d-u1*s;
    R[ 3] = 0.0f;
    // column 1
    R[ 4] = u0*u1*d-u2*s;
    R[ 5] = u1*u1*d+c;
    R[ 6] = u1*u2*d+u0*s;
    R[ 7] = 0.0f;
    // column 2
    R[ 8] = u0*u2*d+u1*s;
    R[ 9] = u1*u2*d-u0*s;
    R[10] = u2*u2*d+c;
    R[11] = 0.0f;
    //  column 3
    R[12] = 0.0f;
    R[13] = 0.0f;
    R[14] = 0.0f;
    R[15] = 1.0f;
  } else { // identity matrix
    R[ 0] = 1.0f; R[ 4] = 0.0f; R[ 8] = 0.0f; R[12] = 0.0f;
    R[ 1] = 0.0f; R[ 5] = 1.0f; R[ 9] = 0.0f; R[13] = 0.0f;
    R[ 2] = 0.0f; R[ 6] = 0.0f; R[10] = 1.0f; R[14] = 0.0f;
    R[ 3] = 0.0f; R[ 7] = 0.0f; R[11] = 0.0f; R[15] = 1.0f;
  }
}

void StaticRotation::vectorToMatrix(float* r /*[4]*/, float* R /*[16]*/) {
  vectorToMatrix(r[0],r[1],r[2],r[3],R);
}

void StaticRotation::matrixToVector(float* R /*[16]*/, float* r /*[4]*/) {
  
  // R  = I + s*u^ - (1-c)*(I-u*u')
  // R' = I - s*u^ - (1-c)*(I-u*u')
  
  // (R-R')/2 = s*u^
  double su2 = (R[1]-R[4])/2.0;
  double su1 = (R[8]-R[2])/2.0;
  double su0 = (R[6]-R[9])/2.0;
  double s   = su0*su0+su1*su1+su2*su2;
  if(s>0.0) s = sqrt(s);
  
  // I-(R+R')/2 = (1-c)(I-u*u') =>
  // c = 1-trace(I-(R+R')/2)/2 =  1-trace(I-R)/2
  double c   = 1.0-(3.0-R[0]-R[5]-R[10])/2.0;
  
  if(s!=0.0) {
    r[0] = (float) (180.0*atan2(s,c)/M_PI);
    r[1] = (float)(su0/s);
    r[2] = (float)(su1/s);
    r[3] = (float)(su2/s);
  } else {
    r[0] = r[1] = r[2] = r[3] = 0.0f;
  }
}

void StaticRotation::multiplyMatrices
(float* A /*[16]*/, float* B /*[16]*/, float* C /*[16]*/) {
  // C = A*B
  // 
  // C[ 0] C[ 4] C[ 8] C[12]
  // C[ 1] C[ 5] C[ 9] C[13]
  // C[ 2] C[ 6] C[10] C[14]
  // C[ 3] C[ 7] C[11] C[15]
  int i,j,k,ij;
  for(ij=j=0;j<4;j++)
    for(i=0;i<4;i++,ij++)
      for(C[ij]=0.0f,k=0;k<4;k++)
        C[ij] += A[i+4*k]*B[k+4*j];
}

void StaticRotation::multiplyMatricesLeft
(float* A /*[16]*/, float* B /*[16]*/) {
  float C[16];
  multiplyMatrices(A,B,C);
  int i;
  for(i=0;i<16;i++)
    B[i]=C[i];
}

void StaticRotation::vectorMultiplyLeft
(float ang_deg, float u0, float u1, float u2, float* r /*[4]*/) {
  float A[16];
  vectorToMatrix(ang_deg, u0, u1, u2, A);
  float B[16];
  vectorToMatrix(r, B);
  float C[16];
  multiplyMatrices(A,B,C);
  matrixToVector(C,r);
}

void StaticRotation::crossProduct
    (double* x /*[3]*/, double* y /*[3]*/, double* z /* z[3]=cross(x,y) */) {
    z[0] = x[1]*y[2]-x[2]*y[1]; 
    z[1] = x[2]*y[0]-x[0]*y[2]; 
    z[2] = x[0]*y[1]-x[1]*y[0]; 
  }
