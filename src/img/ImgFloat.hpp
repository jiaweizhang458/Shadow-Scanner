//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 13:32:39 taubin>
//------------------------------------------------------------------------
//
// ImgFloat.java
//
// Copyright (c) 2009-2012, Gabriel Taubin
// All rights reserved.

#ifndef _ImgFloat_hpp_
#define _ImgFloat_hpp_

#include <util/Vec.hpp>
#include "Img.hpp"

class ImgFloat {

protected:
  int      _width;
  int      _height;
  VecFloat _pixel;

public:

  ImgFloat();
  ImgFloat(int w, int h, float value=0.0f);
  ImgFloat(ImgFloat& src);
  ImgFloat(Img& img, int channel);

  VecFloat& getPixel();
  void  setDimensions(int w, int h);
  void  copy(ImgFloat& src);
  void  getChannel(Img& dst, int channel);
  void  setChannel(Img& src, int channel);
  int   getHeight();
  int   getWidth();
  float get(int x, int y);
  void  set(float value);
  void  clear();
  void  add(float value);
  void  multiply(float value);
  void  _set(int x, int y, float value);
  void  set(int x, int y, float value);
  void  _add(int x, int y, float value);
  void  add(int x, int y, float value);
  void  _multiply(int x, int y, float value);
  void  multiply(int x, int y, float value);
  void  setMax(int x, int y, float value);
  void  setMin(int x, int y, float value);

};

#endif // _ImgFloat_hpp_
