//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 13:29:52 taubin>
//------------------------------------------------------------------------
//
// Img.hpp
//
// Copyright (c) 2014, Gabriel Taubin
// All rights reserved.


#ifndef _Img_hpp_
#define _Img_hpp_

#include "ARGB.hpp"
#include <util/Vec.hpp>

class Img {

public:

  virtual int getHeight() = 0;
  virtual int getWidth() = 0;
  virtual int get(int col, int row) = 0; // returns argb
  
  int getNumberOfPixels() { return getWidth()*getHeight(); }
  
  int getA(int col, int row) { return ARGB::getA(get(col,row)); }
  int getR(int col, int row) { return ARGB::getR(get(col,row)); }
  int getG(int col, int row) { return ARGB::getG(get(col,row)); }
  int getB(int col, int row) { return ARGB::getB(get(col,row)); }
  int getI(int col, int row) { return ARGB::getI(get(col,row)); }
  
  int get(int col, int row, ARGB::Channel channel) {
    int argb = 0xff000000;
    switch(channel) {
    case ARGB::INTENSITY: argb = getI(col,row); break;
    case ARGB::ALPHA:     argb = getA(col,row); break;
    case ARGB::RED:       argb = getR(col,row); break;
    case ARGB::GREEN:     argb = getG(col,row); break;
    case ARGB::BLUE:      argb = getB(col,row); break;
    }
    return argb;
  }
  
  int get(int x, int y, int gray) { // 0<= gray <= 255
    gray &= 0xff;
    int argb = get(x,y);
    int a    = ARGB::getA(argb); a*=gray; a>>=8;
    int r    = ARGB::getR(argb); r*=gray; r>>=8;
    int g    = ARGB::getG(argb); g*=gray; g>>=8;
    int b    = ARGB::getB(argb); b*=gray; b>>=8;
    return     ARGB::argb(a,r,g,b);
  }

  virtual void set(int x, int y, int argb) = 0;

  void set(int x, int y, int a, int r, int g, int b);
  void set(int x, int y, int r, int g, int b);
  void setA(int x, int y, int  a);
  void setR(int x, int y, int  r);
  void setG(int x, int y, int  g);
  void setB(int x, int y, int  b);
  void setI(int x, int y, int gr);
  void set(int argb);
  void set(Img& src, bool convertToGray);
  void setGray(Img& src);
  void set(Img& src);
  void set(VecInt& pixel, bool convertToGray);
  void setGray(VecInt& pixel);
  void set(VecInt& pixel);
  
};

#endif // _Img_hpp_
