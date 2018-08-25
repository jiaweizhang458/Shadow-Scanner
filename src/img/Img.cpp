//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 13:30:45 taubin>
//------------------------------------------------------------------------

#include "Img.hpp"

void Img::set(int x, int y, int a, int r, int g, int b) {
  set(x,y,ARGB::argb(a,r,g,b));
}

void Img::set(int x, int y, int r, int g, int b) {
  set(x,y,ARGB::argb(r,g,b));
}

void Img::setA(int x, int y, int  a) { set(x,y,ARGB::setA(get(x,y), a)); }
void Img::setR(int x, int y, int  r) { set(x,y,ARGB::setR(get(x,y), r)); }
void Img::setG(int x, int y, int  g) { set(x,y,ARGB::setG(get(x,y), g)); }
void Img::setB(int x, int y, int  b) { set(x,y,ARGB::setB(get(x,y), b)); }
void Img::setI(int x, int y, int gr) { set(x,y,ARGB::setI(get(x,y),gr)); }

void Img::set(int argb) {
  int width  = getWidth();
  int height = getHeight();
  for(int x=0;x<width;x++)
    for(int y=0; y<height; y++)
      set(x,y,argb);
}

void Img::set(Img& src, bool convertToGray) {
  // if(src==null) return;
  int width  = getWidth();
  if(src.getWidth() != width) return;
  int height = getHeight();
  if(src.getHeight() != height) return;
  int argb,x,y;
  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      argb = src.get(x,y);
        if(convertToGray) argb = ARGB::setI(argb);
      set(x,y,argb);
    }
  }
}

void Img::setGray(Img& src) {
  set(src,true);
}

void Img::set(Img& src) {
  set(src,false);
}

void Img::set(VecInt& pixel, bool convertToGray) {
  // if(pixel==null) return;
  int width  = getWidth();
  int height = getHeight();
  if(pixel.size() != width*height) return;
  int argb,x,y;
  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      argb = pixel[x+y*width];
        if(convertToGray) argb = ARGB::setI(argb);
      set(x,y,argb);
    }
  }
}

void Img::setGray(VecInt& pixel) {
  set(pixel,true);
}

void Img::set(VecInt& pixel) {
  set(pixel,false);
}

