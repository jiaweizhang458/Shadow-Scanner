//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 12:20:16 taubin>
//------------------------------------------------------------------------
//
// PixelTexture.cpp
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

#include <iostream>
#include "PixelTexture.h"
  
PixelTexture::PixelTexture():
  _repeatS(true),
  _repeatT(true) {

  }

PixelTexture::~PixelTexture() {

}

bool PixelTexture::getRepeatS() {
  return _repeatS;
}
bool PixelTexture::getRepeatT() {
  return _repeatT;
}

void PixelTexture::setRepeatS(bool value) {
  _repeatS = value;
}

void PixelTexture::setRepeatT(bool value) {
  _repeatT = value;
}

void PixelTexture::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "PixelTexture {\n";
  std::cout << indent << "}\n";
}
