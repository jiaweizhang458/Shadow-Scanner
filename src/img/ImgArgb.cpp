//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-04-30 13:24:36 taubin>
//------------------------------------------------------------------------
//
// ImgArgb.hpp
//
// Copyright (c) 2009-2014, Gabriel Taubin
// All rights reserved.

#include <iostream>
#include "ImgArgb.hpp"
#include <QImage>

int ImgArgb::_bgColor = 0xff555555;
int ImgArgb::_fgColor = 0xffffffff;

void ImgArgb::setBgColor(int bgColor) { _bgColor = bgColor; }
void ImgArgb::setFgColor(int fgColor) { _fgColor = fgColor; }

// int          _bitMask  = 0x000000;
// VecInt       _bitColor = null;
// ImgBitplane* _bitPlane = null;
// int          _width    = 0;
// int          _height   = 0;
// VecInt       _pixel    = null;

ImgArgb::~ImgArgb() {
}

ImgArgb::ImgArgb():
  _bitMask(0x000000),
  _bitColor(),
  _bitPlane((ImgBitplane*)0),
  _width(0),
  _height(0),
  _pixel() {
}

ImgArgb::ImgArgb(int w, int h, int bgColor):
  _bitMask(0x000000),
  _bitColor(),
  _bitPlane((ImgBitplane*)0),
  _width(0),
  _height(0),
  _pixel() {
  _init(w,h,bgColor);
}

ImgArgb::ImgArgb(Img& src, bool gray):
  _bitMask(0x000000),
  _bitColor(),
  _bitPlane((ImgBitplane*)0),
  _width(0),
  _height(0),
  _pixel() {
  int w = src.getWidth();
  int h = src.getHeight();
  _init(w,h);
  Img::set(src,gray);
}

ImgArgb::ImgArgb(int w, int h, VecInt& pixel) {
  _init(w,h);
  Img::set(pixel);
}

ImgArgb::ImgArgb(QImage& qImg):
  _bitMask(0x000000),
  _bitColor(),
  _bitPlane((ImgBitplane*)0),
  _width(0),
  _height(0),
  _pixel() {

  // std::cerr << "ImgArgb(QImage& qImg) {\n";

  // QImage::Format format = qImg.format();
  int            width  = qImg.width();
  int            height = qImg.height();

  // std::cerr << "  width    = " <<  width << "\n";
  // std::cerr << "  height   = " << height << "\n";
  //
  // switch(format) {
  // case QImage::Format_Invalid:
  //   std::cerr << "  format   = Invalid\n";
  //   break;
  // case QImage::Format_Mono:
  //   std::cerr << "  format   = Mono\n";
  //   break;
  // case QImage::Format_MonoLSB:
  //   std::cerr << "  format   = MonoLSB\n";
  //   break;
  // case QImage::Format_Indexed8:
  //   std::cerr << "  format   = Indexed8\n";
  //   break;
  // case QImage::Format_RGB32:
  //   std::cerr << "  format   = RGB32\n";
  //   break;
  // case QImage::Format_ARGB32:
  //   std::cerr << "  format   = ARGB32\n";
  //   break;
  // case QImage::Format_ARGB32_Premultiplied:
  //   std::cerr << "  format   = ARGB32_Premultiplied\n";
  //   break;
  // case QImage::Format_RGB16:
  //   std::cerr << "  format   = RGB16\n";
  //   break;
  // case QImage::Format_ARGB8565_Premultiplied:
  //   std::cerr << "  format   = ARGB8565_Premultiplied\n";
  //   break;
  // case QImage::Format_RGB666:
  //   std::cerr << "  format   = RGB666\n";
  //   break;
  // case QImage::Format_ARGB6666_Premultiplied:
  //   std::cerr << "  format   = ARGB6666_Premultiplied\n";
  //   break;
  // case QImage::Format_RGB555:
  //   std::cerr << "  format   = RGB555\n";
  //   break;
  // case QImage::Format_ARGB8555_Premultiplied:
  //   std::cerr << "  format   = ARGB8555_Premultiplied\n";
  //   break;
  // case QImage::Format_RGB888:
  //   std::cerr << "  format   = RGB888\n";
  //   break;
  // case QImage::Format_RGB444:
  //   std::cerr << "  format   = RGB444\n";
  //   break;
  // case QImage::Format_ARGB4444_Premultiplied:
  //   std::cerr << "  format   = ARGB4444_Premultiplied\n";
  //   break;
  // case QImage::Format_RGBX8888:
  //   std::cerr << "  format   = RGBX8888\n";
  //   break;
  // case QImage::Format_RGBA8888:
  //   std::cerr << "  format   = RGBA8888\n";
  //   break;
  // case QImage::Format_RGBA8888_Premultiplied:
  //   std::cerr << "  format   = RGBA8888_Premultiplied\n";
  //   break;
  // case QImage::Format_BGR30:
  //   std::cerr << "  format   = BGR30\n";
  //   break;
  // case QImage::Format_A2BGR30_Premultiplied:
  //   std::cerr << "  format   = A2BGR30_Premultiplied\n";
  //   break;
  // case QImage::Format_RGB30:
  //   std::cerr << "  format   = RGB30\n";
  //   break;
  // case QImage::Format_A2RGB30_Premultiplied:
  //   std::cerr << "  format   = A2RGB30_Premultiplied\n";
  //   break;
  // case QImage::Format_Alpha8:
  //   std::cerr << "  format   = Alpha8\n";
  //   break;
  // case QImage::Format_Grayscale8:
  //   std::cerr << "  format   = Grayscale8\n";
  //   break;
  // default:
  //   std::cerr << "  format   = ???\n";
  //   break;
  // }

  _init(width,height);

  int x,y,a,r,g,b,p;
  for(y=0;y<height;y++) {
    for(x=0;x<width;x++) {
      QRgb pix = qImg.pixel(x,y);

      // set(x,y,static_cast<int>(pix));

      b = pix&0xff; pix>>=8;
      g = pix&0xff; pix>>=8;
      r = pix&0xff; pix>>=8;
      a = pix&0xff;
      p = (a<<24)|(r<<16)|(g<<8)|(b<<0);

      set(x,y,p);
    }
  }

  // std::cerr << "}\n";
}

void ImgArgb::_init(int w, int h, int bgColor) {
  _pixel.clear();
  if(w>0 && h>0) {
    _width  = w;
    _height = h;
    int n = w*h;
    _pixel.reserve(n);
    _pixel.insert(0,n,bgColor);
  } else {
    _width  = 0;
    _height = 0;
  }
}

VecInt& ImgArgb::getPixel() {
  return _pixel;
}

void ImgArgb::setDimensions(int width, int height) {
  _init(width,height);
}

// virtual methods from Img
int ImgArgb::getWidth()  { return  _width; }
int ImgArgb::getHeight() { return _height; }
int ImgArgb::get(int x, int y) {
  int value = _bgColor;
  if(0<=x && x<_width && 0<=y && y<_height) {
    value = _pixel[x+y*_width];
  }
  return value;
}
void ImgArgb::set(int x, int y, int argb) {
  if(0<=x && x<_width && 0<=y && y<_height) {
    _pixel[x+y*_width] = argb;
  }
}

ImgBit* ImgArgb::getBitplane(int i) {
  ImgBit* imgBit = (ImgBit*)0;
  int mask = (0<=i && i<32)?(1<<i):0x00000000;
  if((_bitMask&mask)!=0x00000000) {
    imgBit = new ImgBit(_width,_height);
    int x,y;
    bool value;
    for(y=0;y<_height;y++) {
      for(x=0;x<_width;x++) {
        value = _bitPlane->get(x,y,mask);
        if(value) imgBit->set(x,y,true);
      }
    }
  }
  return imgBit;
}

int ImgArgb::newBitplane() {
  int i = -1;
  if(_bitPlane==(ImgBitplane*)0) {
    _bitPlane = new ImgBitplane(*this);
    _bitColor.clear();
    _bitColor.reserve(32);
    _bitColor.insert(0,32,0);
    _bitMask = 0x000000;
  }
  if(_bitMask!=0xffffffff) {
    for(i=0;i<32;i++)
      if(((1<<i)&_bitMask) == 0) { // i-th bit is available
        _bitMask |= ~(1<<i); // set i-th bit
        break;
      }
  }
  return i;
}

void ImgArgb::freeBitplane(int i) {
  if(0<=i && i<32) {
    _bitMask &= ~(1<<i); // clear i-th bit
    if(_bitMask==0x00000000) {
      _bitPlane = (ImgBitplane*)0;
      _bitColor.clear();
    }
  }
}

void ImgArgb::setBitColor(int i, int color) {
  if(_bitColor.size()==32 && 0<=i && i<32) _bitColor[i] = color;
}
int ImgArgb::getBitColor(int i) {
  return (_bitColor.size()==32 && 0<=i && i<32)?_bitColor[i]:0x00000000;
}

// TODO Wed Jul  2 18:05:55 2014
// public void setChannel(Img src, Channel channel) {
 
void ImgArgb::setChannel(Img& src, int channel) {
  if(1<=channel && channel<5 &&
     _width  == src.getWidth() && _height == src.getHeight()) {

    int i,x,y,p,q;

    int Mask1 = 0xff;
    switch(channel) {
    case 1 /* ALPHA */ : Mask1<<=24; break;
    case 2 /* RED   */ : Mask1<<=16; break;
    case 3 /* GREEN */ : Mask1<<= 8; break;
    case 4 /* BLUE  */ :             break;
    }
    int Mask0 = ~Mask1;

    for(i=x=0;x<_width;x++) {
      for(y=0;y<_height;y++,i++) {
        p = get(x,y);
        q = src.get(x,y);;
        _pixel[i] = (p&Mask0)|(q&Mask1);
      }
    }
  }
}

Img* ImgArgb::getIntensity() {
  Img* img    = new ImgArgb(*this); // copy src onto dst
  int width  = img->getWidth();
  int height = img->getHeight();
  int x,y,n;
  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      n = ARGB::getI(get(x,y));
      // ((Img*)img)->set(x,y,n,n,n);
      img->set(x,y,n,n,n);
    }
  }

  return img;
}


