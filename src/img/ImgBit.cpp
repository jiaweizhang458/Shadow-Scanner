//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-27 22:03:13 taubin>
//------------------------------------------------------------------------
//
// ImgBit.cpp
//
// Copyright (c) 2014, Gabriel Taubin
// All rights reserved.

#include "ImgBit.hpp"

int ImgBit::_bgColor = 0xff000000;
int ImgBit::_fgColor = 0xffffff;

void ImgBit::setBgColor(int bgColor) { _bgColor = bgColor; }
void ImgBit::setFgColor(int fgColor) { _fgColor = fgColor; }


int  ImgBit::getHeight()         { return _height; }
int  ImgBit::getWidth()          { return _width; }
int  ImgBit::getNumberOfPixels() { return _width*_height; }

void ImgBit::setDimensions(int w, int h) {
  if(w>0 && h>0 && (w!=_width || h!=_height) ) {
    _width  = w;
    _height = h;
    _data.clear();
    _data.pushBackN(w*h,false);
  }
}

ImgBit::ImgBit(int w, int h): _width(0), _height(0), _data() {
  if(w>0 && h>0) {
    _width  = w;
    _height = h;
    _data.clear();
    _data.pushBackN(w*h,false);
  }
}

ImgBit::ImgBit(ImgBit& src): _width(0), _height(0), _data() {
  copy(src);
}

void ImgBit::_set(int x, int y, bool value) {
  _data.set(x+y*_width,value);
}

void ImgBit::set(int x, int y, bool value) {
  if(0<=x && x<_width && 0<=y && y<_height)
    _set(x,y,value);
}

bool ImgBit::_get(int x, int y) {
  return _data.get(x+y*_width);
}

bool ImgBit::get(int x, int y) {
  return (0<=x && x<_width && 0<=y && y<_height)?
    _get(x,y):false;
}

void ImgBit::copy(ImgBit& src) {
  // if(src==null) return;
  setDimensions(src._width,src._height);
  int n = _width*_height;
  for(int i=0;i<n;i++)
    _data.set(i,src._data.get(i));
}

void ImgBit::swap(ImgBit& dst) {
  int   iWidth  = dst._width;
  dst._width = _width; _width = iWidth;
  int   iHeight = dst._height;
  dst._height = _height; _height = iHeight;
  _data.swap(dst._data);
}

void ImgBit::set(bool value) {
  int n = _width*_height;
  for(int i=0;i<n;i++)
    _data.set(i,value);
}

void ImgBit::clear() {
  set(false);
}
