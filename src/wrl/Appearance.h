//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 11:53:31 taubin>
//------------------------------------------------------------------------
//
// Appearance.h
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

#ifndef _Appearance_h_
#define _Appearance_h_

// Appearance {
//   exposedField SFNode material NULL
//   exposedField SFNode texture NULL
//   exposedField SFNode textureTransform NULL
// }

#include "Node.h"

using namespace std;

class Appearance : public Node {

private:

  Node* _material;
  Node* _texture;
  // Node* _textureTransform;

public:

  Appearance();
  virtual ~Appearance();

  Node* getMaterial();
  Node* getTexture();
  // Node* getTextureTransform();

  void setMaterial(Node* material);
  void setTexture(Node* texture);
  // void setTextureTransform(Node* textureTransform);

  virtual bool    isAppearance() const { return         true; }
  virtual string  getType()      const { return "Appearance"; };
  typedef bool    (*Property)(Appearance& appearance);
  typedef void    (*Operator)(Appearance& appearance);

  virtual void    printInfo(string indent);
};

#endif /* _Appearance_h_ */
