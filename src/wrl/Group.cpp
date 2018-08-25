//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 21:55:41 taubin>
//------------------------------------------------------------------------
//
// Group.cpp
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
#include <math.h>
#include <algorithm>
#include "Transform.h"
#include "Shape.h"
#include "IndexedFaceSet.h"
#include "IndexedLineSet.h"
  
Group::Group():
_bboxCenter(0.0f,0.0f,0.0f),
_bboxSize(-1.0f,-1.0f,-1.0f) {
}

Group::~Group() {
  pNode child;
  while(_children.size()>0) {
    child = _children.back();
    _children.pop_back();
    delete child;
  }
}

vector<pNode>& Group::getChildren() {
  return _children;
}

Node* Group::getChild(const string& name) const {
  Node* node = (Node*)0;
  for(int i=0;i<(int)_children.size();i++)
    if(_children[i]->nameEquals(name)) {
      node = _children[i];
      break;
    }
  return node;
}

int Group::getNumberOfChildren() const {
  return (int)(_children.size());
}

pNode Group::operator[](const int i) {
  pNode node = (pNode)0;
  if(0<=i && i<(int)_children.size())
    node = _children[i];
  return node;
}

Vec3f& Group::getBBoxCenter() {
  return _bboxCenter;
}

Vec3f& Group::getBBoxSize() {
  return _bboxSize;
}

float Group::getBBoxDiameter() {
  float diam2 =
    _bboxSize.x*_bboxSize.x+
    _bboxSize.y*_bboxSize.y+
    _bboxSize.z*_bboxSize.z;
  return (diam2>0.0f)?(float)sqrt(diam2):0.0f;
}

void Group::addChild(const pNode child) {
  child->setParent(this);
  _children.push_back(child);
}

void Group::removeChild(const pNode child) {
  vector<Node*>::iterator node;
  node = find(_children.begin(),_children.end(),child);
  if(node!=_children.end()) {
    _children.erase(node);
    delete &(*node);
  }
}

void Group::setBBoxCenter(Vec3f& value) {
  _bboxCenter = value;
}
void Group::setBBoxSize(Vec3f& value) {
  _bboxSize = value;
}

void Group::clearBBox() {
  _bboxCenter.x = _bboxCenter.y = _bboxCenter.z = 0.0f;
  _bboxSize.x   = _bboxSize.y   = _bboxSize.z = -1.0f;
}
bool Group::hasEmptyBBox() const {
  return (_bboxSize.x<=0.0f ||_bboxSize.y<=0.0f ||_bboxSize.z<=0.0f);
}

void Group::appendBBoxCoord(vector<float>& coord) {
  if(hasEmptyBBox()==false) {
    Vec3f min;
    Vec3f max;
    min.x = _bboxCenter.x-0.5f*_bboxSize.x;
    min.y = _bboxCenter.y-0.5f*_bboxSize.y;
    min.z = _bboxCenter.z-0.5f*_bboxSize.z;
    max.x = _bboxCenter.x+0.5f*_bboxSize.x;
    max.y = _bboxCenter.y+0.5f*_bboxSize.y;
    max.z = _bboxCenter.z+0.5f*_bboxSize.z;
    coord.push_back(min.x); coord.push_back(min.y); coord.push_back(min.z);
    coord.push_back(min.x); coord.push_back(min.y); coord.push_back(max.z);
    coord.push_back(min.x); coord.push_back(max.y); coord.push_back(min.z);
    coord.push_back(min.x); coord.push_back(max.y); coord.push_back(max.z);
    coord.push_back(max.x); coord.push_back(min.y); coord.push_back(min.z);
    coord.push_back(max.x); coord.push_back(min.y); coord.push_back(max.z);
    coord.push_back(max.x); coord.push_back(max.y); coord.push_back(min.z);
    coord.push_back(max.x); coord.push_back(max.y); coord.push_back(max.z);
  }
}

void Group::updateBBox(vector<float>& coord) {
  if(coord.size()>=3) {
    if(hasEmptyBBox()) {
        _bboxCenter.x = coord[0];
        _bboxCenter.y = coord[1];
        _bboxCenter.z = coord[2];
        _bboxSize.x = _bboxSize.y = _bboxSize.z = 0.0f;
    }
    Vec3f min;
    Vec3f max;
    min.x = _bboxCenter.x-0.5f*_bboxSize.x;
    min.y = _bboxCenter.y-0.5f*_bboxSize.y;
    min.z = _bboxCenter.z-0.5f*_bboxSize.z;
    max.x = _bboxCenter.x+0.5f*_bboxSize.x;
    max.y = _bboxCenter.y+0.5f*_bboxSize.y;
    max.z = _bboxCenter.z+0.5f*_bboxSize.z;
    int nCoord = (int)(coord.size()/3);
    for(int i=0;i<nCoord;i++) {
      Vec3f p(coord[3*i+0],coord[3*i+1],coord[3*i+2]);
      if(p.x<min.x) min.x = p.x; else if(p.x>max.x) max.x = p.x;
      if(p.y<min.y) min.y = p.y; else if(p.y>max.y) max.y = p.y;
      if(p.z<min.z) min.z = p.z; else if(p.z>max.z) max.z = p.z;
    }
  _bboxCenter.x = (max.x+min.x)/2.0f;
  _bboxCenter.y = (max.y+min.y)/2.0f;
  _bboxCenter.z = (max.z+min.z)/2.0f;
  _bboxSize.x   = (max.x-min.x);
  _bboxSize.y   = (max.y-min.y);
  _bboxSize.z   = (max.z-min.z);
  }
}

void Group::updateBBox() {
  int nChildren = getNumberOfChildren();
  for(int i=0;i<nChildren;i++) {
    Node* node = (*this)[i];
    if(node->isTransform()) {
      Transform* transform = (Transform*)node;
      transform->updateBBox();
      // get vertices of bounding box
      vector<float> coord;
      transform->appendBBoxCoord(coord);
      // apply the transform to thoe vertices
      // TODO Thu Nov 08 18:25:47 2012
      // update this group bounding box
      updateBBox(coord);
    } else if(node->isGroup()) {
      Group* group = (Group*)node;
      group->updateBBox();
      // get vertices of bounding box
      vector<float> coord;
      group->appendBBoxCoord(coord);
      // update this group bounding box
      updateBBox(coord);
    } else if(node->isShape()) {
      Shape* shape = (Shape*)node;
      node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        IndexedFaceSet* pIfs = (IndexedFaceSet*)node;
        vector<float> &coord = pIfs->getCoord();    
        // update this group bounding box
        updateBBox(coord);
      } else if(node!=(Node*)0 && node->isIndexedLineSet()) {
        IndexedLineSet* pIls = (IndexedLineSet*)node;
        vector<float> &coord = pIls->getCoord();    
        // update this group bounding box
        updateBBox(coord);
      }
    }
  }
}

void Group::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "Group {\n";
  std::cout << indent << "  " << "children [\n";
  int nChildren = getNumberOfChildren();
  for(int i=0;i<nChildren;i++) {
    Node* node = (*this)[i];
    node->printInfo(indent+"    ");
  }
  std::cout << indent << "  " << "]\n";
  std::cout << indent << "}\n";
}
