//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 11:54:21 taubin>
//------------------------------------------------------------------------
//
// PixelTexture.h
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

#ifndef _PixelTexture_h_
#define _PixelTexture_h_

// PixelTexture {
//   exposedField SFImage image 0 0 0
//   field SFBool repeatS TRUE
//   field SFBool repeatT TRUE
// }


#include "Node.h"

class PixelTexture : public Node {

private:

  // TODO Wed Nov  7 01:39:32 2012
  // Image _image;

  bool _repeatS;
  bool _repeatT;

public:
  
  PixelTexture();
  virtual ~PixelTexture();

  bool getRepeatS();
  bool getRepeatT();

  void setRepeatS(bool value);
  void setRepeatT(bool value);

  virtual bool    isPixelTexture() const { return           true; }
  virtual string  getType()        const { return "PixelTexture"; }
  typedef bool    (*Property)(PixelTexture& pixelTexture);
  typedef void    (*OPerator)(PixelTexture& pixelTexture);

  virtual void    printInfo(string indent);
};

#endif /* _PixelTexture_h_ */
