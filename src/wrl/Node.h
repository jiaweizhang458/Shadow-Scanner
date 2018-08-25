//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 11:53:17 taubin>
//------------------------------------------------------------------------
//
// Node.h
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

#ifndef _Node_h_
#define _Node_h_

#include <string>

using namespace std;

class Color {
public:
  float r,g,b;
public:
  Color(float R=0.0f, float G=0.0f, float B=0.0f):r(R),g(G),b(B) {}
  Color& operator=(Color& src) {
    r=src.r; g=src.g; b=src.b;
    return *this;
  }
};

class Vec2f {
public:
  float x,y;
public:
  Vec2f(float X=0.0f, float Y=0.0f):x(X),y(Y) {}
  Vec2f& operator=(Vec2f& src) {
    x=src.x; y=src.y;
    return *this;
  }
};

class Vec3f {
public:
  float x,y,z;
public:
  Vec3f(float X=0.0f, float Y=0.0f, float Z=0.0f):x(X),y(Y),z(Z) {}
  Vec3f& operator=(Vec3f& src) {
    x=src.x; y=src.y; z=src.z;
    return *this;
  }
};

class Vec4f {
public:
  float x,y,z,w;
public:
  Vec4f(float X=0.0f, float Y=0.0f, float Z=0.0f, float W=0.0f):
    x(X),y(Y),z(Z),w(W) {}
  Vec4f& operator=(Vec4f& src) {
    x=src.x; y=src.y; z=src.z; w=src.w;
    return *this;
  }
};

class Node {

protected:

  string      _name;
  const Node* _parent;
  bool        _show;

public:
  
  Node();
  virtual ~Node();

  const string&   getName() const;
  void            setName(const string& name);
  bool            nameEquals(const string& name);
  const Node*     getParent() const;
  void            setParent(const Node* node);
  bool            getShow() const;
  void            setShow(bool value);
  int             getDepth() const; 

  virtual bool    isAppearance() const;
  virtual bool    isGroup() const;
  virtual bool    isImageTexture() const;
  virtual bool    isIndexedFaceSet() const;
  virtual bool    isIndexedLineSet() const;
  virtual bool    isMaterial() const;
  virtual bool    isPixelTexture() const;
  virtual bool    isSceneGraph() const;
  virtual bool    isShape() const;
  virtual bool    isTransform() const;
  virtual string  getType() const;

  typedef bool    (*Property)(Node& node);
  typedef void    (*Operator)(Node& node);

  virtual void    printInfo(string indent);
};

typedef Node *pNode;

#endif /* _Node_h_ */
