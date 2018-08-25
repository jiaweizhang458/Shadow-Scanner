//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 15:16:33 taubin>
//------------------------------------------------------------------------
//
// SceneGraph.cpp
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
#include "SceneGraph.h"
#include "SceneGraphTraversal.h"
#include "Shape.h"
#include "Appearance.h"
  
SceneGraph::SceneGraph() {
  _parent = this;
}

SceneGraph::~SceneGraph() {
}

void SceneGraph::clear() {
  pNode node;
  while(_children.size()>0) {
    node = _children.back(); _children.pop_back();
    delete node;
  }
}

string& SceneGraph::getUrl() {
  return _url;
}

void SceneGraph::setUrl(const string& url) {
  _url = url;
}

Node* SceneGraph::find(const string& name) {
  Node* node = (Node*)0;
  SceneGraphTraversal t(*this); t.start();
  while((node=t.next())!=(Node*)0) {
    if(node->nameEquals(name)) break;
    if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getAppearance();
      if(node!=(Node*)0) {
        if(node->nameEquals(name)) break;
        Appearance* appearance = (Appearance*)node;
        node = appearance->getMaterial();
        if(node!=(Node*)0 && node->nameEquals(name)) break;
        node = appearance->getTexture();
        if(node!=(Node*)0 && node->nameEquals(name)) break;
      }
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->nameEquals(name)) break;
    }
  }
  return node;
}

void SceneGraph::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "SceneGraph {\n";
  std::cout << indent << "  " << "_url = \"" << _url << "\"\n";
  std::cout << indent << "  " << "children [\n";
  int nChildren = getNumberOfChildren();
  for(int i=0;i<nChildren;i++) {
    Node* node = (*this)[i];
    node->printInfo(indent+"    ");
  }
  std::cout << indent << "  " << "]\n";
  std::cout << indent << "}\n";
}
