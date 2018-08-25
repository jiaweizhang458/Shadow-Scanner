//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 13:32:55 taubin>
//------------------------------------------------------------------------
//
// ImgInt.java
//
// Copyright (c) 2009-2014, Gabriel Taubin
// All rights reserved.

#ifndef _ImgInt_hpp_
#define _ImgInt_hpp_

#include <util/Vec.hpp>
#include "Img.hpp"

class ImgInt: public Img {

protected: // static

  static int   _bgColor;
  static int   _fgColor;

public: // static

  static void setBgColor(int bgColor);
  static void setFgColor(int fgColor);

protected:

  int    _width;
  int    _height;
  VecInt _pixel;

public:

  ImgInt();
  ImgInt(int w, int h, int bgColor=_bgColor);
  ImgInt(ImgInt& src);
  ImgInt(int w, int h, VecInt& pixel);

  void    setDimensions(int w, int h);
  bool    equals(ImgInt& img);
  VecInt& getPixel();
  bool    isConstant();
  void    copy(ImgInt& src);
  bool    isSameSize(ImgInt& img);
  void    swap(ImgInt& dst);
  void    setBackgroundColor(int bgColor);
  void    setForegroundColor(int fgColor);
  void    setPixel(VecInt& pixel);
  void    set(VecInt& pixel);
  void    set(int value);
  void    clear();

  // Img
  int     getHeight();
  int     getWidth();
  int     get(int x, int y);
  void    set(int x, int y, int value);

protected:
  
  void    _set(int x, int y, int value);
  
};

#endif // _ImgInt_hpp_
