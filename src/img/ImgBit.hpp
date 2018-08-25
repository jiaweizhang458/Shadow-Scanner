//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-27 22:05:30 taubin>
//------------------------------------------------------------------------
//
// ImgBit.hpp
//
// Copyright (c) 2014, Gabriel Taubin
// All rights reserved.

#ifndef _ImgBit_hpp_
#define _ImgBit_hpp_

#include <util/Vec.hpp>

class ImgBit {

protected:

  static int _bgColor;
  static int _fgColor;

public:

  static void setBgColor(int bgColor);
  static void setFgColor(int fgColor);

protected:

  int     _width;
  int    _height;
  VecBit   _data;

public:
  
  int  getHeight();
  int  getWidth();
  int  getNumberOfPixels();

  void setDimensions(int w, int h);

  ImgBit(int w, int h);
  ImgBit(ImgBit& src);

private:

  void _set(int x, int y, bool value);

public:

  void set(int x, int y, bool value);

private:

  bool _get(int x, int y);

public:

  bool get(int x, int y);
  void copy(ImgBit& src);
  void swap(ImgBit& dst);
  void set(bool value);
  void clear();

};

#endif // _ImgBit_hpp_
