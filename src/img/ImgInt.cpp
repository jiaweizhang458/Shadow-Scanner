//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 14:27:08 taubin>
//------------------------------------------------------------------------
//
// ImgInt.cpp
//
// Copyright (c) 2009-2014, Gabriel Taubin
// All rights reserved.

#include "ImgInt.hpp"

// private static

int ImgInt::_bgColor = 0xff555555;
int ImgInt::_fgColor = 0xffffffff;

// public static

void ImgInt::setBgColor(int bgColor) { _bgColor = bgColor; }
void ImgInt::setFgColor(int fgColor) { _fgColor = fgColor; }

// int   _width   = 0;
// int   _height  = 0;
// VecInt _pixel   = null;

ImgInt::ImgInt():
  _width(0),
  _height(0),
  _pixel() {
}

ImgInt::ImgInt(int w, int h, int bgColor):
  _width(0),
  _height(0),
  _pixel() {
  if(w>0 && h>0) {
    _width  = w;
    _height = h;
    int n = _width*_height;
    _pixel.reserve(n);
    _pixel.insert(0,n,bgColor);
  }
}

ImgInt::ImgInt(ImgInt& src):
  _width(0),
  _height(0),
  _pixel() {
  copy(src);
}

ImgInt::ImgInt(int w, int h, VecInt& pixel):
  _width(0),
  _height(0),
  _pixel() {
  if(w>0 && h>0) {
    _width  = w;
    _height = h;
    int n = _width*_height;
    _pixel.reserve(n);
    _pixel.insert(0,n,_bgColor);
    set(pixel);
  }
}

void ImgInt::setDimensions(int w, int h) {
  if(w>0 && h>0 && (w!=_width || h!=_height) ) {
    _width  = w;
    _height = h;
    _pixel.clear();
    int n = _width*_height;
    _pixel.reserve(n);
    _pixel.insert(0,n,_bgColor);
    set(_bgColor);
  }
}

bool ImgInt::equals(ImgInt& img) {
  bool success = false;
  if(_width==img._width && _height==img._height) {
    int n = _width*_height;
    success = true;
    for(int i=0;i<n;i++)
      if(_pixel[i]!=img._pixel[i]) {
        success = false;
        break;
      }
  }
  return success;
}

int ImgInt::getHeight() { return _height; }
int ImgInt::getWidth()  { return _width; }

VecInt& ImgInt::getPixel() {
  return _pixel;
}

bool ImgInt::isConstant() {
  int i,p;
  for(p=_pixel[0],i=0;i<_pixel.size() &&p==_pixel[i];i++);
  return (i==_pixel.size());
}

void ImgInt::copy(ImgInt& src) {
  int x,y,i;
  _width  = src._width;
  _height = src._height;
  _pixel.clear();
  int n = _width*_height;
  _pixel.reserve(n);
  _pixel.insert(0,n,_bgColor);
  for(i=x=0;x<_width;x++) {
    for(y=0;y<_height;y++,i++) {
      _pixel[i] = src._pixel[i];
    }
  }
}

bool ImgInt::isSameSize(ImgInt& img) {
  return (_width==img._width&&_height==img._height);
}

void ImgInt::swap(ImgInt& dst) {
  int   iWidth  = dst._width;
  dst._width = _width; _width = iWidth;
  int   iHeight = dst._height;
  dst._height = _height; _height = iHeight;
  _pixel.swap(dst._pixel);
}

int ImgInt::get(int x, int y) {
  return (x>=0 && x<_width && y>=0 && y<_height)?
    _pixel[x+y*_width]:_bgColor;
}

void ImgInt::setBackgroundColor(int bgColor) {
  _bgColor = bgColor;
}

void ImgInt::setForegroundColor(int fgColor) {
  _fgColor = fgColor;
}

void ImgInt::setPixel(VecInt& pixel) {
  if(pixel.size()==_width*_height)
    _pixel = pixel;
}

void ImgInt::set(VecInt& pixel) {
  for(int i=0;i<_pixel.size();i++)
    _pixel[i] = (i<pixel.size())?pixel[i]:_bgColor;
}

void ImgInt::set(int value) {
  for(int i=0;i<_pixel.size();i++)
    _pixel[i] = value;
}

void ImgInt::clear() {
  set(_bgColor);
}

void ImgInt::set(int x, int y, int value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    _set(x,y,value);
  }
}

void ImgInt::_set(int x, int y, int value) { // no checking
  _pixel[x+y*_width] = value;
}
