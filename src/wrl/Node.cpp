//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 11:55:15 taubin>
//------------------------------------------------------------------------
//
// Node.cpp
//
// Software developed for the Fall 2012 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2012, Gabriel Taubin
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
#include "Node.h"
  
Node::Node():
  _name(""),
  _parent((Node*)0),
  _show(true) {
}

Node::~Node() {
}

const string& Node::getName() const {
  return _name;
}

void Node::setName(const string& name) {
  _name = name;
}

bool Node::nameEquals(const string& name) {
  // return (strcmp(_name.c_str(),name.c_str())==0);
  return (_name==name);
}


const Node* Node::getParent() const {
  return _parent;
}
void Node::setParent(const Node* node) {
  _parent = node;
}

bool Node::getShow() const {
  return _show;
}

void Node::setShow(bool value) {
  _show = value;
}

int Node::getDepth() const {
  int d = 0;
  const Node* p = _parent;
  while((p!=p->_parent)) {
    if(p->isGroup())
      d++;
    p = p->_parent;
  }

  return d;
}

bool    Node::isAppearance() const     { return  false; }
bool    Node::isGroup() const          { return  false; }
bool    Node::isImageTexture() const   { return  false; }
bool    Node::isIndexedFaceSet() const { return  false; }
bool    Node::isIndexedLineSet() const { return  false; }
bool    Node::isMaterial() const       { return  false; }
bool    Node::isPixelTexture() const   { return  false; }
bool    Node::isSceneGraph() const     { return  false; }
bool    Node::isShape() const          { return  false; }
bool    Node::isTransform() const      { return  false; }

string  Node::getType() const          { return "Node"; }

void    Node::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "Node {\n";
  std::cout << indent << "}\n";
}
