//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 14:13:07 taubin>
//------------------------------------------------------------------------
//
// ImgArgb.hpp
//
// Copyright (c) 2009-2014, Gabriel Taubin
// All rights reserved.

#ifndef _ImgArgb_hpp_
#define _ImgArgb_hpp_

#include "Img.hpp"
#include "ImgBitplane.hpp"
#include "ImgBit.hpp"

class QImage;

class ImgArgb: public Img {

 private: // static

  static int   _bgColor;
  static int   _fgColor;
  
 public: // static

  static void setBgColor(int bgColor);
  static void setFgColor(int fgColor);

 private:

  int          _bitMask;
  VecInt       _bitColor;
  ImgBitplane* _bitPlane;

 protected:

  int          _width;
  int         _height;
  VecInt       _pixel;

 private:

  void _init(int w, int h, int bgColor=_bgColor);

 public:

 ~ImgArgb();
  ImgArgb();
  ImgArgb(int w, int h, int bgColor=_bgColor);
  ImgArgb(Img& src, bool gray=false);
  ImgArgb(int w, int h, VecInt& pixel);
  ImgArgb(QImage& qImg);

  void    setDimensions(int width, int height);
  ImgBit* getBitplane(int i);
  int     newBitplane();
  void    freeBitplane(int i);
  void    setBitColor(int i, int color);
  int     getBitColor(int i);
  Img*    getIntensity();
  void    setChannel(Img& src, int channel);
  VecInt& getPixel();

  // Img
  int     getWidth();
  int     getHeight();
  int     get(int x, int y);
  void    set(int x, int y, int argb);

};

#endif // _ImgArgb_hpp_
