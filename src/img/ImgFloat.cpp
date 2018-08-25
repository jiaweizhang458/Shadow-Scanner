//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-29 14:26:25 taubin>
//------------------------------------------------------------------------
//
// ImgFloat.java
//
// Copyright (c) 2009-2012, Gabriel Taubin
// All rights reserved.

#include "ImgFloat.hpp"

ImgFloat::ImgFloat():_width(0),_height(0),_pixel() {
}

ImgFloat::ImgFloat(int w, int h, float value):_width(0),_height(0),_pixel() {
  if(w>0 && h>0) {
    _width  = w;
    _height = h;
    int n = _width*_height;
    _pixel.reserve(n);
    _pixel.insert(0,n,value);
  }
}

ImgFloat::ImgFloat(ImgFloat& src):_width(0),_height(0),_pixel() {
  _width  = src._width;
  _height = src._height;
  int n = _width*_height;
  _pixel.reserve(n);
  _pixel.insert(0,n,0.0f);
  copy(src);
}

ImgFloat::ImgFloat(Img& img, int channel):_width(0),_height(0),_pixel() {
  setDimensions(img.getWidth(),img.getHeight());
  setChannel(img,channel);
}

void ImgFloat::setDimensions(int w, int h) {
  if(w>0 && h>0 && (w!=_width || h!=_height) ) {
    _width  = w;
    _height = h;
    int n = _width*_height;
    _pixel.reserve(n);
    _pixel.insert(0,n,0.0f);
    set(0.0f);
  }
}

void ImgFloat::copy(ImgFloat& src) {
  if(_width==src._width && _height==src._height) {
    int i,x,y;
    for(i=y=0;y<_height;y++) {
      for(x=0;x<_width;x++,i++) {
        _pixel[i] = src._pixel[i];
      }
    }
  }
}

void ImgFloat::getChannel(Img& dst, int channel) {
  if(1<=channel && channel<5 &&
     dst.getWidth() ==_width && dst.getHeight()==_height) {

    int i,x,y,p,a,r,g,b,v;

    // int[] dstPix = dst.getPixel();
      
    for(i=y=0;y<_height;y++) {
      for(x=0;x<_width;x++,i++) {
          
        // get pixel value from array
        // p = dstPix[i];
        p = dst.get(x,y);
        // extract a components
        b  = p&0xff; p>>=8;
        g  = p&0xff; p>>=8;
        r  = p&0xff; p>>=8;
        a  = p&0xff;
          
        // v = (int)(255.0f*_pixel[i]);
        v = (int)(255.0f*get(x,y));
        if(v<0) v=0; else if(v>255) v=255;
          
        // replace selected channel
        switch(channel) {
        case 1  /* "ALPHA"     */ :
          a = v; // 0xff&((int)(255.0f*_pixel[i]));
          break;
        case 2  /* "RED"       */ :
          r = v; // 0xff&((int)(255.0f*_pixel[i]));
          break;
        case 3  /* "GREEN"     */ :
          g = v; // 0xff&((int)(255.0f*_pixel[i]));
          break;
        case 4  /* "BLUE"      */ :
          b = v; // 0xff&((int)(255.0f*_pixel[i]));
          break;
        }
          
        // reconstruct pixel value
        p = a; p<<=8; p|= r; p<<=8; p|= g; p<<=8; p|= b;
          
        // set new pixel value
        // dstPix[i] = p;
        dst.set(x,y,p);
      }
    }
  }
}

void ImgFloat::setChannel(Img& src, int channel) {
  if(0<=channel && channel<5 &&
     src.getWidth() ==_width && src.getHeight()==_height) {

    int i,x,y,p,a,r,g,b;

    // int[] srcPix = src.getPixel();

    for(i=y=0;y<_height;y++) {
      for(x=0;x<_width;x++,i++) {

        // get pixel value from array
        // p = srcPix[i];
        p = src.get(x,y);
        // extract the components
        b  = p&0xff; p>>=8;
        g  = p&0xff; p>>=8;
        r  = p&0xff; p>>=8;
        a  = p&0xff;

        switch(channel) {
        case 0  /* "INTENSITY" */ :
          _pixel[i] =
            (0.299f*((float)r)+0.587f*((float)g)+0.114f*((float)b))/255.0f;
          break;
        case 1  /* "ALPHA"     */ :
          _pixel[i] = ((float)a)/255.0f;
          break;
        case 2  /* "RED"       */ :
          _pixel[i] = ((float)r)/255.0f;
          break;
        case 3  /* "GREEN"     */ :
          _pixel[i] = ((float)g)/255.0f;
          break;
        case 4  /* "BLUE"      */ :
          _pixel[i] = ((float)b)/255.0f;
          break;
        }
      }
    }
  }
}

int  ImgFloat::getHeight() { return _height; }
int  ImgFloat::getWidth()  { return _width; }

VecFloat& ImgFloat::getPixel()   { return _pixel;   }

float ImgFloat::get(int x, int y) {
  return (x>=0 && x<_width && y>=0 && y<_height)?_pixel[x+y*_width]:0.0f;
}

void ImgFloat::set(float value) {
  for(int i=0;i<_pixel.size();i++)
    _pixel[i] = value;
}

void ImgFloat::clear() {
  set(0.0f);
}

void ImgFloat::add(float value) {
  for(int i=0;i<_pixel.size();i++)
    _pixel[i] += value;
}

void ImgFloat::multiply(float value) {
  for(int i=0;i<_pixel.size();i++)
    _pixel[i] *= value;
}

void ImgFloat::_set(int x, int y, float value) {
  _pixel[x+y*_width]   = value;
}
  
void ImgFloat::set(int x, int y, float value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    _set(x,y,value);
  }
}

void ImgFloat::_add(int x, int y, float value) {
  _pixel[x+y*_width]   += value;
}

void ImgFloat::add(int x, int y, float value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    _add(x,y,value);
  }
}

void ImgFloat::_multiply(int x, int y, float value) {
  _pixel[x+y*_width]   *= value;
}

void ImgFloat::multiply(int x, int y, float value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    _multiply(x,y,value);
  }
}

void ImgFloat::setMax(int x, int y, float value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    if(value>_pixel[x+y*_width]) _set(x,y,value);
  }
}

void ImgFloat::setMin(int x, int y, float value) {
  if(x>=0 && x<_width && y>=0 && y<_height) {
    if(value<_pixel[x+y*_width]) _set(x,y,value);
  }
}
