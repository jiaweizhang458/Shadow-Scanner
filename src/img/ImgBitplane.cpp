//------------------------------------------------------------------------
//  Copyright (C) 2001-2005 Gabriel Taubin
//  Time-stamp: <2016-03-28 19:45:05 taubin>
//------------------------------------------------------------------------
//
// ImgBitplane.cpp
//
// Copyright (c) 2009-2016, Gabriel Taubin
// All rights reserved.

#include "ImgBitplane.hpp"

void ImgBitplane::set
(VecInt& I, int width, int height, int bitplane, bool white) {

  if(width<=0 || height<=0 ||
     I.size()!=(width*height) ||
     bitplane<0 || bitplane>31) return;

  if(width<=0) {
  } else if(height<=0) {
  } else if(I.size()!=(width*height)) {
  } else if(bitplane<0) {
  } else if(bitplane>31) {
  } else {

    int i,row,col,mask0,mask1;
      
    mask1 = 1<<bitplane;
    mask0 = ~mask1;
      
    // clear the workBitplane
    for(i=row=0;row<height;row++)
      for(col=0;col<width;col++,i++)
        if(white)
          I[i] |= mask1;
        else
          I[i] &= mask0;
  }
}

void ImgBitplane::invert
(VecInt& I, int width, int height, int bitplane) {

  if(width<=0 || height<=0 ||
     I.size()!=(width*height) ||
     bitplane<0 || bitplane>31) return;

  if(width<=0) {
  } else if(height<=0) {
  } else if(I.size()!=(width*height)) {
  } else if(bitplane<0) {
  } else if(bitplane>31) {
  } else {

    int i,row,col,mask0,mask1;
    bool bit;
      
    mask1 = 1<<bitplane;
    mask0 = ~mask1;
      
    // clear the workBitplane
    for(i=row=0;row<height;row++) {
      for(col=0;col<width;col++,i++) {
        bit = ((I[i]&mask1)==0);
        if(bit)
          I[i] |= mask1;
        else
          I[i] &= mask0;
      }
    }
  }
}

void ImgBitplane::copy
(VecInt& I, int width, int height, int fromBitplane,
 int toBitplane, bool invert) {

  if(width<=0) {
  } else if(height<=0) {
  } else if(I.size()!=(width*height)) {
  } else if(fromBitplane<0) {
  } else if(fromBitplane>31) {
  } else if(toBitplane<0) {
  } else if(toBitplane>31) {
  } else if(fromBitplane==toBitplane) {
  } else {

    int i,row,col,maskF0,maskF1,maskT0,maskT1;
    bool bit;
      
    maskF1 = 1<<fromBitplane;
    maskF0 = ~maskF1;
    maskT1 = 1<<toBitplane;
    maskT0 = ~maskT1;
      
    // _log(String.format("  maskF1 = 0x%08x",maskF1));
    // _log(String.format("  maskF0 = 0x%08x",maskF0));
    // _log(String.format("  maskT1 = 0x%08x",maskT1));
    // _log(String.format("  maskT0 = 0x%08x",maskT0));
      
    for(i=row=0;row<height;row++)
      for(col=0;col<width;col++,i++) {
        bit = ((I[i]&maskF1)!=0);
        if(invert) bit = !bit;
        if(bit)
          I[i] |= maskT1;
        else
          I[i] &= maskT0;
      }
  }
}

// TODO Wed Jul 10 19:54:41 2013
// not ideal implementation
void ImgBitplane::erode
(int N, VecInt& I, int width, int height,
 int fromBitplane, int workBitplane) {
  invert(I,width,height,fromBitplane);
  dilate(N,I,width,height,fromBitplane,workBitplane);
  invert(I,width,height,fromBitplane);
}

void ImgBitplane::dilate
(int N, VecInt& I, int width, int height,
 int fromBitplane, int workBitplane) {

  if(width<=0) {
  } else if(height<=0) {
  } else if(I.size()!=(width*height)) {
  } else if(fromBitplane<0) {
  } else if(fromBitplane>31) {
  } else if(workBitplane<0) {
  } else if(workBitplane>31) {
  } else if(fromBitplane==workBitplane) {
  } else {

    int row,col,i0,i1,maskF0,maskF1,maskW0,maskW1,step;

    maskF1 = (1<<fromBitplane);
    maskF0 = ~maskF1;
    maskW1 = (1<<workBitplane);
    maskW0 = ~maskF1;

    // clear the workBitplane
    // for(i=row=0;row<height;row++)
    //   for(col=0;col<width;col++,i++)
    //     I[i] &= maskW1;

    set(I,width,height,workBitplane,false);

    for(step=0;step<N;step++) {

      // dilate white pixels along rows
      for(row=0;row<height;row++) {
        for(col=0;col+1<width;col++) {
          i0 = (col  )+(row  )*width;
          i1 = (col+1)+(row  )*width;
          if((I[i0]&maskF1)!=0 || (I[i1]&maskF1)!=0) {
            I[i0] |= maskW1; I[i1] |= maskW1;
          }
        }
      }
      // dilate white pixels cols
      for(row=0;row+1<height;row++) {
        for(col=0;col<width;col++) {
          i0 = (col  )+(row  )*width;
          i1 = (col  )+(row+1)*width;
          if((I[i0]&maskF1)!=0 || (I[i1]&maskF1)!=0) {
            I[i0] |= maskW1; I[i1] |= maskW1;
          }
        }
      }

      // move workBitplane onto toBitplane and reset workBitplane
      // for(i=row=0;row<height;row++) {
      //   for(col=0;col<width;col++,i++) {
      //     // get bit maskW1
      //     if((I[i]&maskW1)!=0) I[i]|=maskF1; 
      //     // clear bit maskW1
      //     I[i]&=maskW0;
      //   }
      // }

      copy(I,width,height,workBitplane,fromBitplane,false);
      set(I,width,height,workBitplane,false);

    }
  }
}

ImgBitplane::ImgBitplane(int width, int height):_img((ImgInt*)0) {
  if(width>0 && height>0)
    _img = new ImgInt(width,height);
}

ImgBitplane::ImgBitplane(ImgInt& src):_img((ImgInt*)0) {
  int width  = src.getWidth();
  int height = src.getHeight();
  if(width>0 && height>0)
    _img = new ImgInt(width,height);
}

ImgBitplane::ImgBitplane(Img& srcImg):_img((ImgInt*)0) {
  int width  = 0;
  int height = 0;
  if((width=srcImg.getWidth())>0 &&
     (height=srcImg.getHeight())>0) {
    _img = new ImgInt(width,height);
  }
}

ImgBitplane::ImgBitplane(ImgBitplane& srcImg):_img((ImgInt*)0) {
  if((srcImg._img)!=(ImgInt*)0)
    _img = new ImgInt(*(srcImg._img));
}

ImgBitplane::~ImgBitplane() {
  // TODO ???
  // delete _img;
}

int ImgBitplane::getWidth()  {
  return (_img==(ImgInt*)0)?0:_img->getWidth();
}

int ImgBitplane::getHeight() {
  return (_img==(ImgInt*)0)?0:_img->getHeight();
}

bool ImgBitplane::get(int x, int y, int mask) {
  bool value = false;
  if(_img!=(ImgInt*)0 &&
     0<=x && x<_img->getWidth() &&
     0<=y && y<_img->getHeight()) {
    value = ((_img->get(x,y) & mask)!=0);
  }
  return value;
}
  
void ImgBitplane::set(int bitplane, bool white) {
  set(_img->getPixel(),_img->getWidth(),_img->getHeight(),bitplane,white);
}

void ImgBitplane::invert(int bitplane) {
  invert(_img->getPixel(),_img->getWidth(),_img->getHeight(),bitplane);
}
  
void ImgBitplane::copy(ImgBitplane& srcImg) {
  if(_img!=(ImgInt*)0 && srcImg._img!=(ImgInt*)0)
    _img->copy(*(srcImg._img));
}

void ImgBitplane::copy(int fromBitplane, int toBitplane, bool invert) {
  copy(_img->getPixel(),_img->getWidth(),_img->getHeight(),
       fromBitplane,toBitplane,invert);
}

void ImgBitplane::erode
(int N, int fromBitplane, int workBitplane) {
  erode(N,_img->getPixel(),_img->getWidth(),_img->getHeight(),
        fromBitplane,workBitplane);
}

void ImgBitplane::dilate
(int N, int fromBitplane, int workBitplane) {
  dilate(N,_img->getPixel(),_img->getWidth(),_img->getHeight(),
         fromBitplane,workBitplane);
}
