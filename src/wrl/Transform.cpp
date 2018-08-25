//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 12:40:54 taubin>
//------------------------------------------------------------------------
//
// Transform.cpp
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

#include <math.h>
#include <iostream>
#include "Transform.h"
  
Transform::Transform():
  _center(0.0f,0.0f,0.0f),
  _rotation(0.0f,0.0f,1.0f,0.0f),
  _scale(1.0f,1.0f,1.0f),
  _scaleOrientation(0.0f,0.0f,1.0f,0.0f),
  _translation(0.0f,0.0f,0.0f) {
}

Transform::~Transform() {

}

Vec3f&    Transform::getCenter()                     {  return           _center; }
Rotation& Transform::getRotation()                   {  return         _rotation; }
Vec3f&    Transform::getScale()                      {  return            _scale; }
Rotation& Transform::getScaleOrientation()           {  return _scaleOrientation; }
Vec3f&    Transform::getTranslation()                {  return      _translation; }

void Transform::setCenter(Vec3f& value)              {           _center = value; }
void Transform::setRotation(Rotation& value)         {         _rotation = value; }
void Transform::setScale(Vec3f& value)               {            _scale = value; }
void Transform::setScaleOrientation(Rotation& value) { _scaleOrientation = value; }
void Transform::setTranslation(Vec3f& value)         {      _translation = value; }

void Transform::setRotation(Vec4f& value) {
  _rotation = value;
}

void Transform::setScaleOrientation(Vec4f& value) {
  _scaleOrientation = value;
}

void Transform::getMatrix(float* M /*[16]*/) {
  M[ 0] = 1.0f; M[ 1] = 0.0f; M[ 2] = 0.0f; M[ 3] = 0.0f;
  M[ 4] = 0.0f; M[ 5] = 1.0f; M[ 6] = 0.0f; M[ 7] = 0.0f;
  M[ 8] = 0.0f; M[ 9] = 0.0f; M[10] = 1.0f; M[11] = 0.0f;
  M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f;

  // TODO Thu Nov 08 15:02:28 2012
  // center           C
  // rotation         R
  // scale            S
  // scaleOrientation O
  // translation      T

  // M = T x C x R x O x S x O^{t} x C^{-1}

  //
  // p |-> [ROSO^t]*p + [(I-ROSO^t)*c+t]
  //
  float O[9];
  _makeRotation(_scaleOrientation,O);
  float s[3] = { _scale.x, _scale.y, _scale.z };
  float OSOt[9];
  int i,ii,j,jj,k,kk;
  for(ii=i=0;i<3;ii+=3,i++)
    for(jj=j=0;j<3;jj+=3,j++)
      for(OSOt[ii+j]=0.0f,k=0;k<3;k++)
        OSOt[ii+j] += O[ii+k]*s[k]*O[jj+k];
  // A = R*O*S*O^t
  float R[9];
  _makeRotation(_rotation,R);
  float A[9];
  for(ii=i=0;i<3;ii+=3,i++)
    for(jj=j=0;j<3;jj+=3,j++)
      for(A[ii+j]=0.0f,kk=k=0;k<3;kk+=3,k++)
        A[ii+j] += R[ii+k]*OSOt[kk+j];
  // B = (I-A)*C+T
  float c[3] = { _center.x, _center.y, _center.z};
  float t[3] = { _translation.x, _translation.y, _translation.z };
  float B[3];
  for(ii=i=0;i<3;ii+=3,i++)
    for(B[i]=c[i]+t[i],j=0;j<3;j++)
      B[i] -= A[ii+j]*c[j];
  //     | A B |
  // M = |     |
  //     | 0 1 |
  M[ 0] = A[0]; M[ 1] = A[1]; M[ 2] = A[2]; M[ 3] = B[0]; 
  M[ 4] = A[3]; M[ 5] = A[4]; M[ 6] = A[5]; M[ 7] = B[1]; 
  M[ 8] = A[6]; M[ 9] = A[7]; M[10] = A[8]; M[11] = B[2]; 
  M[12] = 0.0f; M[13] = 0.0f; M[14] = 0.0f; M[15] = 1.0f; 
}

void Transform::_makeRotation(Rotation& r, float* R /*[9]*/) {
  Vec3f& axis  = r.getAxis();
  float  angle = r.getAngle();
  if(angle==0.0f) {
    R[0] = 1.0f; R[1] = 0.0f; R[2] = 0.0f;
    R[3] = 0.0f; R[4] = 1.0f; R[5] = 0.0f;
    R[6] = 0.0f; R[7] = 0.0f; R[8] = 1.0f; 
  } else {
    float x = axis.x;
    float y = axis.y;
    float z = axis.z;
    float c = (float)cos(angle);
    float s = (float)sin(angle);
    float t = 1.0f-c;
    R[0] =    c+t*x*x; R[1] = -s*z+t*x*y; R[2] =  s*y+t*x*z;
    R[3] =  s*z+t*x*y; R[4] =    c+t*y*y; R[5] = -s*x+t*y*z;
    R[6] = -s*y+t*x*z; R[7] =  s*x+t*y*z; R[8] =    c+t*z*z; 
  }
}

void Transform::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "Transform {\n";
  std::cout << indent << "  " << "children [\n";
  int nChildren = getNumberOfChildren();
  for(int i=0;i<nChildren;i++) {
    Node* node = (*this)[i];
    node->printInfo(indent+"    ");
  }
  std::cout << indent << "  " << "]\n";
  std::cout << indent << "}\n";
}
