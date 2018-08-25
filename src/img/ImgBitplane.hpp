//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-28 19:45:33 taubin>
//------------------------------------------------------------------------
//
// ImgBitplane.java
//
// Copyright (c) 2009-2014, Gabriel Taubin
// All rights reserved.

#ifndef _ImgBitplane_hpp_
#define _ImgBitplane_hpp_

#include "ImgInt.hpp"

class ImgBitplane {

public: // static

  static void set
  (VecInt& I, int width, int height,
   int bitplane, bool white);

  static void invert
  (VecInt& I, int width, int height,
   int bitplane);

  static void copy
    (VecInt& I, int width, int height,
     int fromBitplane, int toBitplane, bool invert);

  static void erode
    (int N, VecInt& I, int width, int height,
     int fromBitplane, int workBitplane);

  static void dilate
    (int N, VecInt& I, int width, int height,
     int fromBitplane, int workBitplane);

private:

  ImgInt* _img;

public:

       ImgBitplane(int width, int height);
       ImgBitplane(ImgInt& src);
       ImgBitplane(Img& srcImg);
       ImgBitplane(ImgBitplane& srcImg);

      ~ImgBitplane();

  int  getWidth();
  int  getHeight();
  bool get(int x, int y, int mask);
  void set(int bitplane, bool white);
  void invert(int bitplane);
  void copy(ImgBitplane& srcImg);
  void copy(int fromBitplane, int toBitplane, bool invert);
  void erode(int N, int fromBitplane, int workBitplane);
  void dilate(int N, int fromBitplane, int workBitplane);
};

#endif // _ImgBitplane_hpp_
