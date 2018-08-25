//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 12:20:32 taubin>
//------------------------------------------------------------------------
//
// Rotation.cpp
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

#include "Rotation.h"

Rotation::Rotation():
  _axis(0.0f,0.0f,1.0f),
  _angle(0.0f) {
}
 
Rotation::Rotation(float x, float y, float z, float angle):
  _axis(x,y,z),
  _angle(angle) {
}

Rotation::Rotation(Vec3f& axis, float angle):
  _axis(axis),
  _angle(angle) {
}

void Rotation::set(float x, float y, float z, float angle) {
  _axis.x = x; _axis.y = y; _axis.z = z;
  _angle  = angle;
}
void Rotation::set(Vec4f& value) {
  _axis.x = value.x; _axis.y = value.y; _axis.z = value.z;
  _angle  = value.w;
}
void Rotation::operator=(Vec4f& value) {
  _axis.x = value.x; _axis.y = value.y; _axis.z = value.z;
  _angle  = value.w;
}

Vec3f& Rotation::getAxis() {
  return _axis;
}
float  Rotation::getAngle() {
  return _angle;
}
