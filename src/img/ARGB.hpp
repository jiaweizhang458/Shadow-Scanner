//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-27 20:33:21 taubin>
//------------------------------------------------------------------------
//
// ARGB.hpp
//
// Copyright (c) 2014, Gabriel Taubin
// All rights reserved.

#ifndef _ARGB_hpp_
#define _ARGB_hpp_

class ARGB {

public:

  enum Channel {
    INTENSITY,ALPHA,RED,GREEN,BLUE
  };
   
  static int getA(int pixel) {
    return ((pixel>>24)&0xff);
  }
  static int getR(int pixel) {
    return ((pixel>>16)&0xff);
  }
  static int getG(int pixel) {
    return ((pixel>>8)&0xff);
  }
  static int getB(int pixel) {
    return (pixel&0xff);
  }
  static int getI(int pixel) {
    int b = pixel&0xff; pixel>>=8;
    int g = pixel&0xff; pixel>>=8;
    int r = pixel&0xff;
    return 0xff&((int)(0.299f*r+0.587*g+0.114*b));
  }

  static int argb(int a, int r, int g, int b) {
    return ((a&0xff)<<24)|((r&0xff)<<16)|((g&0xff)<<8)|((b&0xff)<<0);
  }
  static int argb(int r, int g, int b) {
    return argb(0xff,r,g,b);
  }
  static int setA(int pixel, int a) {
    pixel &= 0x00ffffff; pixel |= (a&0xff)<<24; return pixel;
  }
  static int setR(int pixel, int r) {
    pixel &= 0xff00ffff; pixel |= (r&0xff)<<16; return pixel;
  }
  static int setG(int pixel, int g) {
    pixel &= 0xffff00ff; pixel |= (g&0xff)<< 8; return pixel;
  }
  static int setB(int pixel, int b) {
    pixel &= 0xffffff00; pixel |= (b&0xff)<< 0; return pixel;
  }
  static int setI(int pixel, int gr) {
    return argb(getA(pixel),gr,gr,gr);
  }
  static int setI(int pixel) {
    int gr = getI(pixel);
    return argb(getA(pixel),gr,gr,gr);
  }
  static int modulate(int pixel0, float t) {
    int pixel = 0xff000000;
    if(t>=1.0f) {
      pixel = pixel0;
    } else if(t>0.0f) {
      float fr = t*((float)getR(pixel0)); int r = 0xff&((int)fr);
      float fg = t*((float)getG(pixel0)); int g = 0xff&((int)fg);
      float fb = t*((float)getB(pixel0)); int b = 0xff&((int)fb);
      pixel = argb(0xff,r,g,b);
    }
    return pixel;
  }
  static int blend(int pixel0, int pixel1, float t) {
    int pixel = 0xff000000;
    if(t<=0.0f) {
      pixel = pixel0;
    } else if(t>=1.0f) {
      pixel = pixel1;
    } else {
      int /*a0,*/r0,g0,b0,/*a1,*/r1,g1,b1,/*a,*/r,g,b;
      // a0 = getA(pixel0); a1 = getA(pixel1);
      r0 = getR(pixel0); r1 = getR(pixel1);
      g0 = getG(pixel0); g1 = getG(pixel1);
      b0 = getB(pixel0); b1 = getB(pixel1);
      float t0,t1;
      t0 = 1.0f-t; t1 = t;
      // a = 0xff&((int)(t0*((float)a0)+t1*((float)a1)));
      r = 0xff&((int)(t0*((float)r0)+t1*((float)r1)));
      g = 0xff&((int)(t0*((float)g0)+t1*((float)g1)));
      b = 0xff&((int)(t0*((float)b0)+t1*((float)b1)));
      pixel = argb(0xff /*a*/,r,g,b);
    }
    return pixel;
  }
};

#endif // _ARGB_hpp_

