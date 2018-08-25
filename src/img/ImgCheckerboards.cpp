// Software developed for the Spring 2016 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO NOR GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <QtAlgorithms>
#include "ImgCheckerboards.hpp"
#include "ImgBuckets.hpp"
#include "ImgArgb.hpp"
#include "ImgDraw.hpp"
#include <util/Vec.hpp>
#include <util/GraphFaces.hpp>
#include <util/GraphTraversal.hpp>
#include <util/CircleNeighborhood.hpp>
#include <util/PartitionLists.hpp>
#include <util/Heap.hpp>
#include <vector>

// private static
int ImgCheckerboards::RED   = 7;
int ImgCheckerboards::WHITE = 3;
int ImgCheckerboards::BLACK = 4;
int ImgCheckerboards::BLUE  = 6;

float ImgCheckerboards::_wMinSobel = 0.777f;
float ImgCheckerboards::_wMaxSobel = 0.888f;

float ImgCheckerboards::_minScale =   0.0f; //  16.0f;
float ImgCheckerboards::_maxScale = 255.0f; // 192.0f;

float ImgCheckerboards::_randomColorBase  =  30.00f; // 56.0f;
float ImgCheckerboards::_randomColorScale = 220.00f;

float ImgCheckerboards::_random() {
  return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
}
int ImgCheckerboards::_makeRandomColor() {
  int r = (int)(_randomColorBase+_randomColorScale*_random());
  int g = (int)(_randomColorBase+_randomColorScale*_random());
  int b = (int)(_randomColorBase+_randomColorScale*_random());
  return 0xff000000|(r<<16)|(g<<8)|b;
}

//////////////////////////////////////////////////////////////////////
//  public static
ImgArgb* ImgCheckerboards::boxFilter(Img& srcImg, int radius) {

  ImgArgb* img = new ImgArgb(srcImg); // make a copy of srcImg
    
  if(radius<=0) radius = 1;
  
  int width   = srcImg.getWidth();
  int height  = srcImg.getHeight();
  int nPixels = width*height;

  // GT Tue Mar 29 14:35:42 2016
  // makes the application crash img->get(x,y)
  // int A[nPixels];
  // int R[nPixels];
  // int G[nPixels];
  // int B[nPixels];

  VecInt A(nPixels,0);
  VecInt R(nPixels,0);
  VecInt G(nPixels,0);
  VecInt B(nPixels,0);

  int i,i00,i01,i10,i11,row,col,row0,row1,col0,col1;
  int a,r,g,b,argb,Rp,Gp,Bp,nPixBox;

  // compute integral images

  // first pass : integrate along rows
  for(i=row=0;row<height;row++) {
    for(Rp=Gp=Bp=col=0;col<width;col++,i++) {
      a = img->get(col,row);
      b = a&0xff; a>>=8;
      g = a&0xff; a>>=8;
      r = a&0xff; a>>=8;
      a &= 0xff;
      A[i] = a;
      R[i] = Rp+r; Rp = R[i];
      G[i] = Gp+g; Gp = G[i];
      B[i] = Bp+b; Bp = B[i];
    }
  }
  // second pass : integrate along cols
  for(col=0;col<width;col++) {
    for(i=col,Rp=Gp=Bp=row=0;row<height;row++,i+=width) {
      // i = col+row*width;
      R[i] += Rp; Rp = R[i];
      G[i] += Gp; Gp = G[i];
      B[i] += Bp; Bp = B[i];
    }
  }

  // apply box filter
  for(i=row=0;row<height;row++) {
    if((row0=row-radius-1)<      -1) row0 =       -1;
    if((row1=row+radius  )>height-1) row1 = height-1;

    for(Rp=Gp=Bp=col=0;col<width;col++,i++) {
      if((col0=col-radius-1)<      -1) col0 =       -1;
      if((col1=col+radius  )> width-1) col1 =  width-1;
      nPixBox = (row1-row0)*(col1-col0);
            
      // Ibox[i] = (I[i11]-I[i10]-I[i01]+I[i00])/nPixBox;

      // i = col+row*width;
      i00 = col0+row0*width; i01 = col1+row0*width;
      i10 = col0+row1*width; i11 = col1+row1*width;

      a = _makeRandomColor();
      b = a&0xff; a>>=8;
      g = a&0xff; a>>=8;
      r = a&0xff; a>>=8;
      a &= 0xff;

      // r = g = b = 0;
      {
        // i11 = col1+row1*width;
        r = R[i11]; g = G[i11]; b = B[i11];
      }
      if(row0>=0) {
        // i01 = col1+row0*width;
        r -= R[i01]; g -= G[i01]; b -= B[i01];
      }
      if(col0>=0) {
        // i10 = col0+row1*width;
        r -= R[i10]; g -= G[i10]; b -= B[i10];
      }
      if(row0>=0 && col0>=0) {
        // i00 = col0+row0*width;
        r += R[i00]; g += G[i00]; b += B[i00];
      }

      a = A[i]; // transfer alpha channel from input image
      r /= nPixBox; g /= nPixBox; b /= nPixBox;
      argb = (a<<24)|(r<<16)|(g<<8)|(b<<0);
      img->set(col,row,argb);
    }
  }

  return img;
}

//////////////////////////////////////////////////////////////////////
// public static 
int ImgCheckerboards::boxFilterThreshold
(Img& srcImg, int nMorphology, PartitionLists& P) {

  int iUnclassified = -1;

  int   row,col,aI,rI,gI,bI,aB,rB,gB,bB,grayI,grayB,i,i0,i1,j;
  int   nParts,iPart,P_size_i;
  float f;

  int   width        = srcImg.getWidth();
  int   height       = srcImg.getHeight();
  int   nPixels      = width*height;
  int   radius       = (width+height)/10; // for the box filter
  // int   nMorphology  = (radius/50); if(nMorphology<1) nMorphology = 1;
  // int   nMorphology  = (radius/40); if(nMorphology<1) nMorphology = 1;
  int   nMinCompSize = (2+nMorphology)*(2+nMorphology);
  VecInt I(width*height,0);

  _log("  nPixels      = ",nPixels);
  _log("  radius       = ",radius);
  _log("  nMorphology  = ",nMorphology);
  _log("  nMinCompSize = ",nMinCompSize);

  _log("  computing boxfiltered image ...");

  ImgArgb* boxFilteredImg = boxFilter(srcImg,radius);

  _log("  computing initial binary segmentation ...");

  // initial binary segmentation
  for(f=0.0f,i=row=0;row<height;row++) {
    for(col=0;col<width;col++,i++) {

      // get and decompose input pixel
      aI  = srcImg.get(col,row);
      bI  = aI&0xff; aI>>=8;
      gI  = aI&0xff; aI>>=8;
      rI  = aI&0xff; aI>>=8;
      aI &= 0xff;
      f = 0.299f*((float)rI)+0.587f*((float)gI)+0.114f*((float)bI);
      if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
      grayI = (int)floor(f);

      // get and decompose box filtered image pixel
      aB  = boxFilteredImg->get(col,row);
      bB  = aB&0xff; aB>>=8;
      gB  = aB&0xff; aB>>=8;
      rB  = aB&0xff; aB>>=8;
      aB &= 0xff;
      f = 0.299f*((float)rB)+0.587f*((float)gB)+0.114f*((float)bB);
      if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
      grayB = (int)floor(f);

      // binary segmentation based with respect to the gray box
      // filter value; store in bitplane 0x1
          
      I[i] = (grayI<=grayB)?0x00:0x01;
    }
  }
    
  _log(QString("  performing %1 symmetric dilation steps ...").arg(nMorphology));

  // copy white pixels onto bitplane 0x2 and black onto bitplane
  // 0x4 use bitplane 0x8 as tempoary storage
  ImgBitplane::copy(I,width,height,0,1,false);
  ImgBitplane::copy(I,width,height,0,2,true);
      
  // dilation steps
  ImgBitplane::dilate(nMorphology,I,width,height,1,3);
  ImgBitplane::dilate(nMorphology,I,width,height,2,3);

  _log("  creating initial partition ...");

  // create initial partition 
  P.reset(nPixels);
  for(row=0;row<height;row++) {
    for(col=0;col+1<width;col++) {
      i0 = (col  )+(row  )*width;
      i1 = (col+1)+(row  )*width;
      if(I[i0]==I[i1] && (I[i0]==WHITE||I[i0]==BLACK)) P.join(i0,i1);
    }
  }
  for(row=0;row+1<height;row++) {
    for(col=0;col<width;col++) {
      i0 = (col  )+(row  )*width;
      i1 = (col  )+(row+1)*width;
      if(I[i0]==I[i1] && (I[i0]==WHITE||I[i0]==BLACK)) P.join(i0,i1);
    }
  }
  nParts  = P.getNumberOfParts();
  _log("  nParts = ",nParts);

  int P_size_max = 0;
  int nSizeNonZero = 0;
  for(i=0;i<nPixels;i++) {
      P_size_i = P.size(i);
      if(P_size_i>0) nSizeNonZero++;
      if(P_size_i>P_size_max) P_size_max = P_size_i;
  }
  _log("  nSizeNonZero = ",nSizeNonZero);
  _log("  P_size_max = ",P_size_max);
    
  _log("  classifying unclassified pixels ...");

  // find one unclassified pixel
  // one should be found unless the image is constant
  iUnclassified = -1;
  for(i=0;i<nPixels;i++)
    if(I[i]==RED || I[i]==BLUE) { iUnclassified=i; break; }

  if(iUnclassified>=0) {
      
    _log("  P.size(iUnclassified) = ",P.size(iUnclassified));

    _log("  unclassifying small components ...");

    // make all small compunents unclassified
    for(i=0;i<nPixels;i++) {
      if(0<(P_size_i=P.size(i)) && P_size_i<nMinCompSize) {
        P.join(i,iUnclassified);
      }
    }
    nParts  = P.getNumberOfParts();
    _log("    nParts = ",nParts);

    // remember the root of the component tree
    iUnclassified = P.find(iUnclassified);

    // end of segment12

    if(false) {
      
      _log("    making array of pixel labels and component root pixels ...");

      // make array of pixel labels and component root pixels
      nParts  = P.getNumberOfParts();
      _log("    nParts = ",nParts);
      //int part[nPixels];
      //int root[nParts];
	  std::vector<int> part(nPixels);
	  std::vector<int> root(nParts);
	  // assign part # 0 to the unclassified pixels
      part[iUnclassified] = 0;
      root[0] = iUnclassified;
      for(iPart=1,i=0;i<nPixels;i++)
        if(i==P.find(i) && i!=iUnclassified) {
          root[iPart] = i; part[i] = iPart++;
        }
      for(i=0;i<nPixels;i++)
        if(i!=P.find(i))
          part[i] = part[P.find(i)];
        
      _log("    resetting the image buffer ...");

      // erase all bitplanes and set bitplane 0x0 to the classified
      I.fill(0x1);
      for(i=P.beg(iUnclassified);i>=0;i=P.next(i)) I[i] = 0x0;

      _log("    splitting the component of unclassified pixels ...");

      // split the unclassified pixels
      P.split(iUnclassified); iUnclassified = -1;
    
      // allocate input and output stacks
      VecInt wavefrontIn;
      VecInt wavefrontOut;
      // use bitplane 0x2 to track visited pixels
      for(i=0;i<nPixels;i++)
        if(part[i]!=0)
          I[i] |= 0x2;

      _log("    identifying first wavefront ...");

      // find first wave
      for(row=0;row<height;row++) {
        for(col=0;col+1<width;col++) {
          i0 = (col  )+(row  )*width;
          i1 = (col+1)+(row  )*width;
          if(part[i0]==0 && part[i1]!=0) {
              wavefrontIn.append(i0);
              wavefrontIn.append(i1);
            I[i0] |= 0x2;
          } else if(part[i1]==0 && part[i0]!=0) {
              wavefrontIn.append(i1);
              wavefrontIn.append(i0);
            I[i1] |= 0x2;
          }
        }
      }
      // dilate white 0x2 -> 0x8 along cols
      for(row=0;row+1<height;row++) {
        for(col=0;col<width;col++) {
          i0 = (col  )+(row  )*width;
          i1 = (col  )+(row+1)*width;
          if(part[i0]==0 && part[i1]!=0) {
              wavefrontIn.append(i0);
              wavefrontIn.append(i1);
            I[i0] |= 0x2;
          } else if(part[i1]==0 && part[i0]!=0) {
              wavefrontIn.append(i1);
              wavefrontIn.append(i0);
            I[i1] |= 0x2;
          }
        }
      }
    
      _log("    propagating wavefront ...");

      while(wavefrontIn.size()>0) {
        _log("      size = ",wavefrontIn.size());
        while(wavefrontIn.size()>0) {
            
            i1    = wavefrontIn.last(); // classified
            wavefrontIn.removeLast();
            i0    = wavefrontIn.last(); // unclassified
            wavefrontIn.removeLast(); // unclassified
          col   = i0%width;
          row   = i0/width;

          // join i0 to i1 only if i0 still is unclassified
          if(part[i0]!=0) continue;

          part[i0] = part[i1];
          P.join(i0,i1);

          // assign current wavefront pixels and identify next wavefront pixels
          if(col>0) {
            j = (col-1)+(row  )*width;
            if((I[j]&0x2)==0 && part[j]==0) {
                wavefrontOut.append(j);
                wavefrontOut.append(i0);
              I[j] |= 0x2;
            }
          }
          if(col+1<width) {
            j = (col+1)+(row  )*width;
            if((I[j]&0x2)==0 && part[j]==0) {
                wavefrontOut.append(j);
                wavefrontOut.append(i0);
              I[j] |= 0x2;
            }
          }
          if(row>0) {
            j = (col  )+(row-1)*width;
            if((I[j]&0x2)==0 && part[j]==0) {
                wavefrontOut.append(j);
                wavefrontOut.append(i0);
              I[j] |= 0x2;
            }
          }
          if(row+1<height) {
            j = (col  )+(row+1)*width;
            if((I[j]&0x2)==0 && part[j]==0) {
                wavefrontOut.append(j);
                wavefrontOut.append(i0);
              I[j] |= 0x2;
            }
          }
        }
        // next wavefront
        wavefrontIn.swap(wavefrontOut);
      }
      iUnclassified = -1;
    }
  }

  return iUnclassified;
}

//////////////////////////////////////////////////////////////////////
// public static

ImgArgb* ImgCheckerboards::makeSegmentedImg
(PartitionLists& P, int iBlack, Img& img) {
  ImgArgb* segmented = (ImgArgb*)0;

  // _log("makeSegmentedImg() {");

  int width  = img.getWidth();
  int height = img.getHeight();
  int nPixels = width*height;
  if(P.getNumberOfElements()==nPixels) {

    int h,i,j,x,y,argb;

    // label pixels by part
    // int   nElements = P.getNumberOfElements();
    int   nParts    = P.getNumberOfParts();

    // _log("   nPixels   = "+nPixels);
    // _log("   nElements = "+nPixels);
    // _log("   nParts    = "+nParts);
    // _log("   iBlack    = "+iBlack);
    // _log("   making color table ...");

    // make color table
    VecInt color(nParts,0);
    for(i=0;i<nParts;i++)
      color[i] = _makeRandomColor();

    // create segmented image and paint pixels
    // _log("   creating segmented image ...");
    // segmented = new ImgInt(img);
    segmented = new ImgArgb(img.getWidth(),img.getHeight());

    // _log("   painting pixels ...");
    for(h=i=0;i<nPixels;i++) {
      if((j=P.beg(i))>=0) {
        argb =
          (P.find(i)==iBlack)?0xff000000:color[h];
        do {
          y = j/width;
          x = j%width;
          segmented->set(x,y,argb);
        } while((j=P.next(j))>=0);
        h++;
      }
    }
    // _log("}");
  }

  return segmented;
}

//////////////////////////////////////////////////////////////////////

ImgArgb* ImgCheckerboards::segment1
(Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow) {
  ImgArgb* segmented = (ImgArgb*)0;
  PartitionLists P(1);
  segment1(srcImg, maxEdgeWeight, nSegments, P);
  segmented = makeSegmentedImg(P,-1,srcImg);
  return segmented;
}

void ImgCheckerboards::segment1
(Img& srcImg, float maxEdgeWeight, int nSegments, PartitionLists& P) {

  _log("segment1() {");
  _log("  maxEdgeWeight = ",maxEdgeWeight);
  _log("  nMinSegments  = ",nSegments);

  // try {
  int width  = srcImg.getWidth();
  int height = srcImg.getHeight();
  // _log("  width         = ",width);
  // _log("  height        = ",height);
  // _log("  maxEdgeWeight = ",maxEdgeWeight);
  // _log("  nMinSegments  = ",nSegments);
  int nPixels = width*height;
  P.reset(nPixels);

  // apply the sobel operator and get result as an array of floats
  // in the [0:1] range

  // ImgFloat* ImgCheckerboards::_sobelFloatEdgesI(Img& srcImg);

  ImgFloat* pixelWeight = sobelFloatEdgesI(srcImg);
  // pixelWeigh.get(x,y) close to 1 --- non-edge pixel
  // pixelWeigh.get(x,y) close to 0 --- edge pixel
  // these weights re then conevrted to complementary weights
  // w = 1.0-pixelWeight(x,y)
  // so that
  // w ~ 0 is non-edge
  // w > maxEdgeWeight is edge

  int  nHedges = height*(width-1);
  int  nVedges = width*(height-1);
  // int  nEdges  = nHedges+nVedges;

  float  w,w0,w1,wMax;
  int    i,h,hMax,y,x,iEdge,k,iPix0,iPix1;
  QString s;

  // TODO Tue Jun 25 12:08:30 2013
  // this is not working very well; we need a better method to
  // automatically select the threshold
  // _log("  adjusting edge weights ...");

  wMax = 0.0f;
  for(i=y=0;y<height;y++)
    for(x=0;x<width;x++,i++)
      if((w = 1.0f-pixelWeight->get(x,y))>wMax) 
        wMax = w;
  // _log("   wMax = "+wMax);
  if(wMax<1.0f) 
    for(i=y=0;y<height;y++)
      for(x=0;x<width;x++,i++)
        if((w = 1.0f-pixelWeight->get(x,y))>0.0f) {
          w /= wMax;
          pixelWeight->set(x,y,1.0f-w);
        }

  // _log("  creating pixel partition ...");

  w = 0.0f;
  // build weights histogram, just for debugging purposes
  VecInt hist(16,0);
  for(i=y=0;y<height;y++) {
    for(x=0;x<width;x++,i++) {
      if((w = 1.0f-pixelWeight->get(x,y))<0.0f)
        w = 0.0f;
      else if(w>1.0f)
        w = 1.0f;
      h = (int)(15.0f*w);
      hist[h]++;
    }
  }
  for(hMax=h=0;h<16;h++) {
    if(hist[h]>hMax) hMax = hist[h];        
    w = (100.0f*(float)hist[h])/((float)nPixels);
    // n = (int)floor(0.6f*w);
    // s = ""; while(n-->=0) s = s+"*";
    // _log(String.format("%2d | %6.2f%c | %s",h,w,'%',s));
  }

  Heap H;

  // inserting horizontal edges ...
  for(iEdge=y=0;y<height;y++)
    for(x=0;x<width-1;x++,iEdge++) { // edge (y,x  )->(y,x+1)
      w0 = 1.0f-pixelWeight->get(x  ,y);
      w1 = 1.0f-pixelWeight->get(x+1,y);
        w  = (w0>w1)?w0:w1;
      k  = H.add(w,iEdge); // assert(k==iEdge);
    }
  // inserting vertical edges ...
  for(y=0;y<height-1;y++)
    for(x=0;x<width;x++,iEdge++) { // edge (y  ,x)->(y+1,x)
      w0 = 1.0f-pixelWeight->get(x,y  );
      w1 = 1.0f-pixelWeight->get(x,y+1);
        w  = (w0>w1)?w0:w1;
      k  = H.add(w,iEdge); // assert(k==iEdge);
    }

  // _log("  joining edges ...");
  // _log("  heap length = "+H.length());

  while(H.length()>0 && P.getNumberOfParts()>nSegments) {
    k     = H.delMin();
    w     = H.getLastFKey();
    iEdge = H.getLastIKey();
    if(w >= maxEdgeWeight) { break; }
    if(iEdge<nHedges) { // horizontal edge
      x     = iEdge%(width-1); y     = iEdge/(width-1);
      iPix0 =   y*width+(x  ); iPix1 =   y*width+(x+1);
      if(P.find(iPix0)!=P.find(iPix1)) {
        P.join(iPix0,iPix1);
      }
    } else if((iEdge-=nHedges)<nVedges) { // vertical edge
      x     =   iEdge%width; y     =   iEdge/width;
      iPix0 = (y  )*width+x; iPix1 = (y+1)*width+x;
      if(P.find(iPix0)!=P.find(iPix1)) {
        P.join(iPix0,iPix1);
      }
    }
  }

  // _log("  nParts = "+P.getNumberOfParts());

  // } catch(Exception e) {
  //   _log("  Exception catched : "+e);
  // }

  _log("} segment1");
}

//////////////////////////////////////////////////////////////////////

// public static
ImgArgb* ImgCheckerboards::segment2
(Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow) {
  ImgArgb* segmented = (ImgArgb*)0;

  PartitionLists P(1);
  segment2(srcImg, maxEdgeWeight, nSegments, P);
  segmented = makeSegmentedImg(P,-1,srcImg);
  return segmented;
}

// public static
void ImgCheckerboards::segment2
(Img& srcImg, float maxEdgeWeight, int nSegments, PartitionLists& P) {
  _log("segment2() {");

  segment1(srcImg,maxEdgeWeight,nSegments,P);

  int width   = srcImg.getWidth();
  int height  = srcImg.getHeight();
  int nPixels = width*height;
  int nParts  = P.getNumberOfParts();

  int times,nPartsPrev,i,x0,x1,x2,y0,y1,y2;
  int i00,i01,i02,i10,i11,i12,i20,i21,i22,iMax,nMax;

  // sort of smart dilation
  _log("  max filter ...");

  VecInt I(nPixels,-1);
  VecInt nI(nPixels,0);

  for(times=0;times<10;times++) { // at most 10 iterations
    nPartsPrev = nParts;
    for(i=y1=0;y1<height;y1++) {
      y0 = (y1>       0)?y1-1:y1;
      y2 = (y1<height-1)?y1+1:y1;
      for(x1=0;x1<width;x1++,i++) {
        i11 = P.find(x1+width*y1);
        if(P.size(i11)==1) {
          x0 = (x1>      0)?x1-1:x1;
          x2 = (x1<width-1)?x1+1:x1;
          // find component IDs of 3x3 neighborhood
          i00 = P.find(x0+width*y0);
          i01 = P.find(x0+width*y1);
          i02 = P.find(x0+width*y2);
          i10 = P.find(x1+width*y0);
          i12 = P.find(x1+width*y2);
          i20 = P.find(x2+width*y0);
          i21 = P.find(x2+width*y1);
          i22 = P.find(x2+width*y2);
          // determine index with 
          iMax = -1; nMax = 0;
          if(++nI[i00]>nMax) { iMax = i00; nMax = nI[i00]; }
          if(++nI[i01]>nMax) { iMax = i01; nMax = nI[i01]; }
          if(++nI[i02]>nMax) { iMax = i02; nMax = nI[i02]; }
          if(++nI[i10]>nMax) { iMax = i10; nMax = nI[i10]; }
          if(++nI[i11]>nMax) { iMax = i11; nMax = nI[i11]; }
          if(++nI[i12]>nMax) { iMax = i12; nMax = nI[i12]; }
          if(++nI[i20]>nMax) { iMax = i20; nMax = nI[i20]; }
          if(++nI[i21]>nMax) { iMax = i21; nMax = nI[i21]; }
          if(++nI[i22]>nMax) { iMax = i22; nMax = nI[i22]; }
          // assign to the component with most memebers in the
          // neighborhood, but only if such a component has at
          // least two members in the neighborhood
          I[i] = (nMax>1)?iMax:i11;
          // reset counters
          nI[i00] = 0; nI[i01] = 0; nI[i02] = 0;
          nI[i10] = 0; nI[i11] = 0; nI[i12] = 0;
          nI[i20] = 0; nI[i21] = 0; nI[i22] = 0;
        } else { // pixel belongs to a component of size > 1
          I[i] = i11; // preserve the original component assignment
        }
      }
    }

    // reset and rebuild the partition
    P.reset(nPixels);
    for(i=y1=0;y1<height;y1++)
      for(x1=0;x1<width;x1++,i++)
        P.join(i,I[i]);
    nParts = P.getNumberOfParts();

    // quit if the number of connected component did not change
    if(nParts==nPartsPrev) break;
    _log("  nParts = ",nParts);

  }
  _log("} segment2");
}

//////////////////////////////////////////////////////////////////////

// public static 
ImgArgb* ImgCheckerboards::segment3
(Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow) {
  ImgArgb* segmented = (ImgArgb*)0;
  PartitionLists P(1);
  segment3(srcImg, maxEdgeWeight, nSegments, P);
  segmented = makeSegmentedImg(P,-1,srcImg);
  return segmented;
}

// public static
void ImgCheckerboards::segment3
(Img& srcImg, float maxEdgeWeight, int nSegments, PartitionLists& P) {
  _log("segment3() {");
  _log("  maxEdgeWeight = ",maxEdgeWeight);
  _log("  nMinSegments  = ",nSegments);

  segment2(srcImg,maxEdgeWeight,nSegments,P);
  int width   = srcImg.getWidth();
  int height  = srcImg.getHeight();
  int nPixels = width*height;
  int nParts  = P.getNumberOfParts();

  int i,j,x,x1,y,y1,iEdge,iPart0,iPart1,iPix0,iPix1;

  //////////////////////////////////////////////////////////////////////
  Graph  partsGraph(nParts,false);
  VecInt partFirst;
  VecInt partSize;
  VecInt I(nPixels,0);
  for(i=y1=0;y1<height;y1++)
    for(x1=0;x1<width;x1++,i++)
      if((j=P.beg(i))>=0) {
        I[i] = partFirst.size();
        partFirst.append(j);
        partSize.append(P.size(i));
      }

  // horizontal edges ...
  for(iEdge=y=0;y<height;y++)
    for(x=0;x<width-1;x++,iEdge++) { // edge (y,x  )->(y,x+1)
      iPart0 = I[P.find((x  )+width*(y  ))];
      iPart1 = I[P.find((x+1)+width*(y  ))];
      if(iPart0!=iPart1 && partsGraph.getEdge(iPart0,iPart1)==(GraphEdge*)0 &&
         _areSimilarSizes(partSize[iPart0],partSize[iPart1],0.15f)) {
        partsGraph.insert(iPart0,iPart1);
      }
    }
  // vertical edges ...
  for(y=0;y<height-1;y++)
    for(x=0;x<width;x++,iEdge++) { // edge (y  ,x)->(y+1,x)
      iPart0 = I[P.find((x  )+width*(y  ))];
      iPart1 = I[P.find((x  )+width*(y+1))];
      if(iPart0!=iPart1 && partsGraph.getEdge(iPart0,iPart1)==(GraphEdge*)0 &&
         _areSimilarSizes(partSize[iPart0],partSize[iPart1],0.15f)) {
        partsGraph.insert(iPart0,iPart1);
      }
    }

  int nPartsMarked = 0;
  VecBool isMarked(nParts,false);
  GraphTraversal t(partsGraph);
  GraphEdge* e;
  t.start();
  while((e=t.next())!=(GraphEdge*)0) {
    iPart0 = e->getVertex(0);
    iPart1 = e->getVertex(1);
    if(isMarked[iPart0]==false) {
      isMarked[iPart0] = true;
      nPartsMarked++;
    }
    if(isMarked[iPart1]==false) {
      isMarked[iPart1] = true;
      nPartsMarked++;
    }
  }
  _log("  nParts       = ",nParts);
  _log("  nPartsMarked = ",nPartsMarked);

  if(0<nPartsMarked && nPartsMarked<nParts) {

    // TODO Tue Jun 25 18:17:55 2013 this heuristic does not work well
    // when the board is very tilted, since the sizes of the squares
    // change quite a bit from the front to the back
    //
    // perhaps we should fit a linear function to the sizes and
    // use the local size estimate to compare with the size of the
    // component

    // find the median size of marked components
    VecInt sizeMarked(nPartsMarked,0);
    for(i=iPart0=0;iPart0<nParts;iPart0++)
      if(isMarked[iPart0]==true) {
        sizeMarked[i] = partSize[iPart0];
        i++;
      }
    // Arrays.sort(sizeMarked);
    qSort(sizeMarked);

    int sizeMedian = sizeMarked[nPartsMarked/2];
    _log("  sizeMin    = ",sizeMarked[0]);
    _log("  sizeMedian = ",sizeMedian);
    _log("  sizeMax    = ",sizeMarked[nPartsMarked-1]);
    // mark all parts of size close to sizeMedian
    for(iPart0=0;iPart0<nParts;iPart0++)
      isMarked[iPart0] = _areSimilarSizes(partSize[iPart0],sizeMedian,0.75f);

    // find first unmarked part
    for(iPart0=0;iPart0<nParts;iPart0++)
      if(isMarked[iPart0]==false)
        break;
    _log("  joining unmarked components ...");
    // iPart0 is not marked
    iPix0 = P.find(partFirst[iPart0]);
    // join all the not marked parts with iPart0;
    for(iPart1=0;iPart1<nParts;iPart1++)
      if(isMarked[iPart1]==false) {
        iPix1 = P.find(partFirst[iPart1]);
        P.join(iPix0,iPix1);
      }
    _log("  nParts       = ",nParts);
  }

  _log("} segment3");
}

// public static
//////////////////////////////////////////////////////////////////////
ImgArgb* ImgCheckerboards::segment6
(Img& srcImg, float maxEdgeWeight, int nSegments, int fromRow, int toRow) {

  _log("segment6() {");
  _log("  maxEdgeWeight = ",maxEdgeWeight);
  _log("  nMinSegments  = ",nSegments);

  ImgFloat* pNx = sobelFloatEdgesIx(srcImg);
  ImgFloat* pNy = sobelFloatEdgesIy(srcImg);
  ImgFloat& Nx = *pNx;
  ImgFloat& Ny = *pNy;

  float dx,dy,dN,s,c,a,threshMax,threshMin,lambda,mu,alpha,w,wJoin;
  int   width,height,nPixels,nColors,nSteps,step,argb,r,g,b,h,histMax;
  int   col,row,i,j,k;
      
  width   = srcImg.getWidth();
  height  = srcImg.getHeight();
  nPixels = width*height;

  ImgArgb* img = new ImgArgb(width,height);

  _log("nPixels = ",nPixels);
      
  ImgFloat dNy(width,height);
  ImgFloat dNx(width,height);
  ImgFloat wN(width,height);

  lambda =  0.50f;
  mu     =  0.50f;

  // gradient field smoothing
  nSteps = 6;
  for(step=0;step<nSteps;step++) {
    alpha = (step%2==0)?lambda:mu;
    // zero accumulators
    dNy.clear();
    dNx.clear();
    wN.clear();
    // acumulate horizontal edges
    for(row=0;row<height;row++) {
      for(col=0;col+1<width;col++) {
        dx = Nx.get(col+1,row  )-Nx.get(col  ,row  );
        dy = Ny.get(col+1,row  )-Ny.get(col  ,row  );
        w  = 1.0f;
        dNx.add(col  ,row  , dx);
        dNy.add(col  ,row  , dy);
        wN.add (col  ,row  ,  w);
        dNx.add(col+1,row  ,-dx);
        dNy.add(col+1,row  ,-dy);
        wN.add (col+1,row  ,  w);
      }
    }
    // acumulate vertical edges
    for(row=0;row+1<height;row++) {
      for(col=0;col<width;col++) {
        dx = Nx.get(col  ,row+1)-Nx.get(col  ,row  );
        dy = Ny.get(col  ,row+1)-Ny.get(col  ,row  );
        w  = 1.0f;
        dNx.add(col  ,row  , dx);
        dNy.add(col  ,row  , dy);
        wN.add (col  ,row  ,  w);
        dNx.add(col  ,row+1,-dx);
        dNy.add(col  ,row+1,-dy);
        wN.add (col  ,row+1,  w);
      }
    }

    // normalize and update
    for(row=0;row<height;row++) {
      for(col=0;col<width;col++) {
        if((w=wN.get(col,row))>0.0f) {
          dx = alpha*dNx.get(col,row)/w;
          dy = alpha*dNy.get(col,row)/w;
          Nx.add(col,row,dx);
          Ny.add(col,row,dy);
        }
      }
    }
  }

  nColors = 8;
  VecInt color(nColors+1,0);
  for(h=0;h<nColors;h++)
    color[h] = _makeRandomColor();
  color[nColors] = 0xff000000;
  // int colorHist[nColors+1];

  // compute histogram
  VecInt hist(1024,0);
  for(row=0;row<height;row++) {
    for(col=0;col<width;col++) {
      dx   = Nx.get(col,row);
      dy   = Ny.get(col,row);
      dN   = sqrt(dx*dx+dy*dy);
      // -1.0f<=dAng<=1.0f ---> 0.0f<=dAng<=255.0f
      h = (int)(1023.0f*dN); if(h>1023) h = 1023;
      hist[h]++;
    }
  }
  // integrate
  // histMax = nPixels*925/1000;
  histMax = nPixels*950/1000;
  for(h=1;h<1024;h++)
    hist[h] += hist[h-1];
  for(h=0;h<1024;h++)
    if(hist[h]>histMax)
      break;
  // determine thresholds
  threshMax = ((float)h)/1023.0f;
  threshMin = threshMax/3.0f;
  // normalize values
  for(i=row=0;row<height;row++) {
    for(col=0;col<width;col++,i++) {
      dx   = Nx.get(col,row);
      dy   = Ny.get(col,row);
      dN   = sqrt(dx*dx+dy*dy);
      if(dN<threshMin) {
        dx = 0.0f;
        dy = 0.0f;
      } else if(dN>threshMax) {
        dx /= dN;
        dy /= dN;
      } else {
        dN = (dN-threshMin)/(threshMax-threshMin);
        // dN *= dN;
        dx *= dN;
        dy *= dN;
      }
      Nx.set(col,row,dx);
      Ny.set(col,row,dy);
    }
  }

  // partition
  PartitionLists P(nPixels);
  Heap H;
  // insert edges into the heap
  for(i=row=0;row<height;row++) {
    for(col=0;col+1<width;col++,i++) {
      w =
        Nx.get(col+1,row  )*Nx.get(col  ,row  )+
        Ny.get(col+1,row  )*Ny.get(col  ,row  );
      H.add(1.0f-w,col+row*width);
    }
  }
  for(row=0;row+1<height;row++) {
    for(col=0;col<width;col++,i++) {
      w = 
        Nx.get(col  ,row+1)*Nx.get(col  ,row  )+
        Ny.get(col  ,row+1)*Ny.get(col  ,row  );
      H.add(1.0f-w,col+row*width+nPixels);
    }
  }

  wJoin = 0.025f;

  VecBool visited(nPixels,false);

  // delete edges from heap and join
  j = -1;
  while(H.length()>0) {
    k = H.delMin();
    w = H.getLastFKey();
    i = H.getLastIKey();
    if(w>wJoin) { j=i; break;}
    if(i<nPixels) {
      row = i/(width);
      col = i%(width);
      // (col,row) -> (col+1,row)
      j = i+1;
      P.join(i,j);
      visited[i] = true;
      visited[j] = true;
    } else {
      i -= nPixels;
      row = i/(width);
      col = i%(width);
      // (col,row) -> (col,row+1);
      j = i+width;
      P.join(i,j);
      visited[i] = true;
      visited[j] = true;
    }
  }

  if(j>nPixels) j -= nPixels;
  _log("j=",j);

  for(i=row=0;row<height;row++)
    for(col=0;col<width;col++,i++)
      if(visited[i]==false)
        P.join(i,j);

  j = P.find(j);

  bool colorize = false;

  if(colorize) {
    // colorize
    for(i=row=0;row<height;row++) {
      for(col=0;col<width;col++,i++) {
        dx   = Nx.get(col,row);
        dy   = Ny.get(col,row);
        dN   = sqrt(dx*dx+dy*dy);
        argb = color[nColors];
        if(dN>0.0f) {
          c = dx/dN;
          s = dy/dN;
          a = (float)((1.0+(atan2(s,c)/M_PI))/2.0); //
          h = (int)(a*((float)(nColors-1)));
          if(h<0) h = 0; else if(h>=nColors) h = nColors-1;
          argb = color[h];
          dN *= dN;
        }

        if(true) {
          b = argb&0xff; argb>>=8;
          g = argb&0xff; argb>>=8;
          r = argb&0xff; argb>>=8;
          b = (int)(dN*((float)b));
          if(b<0) b = 0; else if(b>255) b = 255;
          g = (int)(dN*((float)g));
          if(g<0) g = 0; else if(g>255) g = 255;
          r = (int)(dN*((float)r));
          if(r<0) r = 0; else if(r>255) r = 255;
        } else {
          r = g = b = (int)(255.0f*dN);
        }
        argb = 0xff000000|(r<<16)|(g<<8)|(b);
        img->set(col,row,argb);
      }
    }
  } else {

    ImgArgb* imgI = makeSegmentedImg(P,-1,srcImg);
    img = new ImgArgb(width,height);
    for(i=row=0;row<height;row++)
      for(col=0;col<width;col++,i++)
        if(P.find(i)==j)
          img->set(col,row,0xff000000);
        else
          img->set(col,row,imgI->get(col,row));
    
  }

  delete pNx;
  delete pNy;
  
  _log("} segment6");
  return img;
}

// // public static 
// ImgArgb* ImgCheckerboards::segment11
// (Img& srcImg, float maxEdgeWeight, int nSegments) {
//   ImgArgb* segmented = (ImgArgb*)0;
//   PartitionLists P(1);
//   segment3(srcImg, maxEdgeWeight, nSegments, P);
//   segmented = makeSegmentedImg(P,-1,srcImg);
//   return segmented;
// }

// // public static
// void ImgCheckerboards::segment11
// (Img& srcImg, float maxEdgeWeight, int nSegments, PartitionLists& P) {
//   _log("segment3() {");
//   _log("  maxEdgeWeight = ",maxEdgeWeight);
//   _log("  nMinSegments  = ",nSegments);

//   // ...

//   _log("} segment11");
// }

//////////////////////////////////////////////////////////////////////
ImgArgb* ImgCheckerboards::segment11
(Img& srcImg, float maxEdgeWeight, int nMinSegments, int rowMin, int rowMax,
 GraphPlanar& edges) {

  _log("segment11() {");
  ImgArgb* segmented = (ImgArgb*)0;

  int width   = srcImg.getWidth();
  int height  = srcImg.getHeight();

  if(rowMin< 0     ) rowMin = 0;
  if(rowMax>=height) rowMax = height;

  int colMin = 0;
  int colMax = width;
  
  // int nPixels = width*height;
  int radius  = (width+height)/10; // for the box filter
  
  VecInt I(width*height,0);

  _log("  box filtering image ...");
  
  ImgArgb* boxFilteredImg = boxFilter(srcImg,radius);
  segmented = boxFilteredImg; // can reuse
  
  int   row,col,aI,rI,gI,bI,aB,rB,gB,bB,argb,argb0,argb1;
  int   grayI,grayB,i,i0,i1,iV0,iV1;
  
  int nMorphology = (radius/40); if(nMorphology<1) nMorphology = 1;
  
  _log("  initial binary segmentation ...");

  // initial binary segmentation
  float f = 0.0f;
  for(i=row=0;row<height;row++) {
    for(col=0;col<width;col++,i++) {
      
      // input pixel
      aI  = srcImg.get(col,row);
      bI  = aI&0xff; aI>>=8;
      gI  = aI&0xff; aI>>=8;
      rI  = aI&0xff; aI>>=8;
      aI &= 0xff;
      f = 0.299f*((float)rI)+0.587f*((float)gI)+0.114f*((float)bI);
      if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
      grayI = (int)floor(f);
      
      // box filtered pixel
      aB  = boxFilteredImg->get(col,row);
      bB  = aB&0xff; aB>>=8;
      gB  = aB&0xff; aB>>=8;
      rB  = aB&0xff; aB>>=8;
      aB &= 0xff;
      f = 0.299f*((float)rB)+0.587f*((float)gB)+0.114f*((float)bB);
      if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
      grayB = (int)floor(f);
      
      // binary segmentation based with respect to the gray box
      // filter value; store in bitplane 0x1
      
      I[i] = (grayI<=grayB)?0x00:0x01;
    }
  }

  // dilations/erosion

  _log("  dilation and erosion  ...");
  
  // copy white pixels from bitplane 0 onto bitplane 1
  ImgBitplane::copy(I,width,height,0,1,/*invert=*/false);
  // copy black pixels from bitplane 0 onto bitplane 2
  ImgBitplane::copy(I,width,height,0,2,/*invert=*/true);
  // dilate white pixels; use bitplane 3 as work space
  ImgBitplane::dilate(nMorphology,I,width,height,1,3);
  // dilation black pixels; use bitplane 3 as work space
  ImgBitplane::dilate(nMorphology,I,width,height,2,3);
  
  // construct and save output pixels

  const int white = 0xffffffff;
  const int red   = 0xffff0000;
  const int blue  = 0xff0000ff;
  const int black = 0xff000000;
  const int green = 0xff00ff00;

  for(i=row=0;row<height;row++) {
    for(col=0;col<width;col++,i++) {
      argb =
        (I[i]==WHITE)?white: /* 011 */
        (I[i]==RED  )?red:   /* 111 */
        (I[i]==BLUE )?blue:  /* 110 */
        (I[i]==BLACK)?black: /* 100 */
                      green; /* just in case */
      segmented->set(col,row,argb);
    }
  }

  _log("  red-blue isocurve ...");

  // compute cuberile iso-curve
  edges.reset(true);
  // VecFloat& coord = edges.getCoord();
  VecInt indx((width+1)*(height+1),-1); // vertex index
  // VecInt orEdge;
  // VecInt nEdges; // 1 or 2

  _log("  vertical edges ...");

  float x,y;

  // vertical edges
  int nV_vertical = 0;
  int nE_vertical = 0;
  for(row=rowMin;row<rowMax;row++) {
    for(col=colMin;col<colMax-1;col++) {

      argb0 = segmented->get(col  ,row);
      argb1 = segmented->get(col+1,row);

      //          col col+1
      //           |   |
      //         +---+---+
      //  row -> | 0 | 1 |
      //         +---+---+

      if(argb0==red && argb1==blue) {

        // RED | BLUE

        i0 = (col+1)+(row  )*(width+1);
        i1 = (col+1)+(row+1)*(width+1);

        // +---0---+ <--row
        // | R | B |
        // +---1---+ <--row+1
        //     |   
        //   col+1

        iV0 = indx[i0];
        if(iV0<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row  ))/((float)height);
          iV0 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i0] = iV0;
          nV_vertical++;
        }

        iV1 = indx[i1];
        if(iV1<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV1 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i1] = iV1;
          nV_vertical++;
        }

        edges.insertEdge(iV0,iV1);
        // orEdge.append(iV0);
        // orEdge.append(iV1);
        // nEdges[iV0]++;
        nE_vertical++;

      } else if(argb0==blue && argb1==red) {

        // BLUE | RED

        i0 = (col+1)+(row+1)*(width+1);
        i1 = (col+1)+(row  )*(width+1);

        // +---1---+ <--row
        // | B | R |
        // +---0---+ <--row+1
        //     |   
        //   col+1

        iV0 = indx[i0];
        if(iV0<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV0 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i0] = iV0;
          nV_vertical++;
        }

        iV1 = indx[i1];
        if(iV1<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row  ))/((float)height);
          iV1 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i1] = iV1;
          nV_vertical++;
        }

        edges.insertEdge(iV0,iV1);
        // orEdge.append(iV0);
        // orEdge.append(iV1);
        // nEdges[iV0]++;
        nE_vertical++;
      }
    }
  }
  _log("    nV_vertical = ",nV_vertical);
  _log("    nE_vertical = ",nE_vertical);
  _log("    vertices = ",edges.getNumberOfVertices());
  _log("    edges    = ",edges.getNumberOfEdges());

  _log("  horizontal edges ...");

  // horizontal edges
  int nV_horizontal = 0;
  int nE_horizontal = 0;
  for(row=rowMin;row<rowMax-1;row++) {
    for(col=colMin;col<colMax;col++) {

      argb0 = segmented->get(col,row);
      argb1 = segmented->get(col,row+1);

      //            col
      //             |
      //           +---+
      //  row   -> | 0 |
      //           +---+
      //  row+1 -> | 1 |
      //           +---+

      if(argb0==red && argb1==blue) {

        // RED | BLUE

        i0 = (col+1)+(row+1)*(width+1); iV0 = indx[i0];
        i1 = (col  )+(row+1)*(width+1); iV1 = indx[i1];

        //         col col+1
        //          |   |
        //          +---+
        //          | R |
        //  row+1 - 1-<-0
        //          | B |
        //          +---+

        if(iV0<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV0 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i0] = iV0;
          nV_horizontal++;
        }

        if(iV1<0) {
          x = ((float)(col  ))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV1 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i1] = iV1;
          nV_horizontal++;
        }

        edges.insertEdge(iV0,iV1);
        // orEdge.append(iV0);
        // orEdge.append(iV1);
        // nEdges[iV0]++;
        nE_horizontal++;

      } else if(argb0==blue && argb1==red) {

        // BLUE | RED

        i0 = (col  )+(row+1)*(width+1); iV0 = indx[i0];
        i1 = (col+1)+(row+1)*(width+1); iV1 = indx[i1];

        //         col col+1
        //          |   |
        //          +---+
        //          | B |
        //  row+1 - 0->-1
        //          | R |
        //          +---+

        if(iV0<0) {
          x = ((float)(col  ))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV0 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i0] = iV0;
          nV_horizontal++;
        }

        if(iV1<0) {
          x = ((float)(col+1))/((float)width);
          y = ((float)(row+1))/((float)height);
          iV1 = edges.pushBackCoord(x,y);
          // nEdges.append(0);
          indx[i1] = iV1;
          nV_horizontal++;
        }

        edges.insertEdge(iV0,iV1);
        // orEdge.append(iV0);
        // orEdge.append(iV1);
        // nEdges[iV0]++;
        nE_horizontal++;

      }
    }
  }
  _log("    nV_horizontal = ",nV_horizontal);
  _log("    nE_horizontal = ",nE_horizontal);
  _log("    vertices = ",edges.getNumberOfVertices());
  _log("    edges    = ",edges.getNumberOfEdges());

  _log("} segment11");
  return segmented;
}

//////////////////////////////////////////////////////////////////////
ImgArgb* ImgCheckerboards::segment12
(Img& srcImg, float maxEdgeWeight, int nMinSegments,
 int rowMin, int rowMax, int cbRows, int cbCols, Polylines& polylines) {

  _log("segment12() {");
  _log(QString("  rowMin       = %1").arg(rowMin));
  _log(QString("  rowMax       = %1").arg(rowMax));
  _log(QString("  checkerboard = %1x%2").arg(cbCols).arg(cbRows));

  int width   = srcImg.getWidth();
  int height  = srcImg.getHeight();

  if(rowMin< 0     ) rowMin = 0;
  if(rowMax>=height) rowMax = height;

  int colMin = 0;
  int colMax = width;

  ImgArgb* segmented = new ImgArgb(width,height);

  int   row,row0,row1,col,col0,col1,aI,rI,gI,bI;
  int   i,iV0,iV1,argb,argb0,argb1,argb2,argb3;
  float x,y;
  bool  reset_iV0;

  // int   row,row0,row1,col,col0,col1,aI,rI,gI,bI,aB,rB,gB,bB;
  // int   grayI,grayB,i,iV0,iV1,argb,argb0,argb1,argb2,argb3;
  // float x,y;
  // bool  reset_iV0;

  // const int white = 0xffffffff;
  const int red   = 0xffff0000;
  const int blue  = 0xff0000ff;
  // const int black = 0xff000000;
  // const int green = 0xff00ff00;

  for(i=row=0;row<height;row++) {
    for(col=0;col<width;col++,i++) {
      
      // input pixel
      aI  = srcImg.get(col,row);
      bI  = aI&0xff; aI>>=8;
      gI  = aI&0xff; aI>>=8;
      rI  = aI&0xff; aI>>=8;
      aI &= 0xff;

      argb = (rI>0x7f)?red:blue;
      segmented->set(col,row,argb);
    }
  }

  
  // VecInt I(width*height,0);
  //
  // // int nPixels = width*height;
  // int radius  = (width+height)/10; // for the box filter
  //  
  // _log("  box filtering image ...");
  //
  // ImgArgb* boxFilteredImg = boxFilter(srcImg,radius);
  // segmented = boxFilteredImg; // can reuse
  //
  // int   row,row0,row1,col,col0,col1,aI,rI,gI,bI,aB,rB,gB,bB;
  // int   grayI,grayB,i,iV0,iV1,argb,argb0,argb1,argb2,argb3;
  // float x,y;
  // bool  reset_iV0;
  //
  // int nMorphology = (radius/40); if(nMorphology<1) nMorphology = 1;
  //
  // _log("  initial binary segmentation ...");
  //
  // // initial binary segmentation
  // float f = 0.0f;
  // for(i=row=0;row<height;row++) {
  //   for(col=0;col<width;col++,i++) {
  //  
  //     // input pixel
  //     aI  = srcImg.get(col,row);
  //     bI  = aI&0xff; aI>>=8;
  //     gI  = aI&0xff; aI>>=8;
  //     rI  = aI&0xff; aI>>=8;
  //     aI &= 0xff;
  //     f = 0.299f*((float)rI)+0.587f*((float)gI)+0.114f*((float)bI);
  //     if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
  //     grayI = (int)floor(f);
  //  
  //     // box filtered pixel
  //     aB  = boxFilteredImg->get(col,row);
  //     bB  = aB&0xff; aB>>=8;
  //     gB  = aB&0xff; aB>>=8;
  //     rB  = aB&0xff; aB>>=8;
  //     aB &= 0xff;
  //     f = 0.299f*((float)rB)+0.587f*((float)gB)+0.114f*((float)bB);
  //     if(f<0.0f) f = 0.0f; else if(f>255.0f) f = 255.0f;
  //     grayB = (int)floor(f);
  //  
  //     // binary segmentation based with respect to the gray box
  //     // filter value; store in bitplane 0x1
  //  
  //     I[i] = (grayI<=grayB)?0x00:0x01;
  //   }
  // }
  //
  // // dilations/erosion
  //
  // _log("  dilation and erosion  ...");
  //
  // // copy white pixels from bitplane 0 onto bitplane 1
  // ImgBitplane::copy(I,width,height,0,1,/*invert=*/false);
  // // copy black pixels from bitplane 0 onto bitplane 2
  // ImgBitplane::copy(I,width,height,0,2,/*invert=*/true);
  // // dilate white pixels; use bitplane 3 as work space
  // ImgBitplane::dilate(nMorphology,I,width,height,1,3);
  // // dilation black pixels; use bitplane 3 as work space
  // ImgBitplane::dilate(nMorphology,I,width,height,2,3);
  //
  // // construct and save output pixels
  //
  // for(i=row=0;row<height;row++) {
  //   for(col=0;col<width;col++,i++) {
  //     argb =
  //       (I[i]==WHITE)?white: /* 011 */
  //       (I[i]==RED  )?red:   /* 111 */
  //       (I[i]==BLUE )?blue:  /* 110 */
  //       (I[i]==BLACK)?black: /* 100 */
  //       green; /* just in case */
  //     segmented->set(col,row,argb);
  //   }
  // }

  _log("  red-blue polylines ...");

  polylines.reset();

  VecInt indx0(width+1,-1); // vertex index
  VecInt indx1(width+1,-1); // vertex index
  VecInt indx2(width+1,-1); // vertex index

  int nV_vertical   = 0;
  int nE_vertical   = 0;
  int nV_horizontal = 0;
  int nE_horizontal = 0;

  for(row=rowMin;row<rowMax;row++) {
    row0 = row;
    row1 = row+1;

    // vertical edges (row0)
    for(col=colMin;col<colMax-1;col++) {

      col0 = col;
      col1 = col+1;

      argb0 = segmented->get(col0,row);
      argb1 = segmented->get(col1,row);

      //             col0   col1
      //              |      |
      //         +-------+-------+
      //  row -> | argb0 | argb1 |
      //         +-------+-------+

      if(argb0==red && argb1==blue) {

        // RED | BLUE

        // +---0---+ <--row0
        // | R | B |
        // +---1---+ <--row1
        //     |   
        //   col1

        iV0 = indx0[col1];
        if(iV0<0) {
          x = ((float)(col1))/((float)width);
          y = ((float)(row0))/((float)height);
          iV0 = polylines.pushBackCoord(x,y);
          indx0[col1] = iV0;
          nV_vertical++;
        }

        iV1 = indx1[col1];
        if(iV1<0) {
          x = ((float)(col1))/((float)width);
          y = ((float)(row1))/((float)height);
          iV1 = polylines.pushBackCoord(x,y);
          indx1[col1] = iV1;
          nV_vertical++;
        }

        polylines.setNext(iV0,iV1);
        nE_vertical++;

      } else if(argb0==blue && argb1==red) {

        // BLUE | RED

        // +---1---+ <--row0
        // | B | R |
        // +---0---+ <--row1
        //     |   
        //   col1

        iV0 = indx1[col1];
        if(iV0<0) {
          x = ((float)(col1))/((float)width);
          y = ((float)(row1))/((float)height);
          iV0 = polylines.pushBackCoord(x,y);
          indx1[col1] = iV0;
          nV_vertical++;
        }

        iV1 = indx0[col1];
        if(iV1<0) {
          x = ((float)(col1))/((float)width);
          y = ((float)(row0))/((float)height);
          iV1 = polylines.pushBackCoord(x,y);
          indx0[col1] = iV1;
          nV_vertical++;
        }

        polylines.setNext(iV0,iV1);
        nE_vertical++;
      }
    } // vertical edges (row0)

    if(row1==rowMax) break;

    // horizontal edges (row0|row1)
    indx2.fill(-1);
    for(col=0;col<width;col++) {

      col0 = col;
      col1 = col+1;

      argb0 = segmented->get(col0,row0);
      argb1 = segmented->get(col0,row1);
      if(col1<width) {
        argb2 = segmented->get(col1,row0);
        argb3 = segmented->get(col1,row1);
      } else {
        argb2 = argb3 = 0xff000000; // black
      }

      //              col0  col1
      //               |     |
      //           +-------+-------+
      //   row0 -> | argb0 | argb2 |
      //           +-------+-------+
      //   row1 -> | argb1 | qrgb3 |
      //           +-------+-------+

      if(argb0==red && argb1==blue) {

        // RED | BLUE

        //        col0 col1
        //          |   |
        //          +---x---
        //          | R | B    iV0>=0
        //   row1 - 1-<-0---   use to link iV0->iV1
        //          | B | B    reseting iV0 as in case 3 has no negative
        //          +---+---   effect
        //          +---+---
        //          | R | R
        //   row1 - 1-<-0-<-x  iV0<0
        //          | B | B    create new vertex
        //          +---+---
        //          +---x---
        //          | R | B    iV0>=0
        //   row1 - 1-<-0->-x  use to link iV0->iV1 and then reset
        //          | B ^ R    so that a new vertex will be created
        //          +---x---   when the next horizontal edge is processed
        //          +---+---
        //          | R | R
        //   row1 - 1-<-0---   iV0<0
        //          | B ^ R    create new vertex
        //          +---x---

        reset_iV0 = false;
        iV0 = indx1[col1];
        if(iV0>=0 /* argb2==blue && argb3==red */) {
          reset_iV0 = true;
        } else /* if(iV0<0) */ {
          x = ((float)(col1))/((float)width);
          y = ((float)(row1))/((float)height);
          iV0 = polylines.pushBackCoord(x,y);
          indx1[col1] = iV0;
          nV_horizontal++;
        }

        iV1 = indx1[col0];
        if(iV1<0) {
          x = ((float)(col0))/((float)width);
          y = ((float)(row1))/((float)height);
          iV1 = polylines.pushBackCoord(x,y);
          indx1[col0] = iV1;
          nV_horizontal++;
        }

        polylines.setNext(iV0,iV1);
        if(reset_iV0) indx1[col1] = -1;
        nE_horizontal++;

      } else if(argb0==blue && argb1==red) {

        // BLUE | RED

        //        col0 col1
        //          |   |
        //          +---+---+
        //          | B | B    iV1<0
        //   row1 - 0->-1---+  create new iV1 and use to link iV0->iV1
        //          | R | B    keep in indx1 for next row of vertical edges
        //          +---x---+
        //          +---x---+  iV1>=0
        //          | B | R    save iV1 in indx2
        //   row1 - 0->-21->-x create new iV1, link iV0->iV1
        //          | R | B    after finishing the horizontal row, copy nonzero
        //          +---x---+  values from indx2 to indx1 and reset indx2
        //          +---+---+
        //          | B | B
        //   row1 - 0->-1->-x  iV1<0
        //          | R | R    create new iV1 and use to link iV0->iV1
        //          +---+---+  keep in indx1 for next row of vertical edges
        //          +---+---+
        //          | B | R    iV1>=0
        //   row1 - 0->-1---+  link iV0->iV1
        //          | R | R    keep in indx1 for next row of vertical edges
        //          +---+---+  

        iV0 = indx1[col0];
        if(iV0<0) {
          x = ((float)(col0))/((float)width);
          y = ((float)(row1))/((float)height);
          iV0 = polylines.pushBackCoord(x,y);
          indx1[col0] = iV0;
          nV_horizontal++;
        }

        iV1 = indx1[col1];
        if(iV1>=0) {
          if(argb2==red && argb3==blue) {
            indx2[col1] = iV1;
            x = ((float)(col1))/((float)width);
            y = ((float)(row1))/((float)height);
            iV1 = polylines.pushBackCoord(x,y);
          }
        } else /* if(iV1<0) */ {
          x = ((float)(col1))/((float)width);
          y = ((float)(row1))/((float)height);
          iV1 = polylines.pushBackCoord(x,y);
          indx1[col1] = iV1;
          nV_horizontal++;
        }

        polylines.setNext(iV0,iV1);
        nE_horizontal++;

      }

    } // horizontal edges (row0|row1)

    // get ready for next row of vertical edges
    for(col=0;col<=width;col++)
      if(indx2[col]>=0) {
        indx1[col] = indx2[col];
        indx2[col] = -1;
      }

    indx0.swap(indx1);
    indx1.fill(-1);
  }

  polylines.resetFirst();

  _log("} segment12");
  return segmented;
}

//////////////////////////////////////////////////////////////////////
// public

Img* ImgCheckerboards::detect
(Img&         srcImg,
 GraphPlanar& graphP,
 GraphPlanar& graphD,
 Mesh&        mesh,
 int          background,
 bool         paintVertexDiscs,
 bool         paintBoundaryDiscs,
 bool         drawBorderTriangles) {
  
  _log("detect() {");

  _log("  background          = ",background);
  _log("  paintVertexDiscs    = ",paintVertexDiscs);
  _log("  paintBoundaryDiscs  = ",paintBoundaryDiscs);
  _log("  drawBorderTriangles = ",drawBorderTriangles);

  ImgArgb* segmented = (ImgArgb*)0;

  int   width        = srcImg.getWidth();
  int   height       = srcImg.getHeight();
  int   nPixels      = width*height;
  int   nMorphology  = ((width+height)/500); if(nMorphology<1) nMorphology = 1;
  
  _log("  width        = ",width);
  _log("  height       = ",height);
  _log("  nPixels      = ",nPixels);
  _log("  nMorphology  = ",nMorphology);
  
  CircleNeighborhood::initialize((width+height)/20);
  
  _log("  computing initial pixel segmentation ...");

  PartitionLists P(nPixels);
  int iBlack = boxFilterThreshold(srcImg,nMorphology,P);

  switch(background) {
  case 1: // black
    segmented = new ImgArgb(width,height,0xff000000);
    break;
  case 2: // segmentation
    segmented = makeSegmentedImg(P,iBlack,srcImg);
    break;
  default: // image
    segmented = new ImgArgb(srcImg);
    break;
  }

  _log("  creating quads ...");

  VecFloat coord;
  VecInt   coordIndex;

  _createQuads(srcImg,nMorphology,P,coord,coordIndex);

  ImgBuckets buckets(width,height);

  int nVbefore,nVafter;

  bool meshHasChanged = false;

  nVafter = coord.size()/2;

  _log("  do {");
    
  do {
    meshHasChanged = false;
    nVbefore = nVafter;

    if(true) {
      _log("   clustering edge centers ...");
      _clusterBoundaryEdgeCenter
        (buckets,coord,coordIndex,*segmented,paintVertexDiscs);
      if(meshHasChanged==false && nVafter!=coord.size()/2)
        meshHasChanged = true;
      nVafter = coord.size()/2;
      _log("  nV = ",nVafter);
    }

    if(true) {
      _log("  clustering border vertices ...");
      _clusterBoundaryVertices
        (buckets,coord,coordIndex,*segmented,paintVertexDiscs);
      if(meshHasChanged==false && nVafter!=coord.size()/2)
        meshHasChanged = true;
      nVafter = coord.size()/2;
      _log("  nV = ",nVafter);
    }

    if(false) {
      _log("  regular quad mesh filter ...");
      _regularQuadMeshFilter(coord,coordIndex);
      if(meshHasChanged==false && nVafter!=coord.size()/2)
        meshHasChanged = true;
      nVafter = coord.size()/2;
      _log("  nV = ",nVafter);
    }

    _log("  nVbefore       = ",nVbefore);
    _log("  nVafter        = ",nVafter);
    _log("  meshHasChanged = ",meshHasChanged);

  } while(meshHasChanged);
    
  _log("  } do");

  if(true) {
    _log("  regular quad mesh filter ...");
    _regularQuadMeshFilter(coord,coordIndex);
    Img* img = dynamic_cast<Img*>(segmented);
    _clusterBoundaryVertices
      (buckets,coord,coordIndex,*img,paintVertexDiscs);
  }

  //////////////////////////////////////////////////////////////////////
  _log("  making mesh ...");
      
  {
    mesh.erase();
    // vertices
    VecFloat& coordSrc = *mesh.getCoordSrc();
    VecFloat& coordDst = *mesh.getCoordDst();
    coordSrc.append(coord);
    coordDst.append(coord);
    // faces
    VecInt& coordIndexMesh = *mesh.getCoordIndex();
    coordIndexMesh.append(coordIndex);
    // connectivity
    mesh.makeEdges();
    mesh.makeLines();
    mesh.makeSelection();
  }

  if(drawBorderTriangles)
    _drawBorderTriangles(mesh,graphD);

  _log("} detect");

  return segmented;
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_regularQuadMeshFilter
(VecFloat& coord, VecInt& coordIndex) {
    
  if(coord.size()<=0) return;

  // _log("_regularQuadMeshFilter() {");

  int nV = coord.size()/2;
  int nF = coordIndex.size()/5; // assumes quad mesh
  PartitionLists Pvertex(nV);
  PartitionLists Pface(nF);

  // PartitionLists Pdual = new PartitionLists(nF);
  GraphFaces g(coordIndex,nV);
  GraphEdge* e;

  // location of face in a regular quad grid
  // only meaningful if component[iF]>=0
  VecInt quadRow(nF,-1);
  VecInt quadCol(nF,-1);

  VecInt stack;
  VecBool visited(nF,false);

  int   iComponent,iF,iF0,i0,i1,i2,i3,iV0,iV1,iV2,iV3,iV;
  int   iW0,iW1,iW2,iW3,iF01,iF12,iF23,iF30,i,nFdel,nFadd,nVadd,nFaddComponent;
  int   row,col,rowMin,colMin,rowMax,colMax,width,height;
  float x,y;
 
  for(iF0=0;iF0<nF;iF0++) {
    if(visited[iF0]) continue;

    visited[iF0] = true;
    quadRow[iF0]=0;
    quadCol[iF0]=0;

    // _log(String.format(" {"));
    // _log(String.format("   (%4d,%4d) %4d root",0,0,iF0));
    stack.clear();
    stack.append(iF0);
    while(stack.size()>0) {

      iF  = stack.last(); stack.removeLast();

      row = quadRow[iF];
      col = quadCol[iF];
      iV0 = coordIndex[5*iF  ];
      iV1 = coordIndex[5*iF+1];
      iV2 = coordIndex[5*iF+2];
      iV3 = coordIndex[5*iF+3];
      e = g.getEdge(iV0,iV1);
      iF01 = (g.isRegularEdge(*e))?g.getOtherEdgeFace(*e,iF):-1;
      e = g.getEdge(iV1,iV2);
      iF12 = (g.isRegularEdge(*e))?g.getOtherEdgeFace(*e,iF):-1;
      e = g.getEdge(iV2,iV3);
      iF23 = (g.isRegularEdge(*e))?g.getOtherEdgeFace(*e,iF):-1;
      e = g.getEdge(iV3,iV0);
      iF30 = (g.isRegularEdge(*e))?g.getOtherEdgeFace(*e,iF):-1;

      if(iF01>=0 && visited[iF01]==false) {
        // find iV0 in iF01;
        for(i=0;i<4;i++)
          if(coordIndex[5*iF01+i]==iV0) break;
        // reorder vertices within face
        iW3 = coordIndex[5*iF01+i]; i = (i+1)%4;
        iW0 = coordIndex[5*iF01+i]; i = (i+1)%4;
        iW1 = coordIndex[5*iF01+i]; i = (i+1)%4;
        iW2 = coordIndex[5*iF01+i];
        coordIndex[5*iF01  ]=iW0;
        coordIndex[5*iF01+1]=iW1;
        coordIndex[5*iF01+2]=iW2;
        coordIndex[5*iF01+3]=iW3;
        quadCol[iF01]=col  ;
        quadRow[iF01]=row-1;

        Pface.join(iF01,iF);
        visited[iF01] = true;
        stack.append(iF01);

        // _log(String.format("   (%4d,%4d) %4d = D(%4d)",
        // quadCol.get(iF01),quadRow.get(iF01),iF01,iF));
      }

      if(iF12>=0 && visited[iF12]==false) {
        // find iV0 in iF12;
        for(i=0;i<4;i++)
          if(coordIndex[5*iF12+i]==iV1) break;
        // reorder vertices within face
        iW0 = coordIndex[5*iF12+i]; i = (i+1)%4;
        iW1 = coordIndex[5*iF12+i]; i = (i+1)%4;
        iW2 = coordIndex[5*iF12+i]; i = (i+1)%4;
        iW3 = coordIndex[5*iF12+i];
        coordIndex[5*iF12  ]=iW0;
        coordIndex[5*iF12+1]=iW1;
        coordIndex[5*iF12+2]=iW2;
        coordIndex[5*iF12+3]=iW3;
        quadCol[iF12]=col+1;
        quadRow[iF12]=row  ;

        Pface.join(iF12,iF);
        visited[iF12] = true;
        stack.append(iF12);

        // _log(String.format("   (%4d,%4d) %4d = R(%4d)",
        // quadCol.get(iF12),quadRow.get(iF12),iF12,iF));
      }

      if(iF23>=0 && visited[iF23]==false) {
        // find iV0 in iF23;
        for(i=0;i<4;i++)
          if(coordIndex[5*iF23+i]==iV2) break;
        // reorder vertices within face
        iW1 = coordIndex[5*iF23+i]; i = (i+1)%4;
        iW2 = coordIndex[5*iF23+i]; i = (i+1)%4;
        iW3 = coordIndex[5*iF23+i]; i = (i+1)%4;
        iW0 = coordIndex[5*iF23+i];
        coordIndex[5*iF23  ]=iW0;
        coordIndex[5*iF23+1]=iW1;
        coordIndex[5*iF23+2]=iW2;
        coordIndex[5*iF23+3]=iW3;
        quadCol[iF23]=col  ;
        quadRow[iF23]=row+1;

        Pface.join(iF23,iF);
        visited[iF23] = true;
        stack.append(iF23);

        // _log(String.format("   (%4d,%4d) %4d = U(%4d)",
        // quadCol.get(iF23),quadRow.get(iF23),iF23,iF));
      }

      if(iF30>=0 && visited[iF30]==false) {
        // find iV0 in iF30;
        for(i=0;i<4;i++)
          if(coordIndex[5*iF30+i]==iV3) break;
        // reorder vertices within face
        iW2 = coordIndex[5*iF30+i]; i = (i+1)%4;
        iW3 = coordIndex[5*iF30+i]; i = (i+1)%4;
        iW0 = coordIndex[5*iF30+i]; i = (i+1)%4;
        iW1 = coordIndex[5*iF30+i];
        coordIndex[5*iF30  ]=iW0;
        coordIndex[5*iF30+1]=iW1;
        coordIndex[5*iF30+2]=iW2;
        coordIndex[5*iF30+3]=iW3;
        quadCol[iF30]=col-1;
        quadRow[iF30]=row  ;

        Pface.join(iF30,iF);
        visited[iF30] = true;
        stack.append(iF30);

        // _log(String.format("   (%4d,%4d) %4d = L(%4d)",
        // quadCol.get(iF30),quadRow.get(iF30),iF30,iF));
      }
    }
    // _log(String.format(" }"));
  }

  // String s,s0,s1;
  VecInt vertex;
  ImgBuckets grid;
  // _log("components {");
  for(nFdel=nFadd=nVadd=iComponent=iF0=0;iF0<nF;iF0++) {
    if(Pface.find(iF0)!=iF0) continue;
    // _log("component "+iComponent+" {");

    rowMin = rowMax = quadRow[iF0];
    colMin = colMax = quadCol[iF0];
    for(iF=Pface.beg(iF0);iF>=0;iF=Pface.next(iF)) {
      row = quadRow[iF];
      if(row<rowMin) rowMin = row;
      if(row>rowMax) rowMax = row;
      col = quadCol[iF];
      if(col<colMin) colMin = col;
      if(col>colMax) colMax = col;
    }
    for(iF=Pface.beg(iF0);iF>=0;iF=Pface.next(iF)) {
      quadRow[iF]-=rowMin;
      quadCol[iF]-=colMin;
    }
    colMax -= colMin; colMin = 0;
    rowMax -= rowMin; rowMin = 0; 
    width  = colMax+1;
    height = rowMax+1;

    // _log("    grid "+width+" x "+height);
    vertex.clear();
    int n = (width+1)*(height+1);
    vertex.reserve(n);
    vertex.insert(0,n,-1);

    grid.reset(width,height);
    grid.init(nF);

    for(iF=Pface.beg(iF0);iF>=0;iF=Pface.next(iF)) {
      grid.set(quadCol[iF],quadRow[iF],iF);
    }

    for(row=0;row<height;row++) {
      // s = "   [ ";
      for(col=0;col<width;col++) {
        n = 0;
        for(iF=grid.getFirst(col,row);iF>=0;iF=grid.getNext(iF))
          n++;
        // s = s + String.format("%2d ",n);
      }
      // s = s+"]";
      // _log(s);
    }

    // _log(" processing vertex array");

    for(row=0;row<height;row++) {
      for(col=0;col<width;col++) {
        n = 0;
        for(iF=grid.getFirst(col,row);iF>=0;iF=grid.getNext(iF)) {

          iV0 = coordIndex[5*iF  ];
          iV1 = coordIndex[5*iF+1];
          iV2 = coordIndex[5*iF+2];
          iV3 = coordIndex[5*iF+3];

          // // _log(" ["+col+","+row+"] face "+iF+" "+
          // +"{ "+iV0+" "+iV1+" "+iV2+" "+iV3+" }");

          i  = (col  )+(row  )*(width+1);
          iV = vertex[i];
          if(iV<0) {
            vertex[i] = iV0;
          } else if(iV!=iV0) {
            Pvertex.join(iV,iV0);
            // _log("  joining vertices "+iV+" and "+iV0);
            coordIndex[5*iF]=iV;
          }
          i  = (col+1)+(row  )*(width+1);
          iV = vertex[i];
          if(iV<0) {
            vertex[i] = iV1;
          } else if(iV!=iV1) {
            Pvertex.join(iV,iV1);
            // _log("  joining vertices "+iV+" and "+iV1);
            coordIndex[5*iF+1]=iV;
          }
          i  = (col+1)+(row+1)*(width+1);
          iV = vertex[i];
          if(iV<0) {
            vertex[i] = iV2;
          } else if(iV!=iV2) {
            Pvertex.join(iV,iV2);
            // _log("  joining vertices "+iV+" and "+iV2);
            coordIndex[5*iF+2]=iV;
          }
          i  = (col  )+(row+1)*(width+1);
          iV = vertex[i];
          if(iV<0) {
            vertex[i] = iV3;
          } else if(iV!=iV3) {
            Pvertex.join(iV,iV3);
            // _log("  joining vertices "+iV+" and "+iV3);
            coordIndex[5*iF+3]=iV;
          }

          if(n>0) { // delete face
            // _log("  deleting duplicate face "+iF);
            coordIndex[5*iF  ]=-1;
            coordIndex[5*iF+1]=-1;
            coordIndex[5*iF+2]=-1;
            coordIndex[5*iF+3]=-1;
            quadCol[iF]=-1;
            quadRow[iF]=-1;
            nFdel++;
          }
          n++;
        }

      } // for(col=0;col<width;col++)
    } // for(row=0;row<height;row++)

    do {

      for(row=0;row<height;row++) {
        // s = "   [ ";
        for(col=0;col<width;col++) {
          n = 0;
          for(iF=grid.getFirst(col,row);iF>=0;iF=grid.getNext(iF))
            n++;
          // s = s + String.format("%2d ",n);
        }
        // s = s+"]";
        // _log(s);
      }

      // add missing faces and vertices
      // _log(" adding missing faces and vertices ...");
      nFaddComponent = nFadd;
      for(row=0;row<height;row++) {
        for(col=0;col<width;col++) {
          if(grid.getFirst(col,row)>=0) continue;

          n = 0;
          i0 = (col  )+(row  )*(width+1);
          i1 = (col+1)+(row  )*(width+1);
          i2 = (col+1)+(row+1)*(width+1);
          i3 = (col  )+(row+1)*(width+1);
          iV0 = vertex[i0]; if(iV0>=0) n++;
          iV1 = vertex[i1]; if(iV1>=0) n++;
          iV2 = vertex[i2]; if(iV2>=0) n++;
          iV3 = vertex[i3]; if(iV3>=0) n++;

          // _log("  missing face @ ("+col+","+row+") = "+
          // "{ "+iV0+" "+iV1+" "+iV2+" "+iV3+" }");

          if(n==3) {
            if(iV0<0) {
              x = coord[2*iV1  ]-coord[2*iV2  ]+coord[2*iV3  ];
              y = coord[2*iV1+1]-coord[2*iV2+1]+coord[2*iV3+1];
              iV0 = coord.size()/2;
              // _log("  adding vertex "+iV0);
              vertex[i0] = iV0;
                coord.append(x);
                coord.append(y);
              Pvertex.addElement();
              nVadd++;
            } else if(iV1<0) {
              x = coord[2*iV2  ]-coord[2*iV3  ]+coord[2*iV0  ];
              y = coord[2*iV2+1]-coord[2*iV3+1]+coord[2*iV0+1];
              iV1 = coord.size()/2;
              // _log("  adding vertex "+iV1);
              vertex[i1] = iV1;
                coord.append(x);
                coord.append(y);
              Pvertex.addElement();
              nVadd++;
            } else if(iV2<0) {
              x = coord[2*iV3  ]-coord[2*iV0  ]+coord[2*iV1  ];
              y = coord[2*iV3+1]-coord[2*iV0+1]+coord[2*iV1+1];
              iV2 = coord.size()/2;
              // _log("  adding vertex "+iV2);
              vertex[i2] = iV2;
                coord.append(x);
                coord.append(y);
              Pvertex.addElement();
              nVadd++;
            } else /* if(iV3<0) */ {
              x = coord[2*iV0  ]-coord[2*iV1  ]+coord[2*iV2  ];
              y = coord[2*iV0+1]-coord[2*iV1+1]+coord[2*iV2+1];
              iV3 = coord.size()/2;
              // _log("  adding vertex "+iV3);
              vertex[i3] = iV3;
                coord.append(x);
                coord.append(y);
              Pvertex.addElement();
              nVadd++;
            }
          }
          if(n>=3) {
            iF = coordIndex.size()/5;
            // _log("  adding missing "+n+" face("+col+","+row+") "+iF+
            // " { "+iV0+" "+iV1+" "+iV2+" "+iV3+" }");
            coordIndex.append(iV0);
            coordIndex.append(iV1);
            coordIndex.append(iV2);
            coordIndex.append(iV3);
            coordIndex.append( -1);
            grid.addElement();
            grid.set(col,row,iF);
            quadCol.append(col);
            quadRow.append(row);
            nFadd++;
            n++;
          }

        } // for(col=0;col<width;col++)
      } // for(row=0;row<height;row++)
      nFaddComponent = nFadd-nFaddComponent;
      // _log("  nFaddComponent = "+nFaddComponent);      

    } while(nFaddComponent>0);

    // _log("}");      

    iComponent++;
  }
  // _log("}");

  // _log("  initial values");
  // _log("   nV    = "+nV);
  // _log("   nF    = "+nF);
  nV = coord.size()/2;
  nF = coordIndex.size()/5;

  // remove empty faces
  for(iF0=iF=0;iF<nF;iF++) {
    iV0 = coordIndex[5*iF  ];
    iV1 = coordIndex[5*iF+1];
    iV2 = coordIndex[5*iF+2];
    iV3 = coordIndex[5*iF+3];
    if(iV0<0 || iV1<0 || iV2<0 || iV3<0) continue;
    if(iF!=iF0) {
      coordIndex[5*iF0  ]=iV0;
      coordIndex[5*iF0+1]=iV1;
      coordIndex[5*iF0+2]=iV2;
      coordIndex[5*iF0+3]=iV3;
    }
    iF0++;
  }
  // _log("   iF0   = "+iF0);

  while(coordIndex.size()>5*iF0) {
    coordIndex.removeLast();
  }
    
  // _log("  before _clusterVertices");
  // _log("   nV    = "+nV);
  // _log("   nVadd = "+nVadd);
  // _log("   nF    = "+nF);
  // _log("   nFadd = "+nFadd);
  // _log("   nFdel = "+nFdel);
  // _log("   nP    = "+Pvertex.getNumberOfParts());

  _clusterVertices(coord,coordIndex,Pvertex);
  nV = coord.size()/2;
  nF = coordIndex.size()/5;

  // _log("  after _clusterVertices");
  // _log("   nV    = "+nV);
  // _log("   nF    = "+nF);

  _dualMeshFilter(coord,coordIndex);

  // _log("}");
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_createQuads
(Img& srcImg, int nMorphology, PartitionLists& P,
 VecFloat& coord, VecInt& coordIndex) {

  // _log("_createQuads() {");

  int      width     = srcImg.getWidth();
  int      height    = srcImg.getHeight();
  int      nPixels   = width*height;

  if(width<=0 || height<=0 || nPixels!=P.getNumberOfElements()) return;

  float    maxDiag   = (float)(width+height)/7;
  float    maxAspect = 0.95f;
  float coordQuad[8];

  coord.clear();
  coordIndex.clear();

  ImgFloat* sobelStrength = _sobelStrength(srcImg);
  // ImgFloat tmpStrength = _sobelStrength(srcImg);
  // ImgFloat sobelStrength = _minNeighborhood(tmpStrength);
  // ImgFloat sobelStrength = new ImgFloat(width,height,0.0f);

  int   i,k,iPart;
  float f,x0,y0,x1,y1,x2,y2,x3,y3,xC,yC;
  float d0,d1,d2,d3,d02,d13,d01,d12,d23,d30,aspect,eps;

  eps = 1.5f*(float)nMorphology;
    
  for(iPart=-1,i=0;i<nPixels;i++) {
    if(i!=P.find(i)) continue;
    iPart++;
    // _log("  cluster "+iPart);
      
    // n  = P.size(i);
      
    f  = _quadness19(30,P,i,*sobelStrength,coordQuad);
    // f  = _quadness15b(30,P,i,sobelStrength,coordQuad);
    if(f<=0.70f) {
      // _log("    did not pass quadness test f="+f);
      continue; // quadness test
    }
      
    x0 = coordQuad[0]; y0 = coordQuad[1];
    x1 = coordQuad[2]; y1 = coordQuad[3];
    x2 = coordQuad[4]; y2 = coordQuad[5];
    x3 = coordQuad[6]; y3 = coordQuad[7];

    xC = (x0+x1+x2+x3)/4.0f;
    yC = (y0+y1+y2+y3)/4.0f;

    // lengths of two diagonals
    d02 = _dist(x0,y0,x2,y2);
    d13 = _dist(x1,y1,x3,y3);

    if(d02>=maxDiag || d13>=maxDiag) {
      // _log("    did not pass the diag test d02="+
      //      d02+" d13="+d13+" maxDiag="+maxDiag);
      continue; // diagonal test
    }

    // lengths of four sides
    d01 = _dist(x0,y0,x1,y1);
    d12 = _dist(x1,y1,x2,y2);
    d23 = _dist(x2,y2,x3,y3);
    d30 = _dist(x3,y3,x0,y0);

    aspect = 
      ((float)fabs(((d01+d23)/(d12+d30))-1.0f)+
       (float)fabs(((d12+d30)/(d01+d23))-1.0f))/2.0f;

    if(false)
      if(aspect>=maxAspect) {
        // _log("    did not pass aspect ratio test aspect="+
        //      aspect+" maxAspect="+maxAspect);
        continue; // aspect ratio test
      }

    // _log("    creating quad");

    // expand the quads by (nMorphology) in each direction

    d0 = _dist(x0,y0,xC,yC);
    d1 = _dist(x1,y1,xC,yC);
    d2 = _dist(x2,y2,xC,yC);
    d3 = _dist(x3,y3,xC,yC);

    x0 = xC+((eps+d0)/d0)*(x0-xC);
    x1 = xC+((eps+d1)/d1)*(x1-xC);
    x2 = xC+((eps+d2)/d2)*(x2-xC);
    x3 = xC+((eps+d3)/d3)*(x3-xC);

    y0 = yC+((eps+d0)/d0)*(y0-yC);
    y1 = yC+((eps+d1)/d1)*(y1-yC);
    y2 = yC+((eps+d2)/d2)*(y2-yC);
    y3 = yC+((eps+d3)/d3)*(y3-yC);

    k = coord.size()/2;
      coord.append(x0/((float)width));
      coord.append(y0/((float)height));
      coord.append(x1/((float)width));
      coord.append(y1/((float)height));
      coord.append(x2/((float)width));
      coord.append(y2/((float)height));
      coord.append(x3/((float)width));
      coord.append(y3/((float)height));

    // quad connectivity
    coordIndex.append(k  );
    coordIndex.append(k+1);
    coordIndex.append(k+2);
    coordIndex.append(k+3);
    coordIndex.append(  -1);
  }

  // _log("}");
}

//////////////////////////////////////////////////////////////////////
// private static 

void ImgCheckerboards::_clusterBoundaryEdgeCenter
  (ImgBuckets& buckets, VecFloat& coord,
   VecInt& coordIndex, Img& segmented,
   bool paintVertexDiscs) {

  int width  = buckets.getWidth();
  int height = buckets.getHeight();

  int   nV,nF,nE,iF0,iF1,i0,i1,j0,j1,iE0,iE1,iV0,iV1,iV2,iV3;
  int   row,col,row0,col0,r,c,radEdge,radEdgeMax,radSearch,nFound,nCircle,h;
  float x,y,x0,y0,x1,y1,x2,y2,x3,y3,xC0,yC0,xB0,yB0,xC1,yC1,xB1,yB1;
  float radE,d03,d12,dC0,dC1,d01,radV;
        
  nV = coord.size()/2;
  nF = coordIndex.size()/5;
  PartitionLists P(nV);

  // comopute vertex radii
  VecFloat coordRadius(nV,0.0f);
  for(iF0=0;iF0<nF;iF0++) {
    for(i0=0;i0<4;i0++) {
      if((i1=i0+1)==4) i1 = 0; 
      iV0 = coordIndex[5*iF0+i0];
      iV1 = coordIndex[5*iF0+i1];
      x0  = ((float) width)*coord[2*iV0  ];
      y0  = ((float)height)*coord[2*iV0+1];
      x1  = ((float) width)*coord[2*iV1  ];
      y1  = ((float)height)*coord[2*iV1+1];
      d01 = 0.4f*_dist(x0,y0,x1,y1);
      radV = coordRadius[iV0];
      if(radV==0.0f || d01<radV) coordRadius[iV0]=d01;
      radV = coordRadius[iV1];
      if(radV==0.0f || d01<radV) coordRadius[iV1]=d01;
    }
  }

  for(radEdgeMax=radEdge=iV0=0;iV0<nV;iV0++) {
    radEdge = (int)ceil(coordRadius[iV0]);
    if(radEdge>radEdgeMax) radEdgeMax = radEdge;
  }
  CircleNeighborhood::initialize(radEdgeMax+1);
  VecInt& circleValues = CircleNeighborhood::getValues();
  nCircle = circleValues.size()/2;

  VecInt   indxEdge;
  VecFloat coordEdge;

  GraphFaces g(coordIndex,nV);
  GraphEdge*  e;

  // _log("   find boundary edges ...");
  for(iF0=0;iF0<nF;iF0++) {
    for(i0=0;i0<4;i0++) {
      if((i1=i0+1)==4) i1 = 0; 
      iV0 = coordIndex[5*iF0+i0];
      iV1 = coordIndex[5*iF0+i1];
      e = g.getEdge(iV0,iV1);
      if(g.isBoundaryEdge(*e)==false) continue;
      indxEdge.append(iF0);
      indxEdge.append(i0);
    }
  }
  nE = indxEdge.size()/2;
        
  // _log("    nV = "+nV);
  // _log("    nF = "+nF);
  // _log("    nE = "+nE);

  // _log("   compute edge centers and insert them in the buckets ...");

  buckets.init(nE);

  for(iE0=0;iE0<nE;iE0++) {
    iF0 = indxEdge[2*iE0  ];
    i0  = indxEdge[2*iE0+1];
    i1  = (i0+1)%4;
    iV0 = coordIndex[5*iF0+i0]; // == 4*iF1+j0
    iV1 = coordIndex[5*iF0+i1]; // == 4*iF1+j1
    x   = ((float) width)*(coord[2*iV0  ]+coord[2*iV1  ])/2.0f;
    y   = ((float)height)*(coord[2*iV0+1]+coord[2*iV1+1])/2.0f;
    coordEdge.append(x);
    coordEdge.append(y);
    // insert edge index in the proper bucket
    col = (int)x;
    row = (int)y;
    buckets.set(col,row,iE0);
  }

  // _log("   compute face centers ...");

  VecFloat coordFace;
  _computeFaceCenters((float)width,(float)height,coord,coordIndex,coordFace);

  // _log("   searching for neighboring midpoints ...");

  for(iE0=0;iE0<nE;iE0++) {
    iF0 = indxEdge[2*iE0  ];
    i0  = indxEdge[2*iE0+1];
    i1  = (i0+1)%4;

    //   X ----- X
    //   |       |
    //   |  pC0  | -- iF0
    //   |       |
    //  p0 ----> p1
    //    \     /
    //     \   /   
    //      pB0 = p0+p1-pC0

    // vertex indices of edge ends
    iV0 = coordIndex[5*iF0+i0];
    iV1 = coordIndex[5*iF0+i1];
    // coordinates of edge ends
    x0 = ((float) width)*coord[2*iV0  ];
    y0 = ((float)height)*coord[2*iV0+1];
    x1 = ((float) width)*coord[2*iV1  ];
    y1 = ((float)height)*coord[2*iV1+1];
    // coordinates of face center
    xC0 = coordFace[2*iF0  ];
    yC0 = coordFace[2*iF0+1];
    // coordinates of boundary vertex
    xB0 = x0+x1-xC0;
    yB0 = y0+y1-yC0;

    // center of search disk
    col = (int)coordEdge[2*iE0  ];
    row = (int)coordEdge[2*iE0+1];
    // radius of search disk
    radE    = (coordRadius[iV0]+coordRadius[iV1])/2.0f;
    radEdge = (int)ceil(radE);
    // scan the disc 
    radSearch = radEdge;
    for(nFound=h=0;h<nCircle;h++) {
      r = circleValues[2*h  ];
      if((row0=row+r)< 0 || row0>= height) continue;
      c = circleValues[2*h+1];
      if((col0=col+c)< 0 || col0>= width) continue;
      if(r*r+c*c>radEdge*radEdge) break;
      for(iE1=buckets.getFirst(col0,row0);iE1>=0;iE1=buckets.getNext(iE1)) {
        if(iE1==iE0) continue;

        //      pB1 = p2+p3-pC1
        //     /   \
        //    /     \
        // iV3 <-0-- iV2
        //  |         |
        //  |   pC1   | -- iF1
        //  |         |
        //  X ------- X

        // found another edge center within the search disk
        // face index
        iF1 = indxEdge[2*iE1  ];
        // vertex indices of edge ends
        j0  = indxEdge[2*iE1+1];
        j1  = (j0+1)%4;
        iV2 = coordIndex[5*iF1+j0];
        iV3 = coordIndex[5*iF1+j1];
        // coordinates of edge ends
        x2  = ((float) width)*coord[2*iV2  ];
        y2  = ((float)height)*coord[2*iV2+1];
        x3  = ((float) width)*coord[2*iV3  ];
        y3  = ((float)height)*coord[2*iV3+1];
        // coordinates of face center
        xC1 = coordFace[2*iF1  ];
        yC1 = coordFace[2*iF1+1];
        // coordinates of boundary vertex
        xB1 = x2+x3-xC1;
        yB1 = y2+y3-yC1;

        //  X ------- X
        //  |   pC0   |
        //  |   pB1   |
        //  |  /   \  |
        // iV0 --0-> iV1
        // iV3 <-0-- iV2
        //  |  \   /  |
        //  |   pB0   |
        //  |   pC1   |
        //  X ------- X

        // distances between pairs of matching ege ends
        d03 = _dist(x0,y0,x3,y3);
        d12 = _dist(x1,y1,x2,y2);

        // distances between face centers and boundary vertices
        dC0 = _dist(xC0,yC0,xB1,yB1);
        dC1 = _dist(xC1,yC1,xB0,yB0);

        if(d03<=radE && d12<=radE && dC0<=radE && dC1<=radE) {
          P.join(iV0,iV3);
          P.join(iV1,iV2);
        }

        // update the search radius
        radSearch = (int)sqrt((float)(r*r+c*c));

        nFound++;
      }
      if(nFound>=1) break;
    }

    // if(paintVertexDiscs) {
    //   color = ImgSegmentation.makeRandomColor();
    //   ImgDraw.vertex(segmented,col,row,color,radSearch);
    // }

  } // for(iE0=0;iE0<nE;iE0++)
  _clusterVertices(coord,coordIndex,P);
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_clusterBoundaryVertices
(ImgBuckets& buckets, VecFloat& coord,
 VecInt& coordIndex, Img& segmented,
 bool paintVertexDiscs) {

  int width  = buckets.getWidth();
  int height = buckets.getHeight();

  int   nV,nF,nFnew,nVnew,nE,iF,iF0,iF1,iF2,iF3,iE0,iE1,iE2,iE3;
  int   iV00,iV01,iV10,iV11,iV20,iV21,iV30,iV31;
  int   iV0,iV1,iV2,iV3,color;
  int   i00,i01,i10,i11,i20,i21,i30,i31;
  int   radSearch,nCircle,col,row,col0,row0,c,r,h;
  float x00,y00,x01,y01,x10,y10,x11,y11;
  float x20,y20,x21,y21,x30,y30,x31,y31;
  float xC0,yC0,xB0,yB0,xB1,yB1;
  float xB2,yB2,xB3,yB3;
  float x,y,radB,radBmax;
  float d0,d1,d2,d3,dA,dB,dC,dD,eps1,eps2,len;
  GraphEdge* e;

  // String s;

  nF = coordIndex.size()/5; // it only contains quads
  nV = coord.size()/2;
  PartitionLists P(nV);

  // _log("   compute face centers ...");

  VecFloat coordFace;
  for(iF0=0;iF0<nF;iF0++) {
    x = y = 0.0f;
    for(i00=0;i00<4;i00++) {
      iV00 = coordIndex[5*iF0+i00];
      x += ((float) width)*coord[2*iV00  ];
      y += ((float)height)*coord[2*iV00+1];
    }
    x /= 4.0f; y /= 4.0f;
    coordFace.append(x);
    coordFace.append(y);
  }
 
  // _log("   compute boundary vertices ...");

  GraphFaces g(coordIndex,nV);

  VecFloat coordBoundary;
  VecFloat radiusBoundary;
  VecInt   dataBoundary;

  radBmax = 0.0f;
  for(iF0=0;iF0<nF;iF0++) {
    for(i00=0;i00<4;i00++) {
      i01 = (i00+1)%4;
      iV00 = coordIndex[5*iF0+i00];
      iV01 = coordIndex[5*iF0+i01];
      e = g.getEdge(iV00,iV01);
      if(g.isBoundaryEdge(*e)) {
        iE0 = coordBoundary.size()/2;
        // vertex coordinates
        x00 = ((float) width)*coord[2*iV00  ];
        y00 = ((float)height)*coord[2*iV00+1];
        x01 = ((float) width)*coord[2*iV01  ];
        y01 = ((float)height)*coord[2*iV01+1];
        // coordinates of face center
        xC0 = coordFace[2*iF0  ];
        yC0 = coordFace[2*iF0+1];
        // coordinates of boundary vertex
        xB0 = x00+x01-xC0;
        yB0 = y00+y01-yC0;
        // radius
        d0 = _dist(xC0,yC0,x00,y00);
        d1 = _dist(xC0,yC0,x01,y01);
        radB = 0.50f*((d0<d1)?d0:d1);
        if(radB>radBmax) radBmax = radB;
        // save
          coordBoundary.append(xB0);
          coordBoundary.append(yB0);
        radiusBoundary.append(radB);
          dataBoundary.append(iF0); // (face)
          dataBoundary.append(i00); // (corner)

        if(paintVertexDiscs) {
          color = _makeRandomColor();
          // ImgDraw.vertex(segmented,col,row,color,radSearch);
            ImgDraw::vertex(segmented,(int)xB0,(int)yB0,color,(int)ceil(radB));
        }

      }
    }
  }
  nE = coordBoundary.size()/2;

  // _log("   insert boundary vertices in buckets ...");

  // ImgBuckets buckets = new ImgBuckets(width,height);
  buckets.init(nE);
  for(iE0=0;iE0<nE;iE0++) {
    col = (int)coordBoundary[2*iE0  ];
    row = (int)coordBoundary[2*iE0+1];
    if(col<0 || col>=width || row<0 || row>=height) continue;
    buckets.set(col,row,iE0);
  }

  // _log("   search for matching boundary vertices ...");

    CircleNeighborhood::initialize(1+(int)ceil(radBmax));
    VecInt& circleValues = CircleNeighborhood::getValues();
  nCircle = circleValues.size()/2;

  VecBool usedBoundary(nE,false);

  // to make the compiler happy
  xB0  = xB1  = xB2  = xB3  = 0;
  yB0  = yB1  = yB2  = yB3  = 0;
  iF0  = iF1  = iF2  = iF3  = 0;
  i00  = i10  = i20  = i30  = 0;
  i01  = i11  = i21  = i31  = 0;
  iV00 = iV10 = iV20 = iV30 = 0;
  iV01 = iV11 = iV21 = iV31 = 0;
  x00  = x10  = x20  = x30  = 0;
  y00  = y10  = y20  = y30  = 0;
  x01  = x11  = x21  = x31  = 0;
  y01  = y11  = y21  = y31  = 0;
  d0   = d1   = d2   = d3   = 0;

  VecInt found;
  for(iE0=0;iE0<nE;iE0++) {
    if(usedBoundary[iE0]) continue;
    // retrieve the boundary edge data
    xB0  = coordBoundary[2*iE0  ];
    yB0  = coordBoundary[2*iE0+1];
    iF0  = dataBoundary[2*iE0  ];
    i00  = dataBoundary[2*iE0+1];
    i01  = (i00+1)%4;
    iV00 = coordIndex[5*iF0+i00];
    iV01 = coordIndex[5*iF0+i01];
    x00  = ((float) width)*coord[2*iV00  ];
    y00  = ((float)height)*coord[2*iV00+1];
    x01  = ((float) width)*coord[2*iV01  ];
    y01  = ((float)height)*coord[2*iV01+1];
    d0   = _dist(x01,y01,x00,y00);

    // scan the disc
    found.clear();
    col = (int)xB0;
    row = (int)yB0;
    radB = radiusBoundary[iE0];
    radSearch = (int)ceil(radB);
    for(h=0;h<nCircle;h++) {
      r = circleValues[2*h  ];
      if((row0=row+r)< 0 || row0>= height) continue;
      c = circleValues[2*h+1];
      if((col0=col+c)< 0 || col0>= width) continue;
      if(r*r+c*c>radSearch*radSearch) break;
      for(iE1=buckets.getFirst(col0,row0);iE1>=0;iE1=buckets.getNext(iE1)) {
        if(iE1==iE0) continue;
        found.append(iE1);

        usedBoundary[iE0] = true;
        usedBoundary[iE1] = true;

        if(found.size()>=3) break;
      }
    } // for(h=0;h<nCircle;h++)

    if(found.size()==0) continue;

    // s = ""+(1+found.size())+" { "+iE0;
    // for(i=0;i<found.size();i++)
    //   s = s+" "+(found[i));
    // s = s+" }";
    // _log(s);

    if(found.size()>=1) {
      iE1  = found[0];
      xB1  = coordBoundary[2*iE1  ];
      yB1  = coordBoundary[2*iE1+1];
      iF1  = dataBoundary[2*iE1  ];
      i10  = dataBoundary[2*iE1+1];
      i11  = (i10+1)%4;
      iV10 = coordIndex[5*iF1+i10];
      iV11 = coordIndex[5*iF1+i11];
      x10  = ((float) width)*coord[2*iV10  ];
      y10  = ((float)height)*coord[2*iV10+1];
      x11  = ((float) width)*coord[2*iV11  ];
      y11  = ((float)height)*coord[2*iV11+1];
      d1   = _dist(x11,y11,x10,y10);
    }
    if(found.size()>=2) {
      iE2 = found[1];
      xB2  = coordBoundary[2*iE2  ];
      yB2  = coordBoundary[2*iE2+1];
      iF2  = dataBoundary[2*iE2  ];
      i20  = dataBoundary[2*iE2+1];
      i21  = (i20+1)%4;
      iV20 = coordIndex[5*iF2+i20];
      iV21 = coordIndex[5*iF2+i21];
      x20  = ((float) width)*coord[2*iV20  ];
      y20  = ((float)height)*coord[2*iV20+1];
      x21  = ((float) width)*coord[2*iV21  ];
      y21  = ((float)height)*coord[2*iV21+1];
      d2   = _dist(x21,y21,x20,y20);
    }
    if(found.size()>=3) {
      iE3 = found[2];
      xB3  = coordBoundary[2*iE3  ];
      yB3  = coordBoundary[2*iE3+1];
      iF3  = dataBoundary[2*iE3  ];
      i30  = dataBoundary[2*iE3+1];
      i31  = (i30+1)%4;
      iV30 = coordIndex[5*iF3+i30];
      iV31 = coordIndex[5*iF3+i31];
      x30  = ((float) width)*coord[2*iV30  ];
      y30  = ((float)height)*coord[2*iV30+1];
      x31  = ((float) width)*coord[2*iV31  ];
      y31  = ((float)height)*coord[2*iV31+1];
      d3   = _dist(x31,y31,x30,y30);
    }

    iV0 = iV1 = iV2 = iV3 = -1;

    if(found.size()==1) {
      // iE0,iE1

      dA  = _dist(x11,y11,x00,y00);
      dB  = _dist(x10,y10,x01,y01);
      dC  = _dist(x11,y11,x10,y10);
      dD  = _dist(x00,y00,x01,y01);

      len = ((d0>d1)?d0:d1);
      eps1 = ((d0<d1)?d0:d1)/4.0f;
      eps2 = 0.35f*eps1;

      if((iV01==iV10) || (dA>len && dB<eps1)) {
        // dB ~ 0 & dA^2 ~ d0^2+d1^2
        //         1
        // dB 10 --->--- 11
        // 01
        //  |       dA
        //  ^ 0
        //  |
        // 00
        // _log("  case {0,1}");
        P.join(iV01,iV10);
        iV0 = iV11;
        iV1 = iV01;
        iV2 = iV00;
        iV3 = coord.size()/2;
        x = x00+x11-x01;
        y = y00+y11-y01;
          coord.append(x/((float)width));
          coord.append(y/((float)height));
      } else if((iV00==iV10) || (dB>len && dA<eps1)) {
        // dA ~ 0 & dB^2 ~ d0^2+d1^2 
        //         0
        // dA 00 --->--- 01
        // 11
        //  |      dB
        //  ^ 1
        //  |
        // 10
        // _log("  case {1,0}");
        P.join(iV00,iV11);
        iV0 = iV01;
        iV1 = iV11;
        iV2 = iV10;
        iV3 = coord.size()/2;
        x = x01+x10-x00;
        y = y01+y10-y00;
          coord.append(x/((float)width));
          coord.append(y/((float)height));
      } else if( fabs(dA-dB)<eps2 &&
                 fabs(dC-dD)<eps2 &&
                 fabs(dA-dC)<eps2 &&
                 fabs(dB-dD)<eps2 ) {
        // dA ~ dB ~ d0 ~ d1
        //         0
        //   00 --->--- 01
        //    |         | 
        // dA ?         ? dB 
        //    |         | 
        //   11 ---<--- 10
        //       1
        // _log("  case ||");
        iV0 = iV01;
        iV1 = iV00;
        iV2 = iV11;
        iV3 = iV10;

        // TODO Fri Jul 19 16:42:34 2013
        // we need a better test for this case

      } else {

        // _log("  case ?");

      }

    } else if(found.size()==2 || found.size()==3) {
      // iE0,iE1,iE2

      len = d0; if(d1>len) len = d1; if(d2>len) len = d2;
      eps1 = d0; if(d1<eps1) eps1 = d1; if(d2<eps1) eps1 = d2;
      eps1 /= 4.0f;

      // there are 6 cases
      // 0,1,2
      // 0,2,1
      // 1,0,2
      // 1,2,0
      // 2,0,1
      // 2,1,0

      if(_dist(x01,y01,x10,y10)<eps1) {
        // 00->-01=10->11
        P.join(iV01,iV10);
        if(_dist(x21,y21,x00,y00)<eps1) {
          // 2,0,1 | 20->-21=00->-01=10->11
          P.join(iV21,iV00);
          if(found.size()==3) {
            // 2,0,1,3 | 20->-21=00->-01=10->11=30->-31
            P.join(iV31,iV20);
            P.join(iV11,iV30);
            // _log("  case {2,0,1,3}");
          } else {
            // _log("  case {2,0,1}");
          }
          iV3 = iV20; iV2 = iV21; iV1 = iV01; iV0 = iV11;
        } else if(_dist(x11,y11,x20,y20)<eps1) {
          // 0,1,2 | 00->-01=10->-11=20->-21
          P.join(iV11,iV20);
          if(found.size()==3) {
            // 0,1,2,3 | 00->-01=10->-11=20->-21=30->-31
            P.join(iV31,iV00);
            P.join(iV21,iV30);
            // _log("  case {0,1,2,3}");
          } else {
            // _log("  case {0,1,2}");
          }
          iV3 = iV00; iV2 = iV01; iV1 = iV11; iV0 = iV21;
        }
      } else if(_dist(x01,y01,x20,y20)<eps1) {
        // 00->-01=20->-21
        P.join(iV01,iV20);
        if(_dist(x21,y21,x10,y10)<eps1) {
          // 0,2,1 | 00->01=20->21=10->-11
          P.join(iV21,iV10);
          if(found.size()==3) {
            // 0,2,1,3 | 00->01=20->21=10->-11=30->-31
            P.join(iV31,iV00);
            P.join(iV11,iV30);
            // _log("  case {0,2,1,3}");
          } else {
            // _log("  case {0,2,1}");
          }
          iV3 = iV00; iV2 = iV01; iV1 = iV21; iV0 = iV11;
        } else if(_dist(x11,y11,x00,y00)<eps1) {
          // REDUNDANT`
          // 1,0,2 | 10->-11=00->01=20->-21
          P.join(iV11,iV00);
          if(found.size()==3) {
            // 1,0,2,3 | 10->-11=00->01=20->-21=30->-31
            P.join(iV31,iV10);
            P.join(iV21,iV30);
            // _log("  case {1,0,2,3}");
          } else {
            // _log("  case {1,0,2}");
          }
          iV3 = iV10; iV2 = iV11; iV1 = iV01; iV0 = iV21;
        }
      } else if(_dist(x11,y11,x00,y00)<eps1) {
        // 10->-11=00->-01
        P.join(iV11,iV00);
        if(_dist(x21,y21,x10,y10)<eps1) {
          // 2,1,0 | 20->-21=10->-11=00->01
          P.join(iV21,iV10);
          if(found.size()==3) {
            // 2,1,0,3 | 20->-21=10->-11=00->01=30->-31
            P.join(iV31,iV20);
            P.join(iV01,iV30);
            // _log("  case {2,1,0,3}");
          } else {
            // _log("  case {2,1,0}");
          }
          iV3 = iV20; iV2 = iV21; iV1 = iV11; iV0 = iV01;
        } else if(_dist(x01,y01,x20,y20)<eps1) {
          // REDUNDANT
          // 1,0,2 | 10->-11=00->01=20->-21
          P.join(iV01,iV20);
          if(found.size()==3) {
            P.join(iV31,iV10);
            P.join(iV21,iV30);
            // _log("  case {1,0,2,3}");
          } else {
            // _log("  case {1,0,2}");
          }
          iV3 = iV10; iV2 = iV11; iV1 = iV01; iV0 = iV21;
        }
      } else if(_dist(x11,y11,x20,y20)<eps1) {
        // 10->-11=20->-21
        P.join(iV11,iV20);
        if(_dist(x21,y21,x00,y00)<eps1) {
          // 1,2,0 | 10->-11=20->21=00->-01
          P.join(iV21,iV00);
          if(found.size()==3) {
            // 1,2,0,3 | 10->-11=20->21=00->-01=30->-31
            P.join(iV31,iV10);
            P.join(iV01,iV30);
            // _log("  case {1,2,0,3}");
          } else {
            // _log("  case {1,2,0}");
          }
          iV3 = iV10; iV2 = iV11; iV1 = iV21; iV0 = iV01;
        } else if(_dist(x01,y01,x10,y10)<eps1) {
          // REDUNDANT
          // 0,1,2 | 00->01=10->-11=20->-21
          P.join(iV01,iV10);
          if(found.size()==3) {
            // 0,1,2,3 | 00->01=10->-11=20->-21=30->-31
            P.join(iV31,iV00);
            P.join(iV21,iV30);
            // _log("  case {0,1,2,3}");
          } else {
            // _log("  case {0,1,2}");
          }
          P.join(iV01,iV10);
          iV3 = iV00; iV2 = iV01; iV1 = iV11; iV0 = iV21;
        }
      } else if(_dist(x21,y21,x00,y00)<eps1) {
        // 20->-21=00->-01
        P.join(iV21,iV00);
        if(_dist(x11,y11,x20,y20)<eps1) {
          // 1,2,0 | 10->-11=20->-21=00->-01
          P.join(iV11,iV20);
          if(found.size()==3) {
            // 1,2,0,3 | 10->-11=20->-21=00->-01=30->-31
            P.join(iV31,iV10);
            P.join(iV01,iV30);
            // _log("  case {1,2,0,3}");
          } else {
            // _log("  case {1,2,0}");
          }
          iV3 = iV10; iV2 = iV11; iV1 = iV21; iV0 = iV01;
        } else if(_dist(x01,y01,x10,y10)<eps1) {
          // REDUNDANT
          // 2,0,1 | 20->-21=00->-01=10->-11
          P.join(iV01,iV10);
          if(found.size()==3) {
            // 2,0,1,3 | 20->-21=00->-01=10->-11=30->-31
            P.join(iV31,iV20);
            P.join(iV11,iV30);
            // _log("  case {2,0,1,3}");
          } else {
            // _log("  case {2,0,1}");
          }
          iV3 = iV20; iV2 = iV21; iV1 = iV01; iV0 = iV11;
        }
      } else if(_dist(x21,y21,x10,y10)<eps1) {
        // 20->-21=10->11
        P.join(iV21,iV10);
        if(_dist(x01,y01,x20,y20)<eps1) {
          // REDUNDANT
          // 0,2,1 | 00->-01=20->-21=10->-11
          P.join(iV01,iV20);
          if(found.size()==3) {
            // 0,2,1,3 | 00->-01=20->-21=10->-11=30->-31
            P.join(iV31,iV00);
            P.join(iV11,iV30);
            // _log("  case {0,2,1,3}");
          } else {
            // _log("  case {0,2,1}");
          }
          iV3 = iV00; iV2 = iV01; iV1 = iV21; iV0 = iV11;
        } else if(_dist(x11,y11,x00,y00)<eps1) {
          // REDUNDANT
          // 2,1,0 | 20->-21=10->11=00->-01
          P.join(iV11,iV00);
          if(found.size()==3) {
            // 2,1,0,3 | 20->-21=10->11=00->-01=30->-31
            P.join(iV31,iV20);
            P.join(iV01,iV30);
            // _log("  case {2,1,0,3}");
          } else {
            // _log("  case {2,1,0}");
          }
          iV3 = iV20; iV2 = iV21; iV1 = iV11; iV0 = iV01;
        }
      }

    }

    if(iV0>=0 && iV1>=0 && iV2>=0 && iV3>=0) {
      iF = coordIndex.size()/5;
      // _log("  face "+iF+" { "+iV0+" "+iV1+" "+iV2+" "+iV3+" }");
      coordIndex.append(iV0);
      coordIndex.append(iV1);
      coordIndex.append(iV2);
      coordIndex.append(iV3);
      coordIndex.append(-1);
    }

  } // for(iE0=0;iE0<nE;iE0++)

  _clusterVertices(coord,coordIndex,P);

  nFnew = coordIndex.size()/5; // it only contains quads
  nVnew = coord.size()/2;

  // _log("  nV before = "+nV);
  // _log("  nF before = "+nF);
  // _log("  nV after  = "+nVnew);
  // _log("  nF after  = "+nFnew);
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_drawBorderTriangles
(Mesh& mesh, GraphPlanar& graphD) {

  _log("  drawing border triangles ...");

  int   nV,nF,iE,iV,iV0,iV1,iV2,iV3,iF,i0,i1,i2,i3,i;
  float xC,yC,xB,yB,x0,y0,x1,y1,x2,y2,x3,y3;

  // output is saved in the dual graph
  graphD.reset(false);
  VecFloat& coordD         = graphD.getCoord();

  // get mesh vertex coordinates and faces
  VecFloat& coordSrc       = *mesh.getCoordSrc();
  // VecFloat& coordDst       = *mesh.getCoordDst();
  VecInt&   coordIndexMesh = *mesh.getCoordIndex();

  nV = coordSrc.size()/2;
  nF = coordIndexMesh.size()/5;

  // create the mesh graph
  GraphFaces g(coordIndexMesh,nV);
  // traverse the graph looking for boundary edges
  GraphTraversal t(g);
  GraphEdge* e;
  for(t.start();(e=t.next())!=(GraphEdge*)0;) {
    iE = e->getIndex();
    if(g.isBoundaryEdge(iE)==false) continue;
    iV0 = e->getVertex(0);
    iV1 = e->getVertex(1);
    iF  = g.getEdgeFace(iE,0);
    // find the vertex corners in the face
    i0 = i1 = -1;
    for(i=0;i<4;i++) {
      iV = coordIndexMesh[5*iF+i];
      if(iV==iV0) i0 = i; else if(iV==iV1) i1 = i;
      if(i0>=0 && i1>=0) break;
    }
    if((i=(i0+1)%4)!=i1) {
      // make edge and face orientations consistent
      i = i0; i0 = i1; i1 = i; i = iV0; iV0 = iV1; iV1 = i;
    }
    i2 = (i1+1)%4; iV2 = coordIndexMesh[5*iF+i2];
    i3 = (i2+1)%4; iV3 = coordIndexMesh[5*iF+i3];
    // compute face center coordinates
    xC = yC = 0;
    x0 = coordSrc[2*iV0  ]; xC += x0;
    y0 = coordSrc[2*iV0+1]; yC += y0;
    x1 = coordSrc[2*iV1  ]; xC += x1;
    y1 = coordSrc[2*iV1+1]; yC += y1;
    x2 = coordSrc[2*iV2  ]; xC += x2;
    y2 = coordSrc[2*iV2+1]; yC += y2;
    x3 = coordSrc[2*iV3  ]; xC += x3;
    y3 = coordSrc[2*iV3+1]; yC += y3;
    xC /= 4.0f; yC /= 4.0f;
    // compute boundary vertex coordinates
    xB = x0+x1-xC;
    yB = y0+y1-yC;
    // insert two edges in dual graph
    i = coordD.size()/2;
    coordD.append(x0); // i
    coordD.append(y0); // i
    coordD.append(xB); // i+1
    coordD.append(yB); // i+1
    coordD.append(x1); // i+2
    coordD.append(y1); // i+2
    graphD.insertEdge(i  ,i+1);
    graphD.insertEdge(i+1,i+2);
  }
}

//////////////////////////////////////////////////////////////////////
// TODO Fri Jul 19 11:46:05 2013
// fails if coordIndex has empty faces !!!

// private static 
void ImgCheckerboards::_clusterVertices
(VecFloat& coord, VecInt& coordIndex, PartitionLists& P) {

  int nV = 0;
  if((nV=coord.size()/2)!=P.getNumberOfElements() || nV<=0) return;

  int     iV0,iV1,nVC,iVC,n,i,i0,i1,iF;
  bool keep;
  float   x,y;

  VecFloat coordClustered;
  VecInt   coordIndexClustered;

  VecInt map(nV,-1);
  for(iVC=iV0=0;iV0<nV;iV0++) {
    if(P.find(iV0)!=iV0) continue;
    // one new vertex per cluster
    iVC = coordClustered.size()/2; map[iV0] = iVC;
    // compute average vertex coordinates and save them
    x = y = 0.0f; n = 0;
    for(iV1=P.beg(iV0);iV1>=0;iV1=P.next(iV1))  {
      x += coord[2*iV1  ]; y += coord[2*iV1+1]; n++;
    }
    x /= ((float)n); y /= ((float)n);
    //save new vertex coordinates
    coordClustered.append(x);
    coordClustered.append(y);
  }
  nVC = coordClustered.size()/2;

  // extend the map to non-root vertices
  for(iV0=0;iV0<nV;iV0++)
    if((iV1=P.find(iV0))!=iV0)
      map[iV0] = map[iV1];

  // use an array to determine if a face has repeated vertices
  VecBool used(nVC,false);

  // fix the faces
  for(iF=i0=i1=0;i1<coordIndex.size();i1++) {
    if(coordIndex[i1]>=0) continue;
    keep = true; // keep the face unless repeated vertices are found
    for(i=i0;keep==true && i<i1;i++) {
      iV0 = coordIndex[i];
      iVC = map[iV0];
      if(used[iVC]) keep = false; // face will be discarded
      used[iVC] = true;
    }
    if(keep) { // create a face
      for(i=i0;i<i1;i++)
        coordIndexClustered.append(map[coordIndex[i]]);
      coordIndexClustered.append(-1);
    }
    // reset the used[] array
    for(i=i0;i<i1;i++)
      used[map[coordIndex[i]]] = false;
    // advance to next face
    i0 = i1+1; iF++;
  }

  // coord.swap(coordClustered);
    coord.clear();
    coord.append(coordClustered);
  // coordIndex.swap(coordIndexClustered);
    coordIndex.clear();
    coordIndex.append(coordIndexClustered);

  _removeUnusedVertices(coord,coordIndex);
}

//////////////////////////////////////////////////////////////////////
// private static 

void ImgCheckerboards::_dualMeshFilter(VecFloat& coord, VecInt& coordIndex) {

  int nV,nF,iE,iF0,iF1,iV0,iV1,iV2,iV3,i;
  float x,y;

  // _log("  processing mesh connected components ...");

  nV = coord.size()/2;
  nF = coordIndex.size()/5;
  // Pmesh.reset(nF);
  PartitionLists Pmesh(nF);

  // _log("  nV = "+nV);
  // _log("  nF = "+nF);

  VecBool marked(nV,false);

  GraphFaces gF(coordIndex,nV);
  GraphTraversal t(gF);
  GraphEdge* e;
  t.start();
  while((e=t.next())!=(GraphEdge*)0) {
    iE = e->getIndex();
    if(gF.isRegularEdge(iE)) {
      iF0 = gF.getEdgeFace(iE,0);
      iF1 = gF.getEdgeFace(iE,1);
      Pmesh.join(iF0,iF1);
      marked[coordIndex[5*iF0  ]] = true;
      marked[coordIndex[5*iF0+1]] = true;
      marked[coordIndex[5*iF0+2]] = true;
      marked[coordIndex[5*iF0+3]] = true;
      marked[coordIndex[5*iF1  ]] = true;
      marked[coordIndex[5*iF1+1]] = true;
      marked[coordIndex[5*iF1+2]] = true;
      marked[coordIndex[5*iF1+3]] = true;
    }
  }

  VecInt coordIndexClean;
  for(iF0=0;iF0<nF;iF0++) {
    iV0 = coordIndex[5*iF0  ];
    iV1 = coordIndex[5*iF0+1];
    iV2 = coordIndex[5*iF0+2];
    iV3 = coordIndex[5*iF0+3];
    iF1 = Pmesh.find(iF0);
    if(Pmesh.size(iF1)>=5) {
      if(marked[iV0]||marked[iV1]||
         marked[iV2]||marked[iV3]) {
        coordIndexClean.append(iV0);
        coordIndexClean.append(iV1);
        coordIndexClean.append(iV2);
        coordIndexClean.append(iV3);
        coordIndexClean.append(-1);
      }
    }
  }
  marked.fill(false);
  for(i=0;i<coordIndexClean.size();i++)
    if((iV0=coordIndexClean[i])>=0)
      marked[iV0] = true;

  VecFloat coordClean;
  VecInt map(nV,-1);
  for(iV0=0;iV0<nV;iV0++)
    if(marked[iV0]) {
      map[iV0] = coordClean.size()/2;
      x = coord[2*iV0  ];
      y = coord[2*iV0+1];
      coordClean.append(x);
      coordClean.append(y);
    }
  for(i=0;i<coordIndexClean.size();i++)
    if((iV0=coordIndexClean[i])>=0)
      coordIndexClean[i] = map[iV0];

  coord.swap(coordClean);
  coordIndex.swap(coordIndexClean);
}

//////////////////////////////////////////////////////////////////////
// private static

ImgFloat* ImgCheckerboards::_sobelStrength(Img& srcImg) {
  ImgFloat* sobelStrength = (ImgFloat*)0;
  // try {
    int width  = srcImg.getWidth();
    int height = srcImg.getHeight();
    sobelStrength = new ImgFloat(width,height);
    ImgFloat* sobelWeight = /*ImgFilters.*/ sobelFloatEdgesI(srcImg);
    int radius = (width+height)/500; if(radius<1) radius = 1;

    int   i,row,row0,row1,col,col0,col1;
    float w0,w1,w2,w3,w4,wMin,wMax;

    for(i=row=0;row<height;row++,i++) {
      row0 = (row>      0)?row-1:row+1;
      row1 = (row<width-1)?row+1:row-1;
      for(col=0;col<width;col++,i++) {
        col0 = (col>      0)?col-1:col+1;
        col1 = (col<width-1)?col+1:col-1;
        w0 = sobelWeight->get(col  ,row  );
        w1 = sobelWeight->get(col-1,row  );
        w2 = sobelWeight->get(col+1,row  );
        w3 = sobelWeight->get(col  ,row-1);
        w4 = sobelWeight->get(col  ,row+1);
        if(w0<0.5f) {
          wMin = w1;
          if(w2<wMin) wMin = w2;
          if(w3<wMin) wMin = w3;
          if(w4<wMin) wMin = w4;
          if(w0<wMin) w0 = wMin;
        } else {
          wMax = w1;
          if(w2>wMax) wMax = w2;
          if(w3>wMax) wMax = w3;
          if(w4>wMax) wMax = w4;
          if(w0>wMax) w0 = wMax;
        }
        sobelStrength->set(col,row,w0);
      }
    }
  // } catch(Exception e) {
  //   sobelStrength = null;
  // }
  return sobelStrength;
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_computeFaceCenters
(float fWidth, float fHeight,
 VecFloat& coord, VecInt& coordIndex, VecFloat& coordFace) {
  
  int   i,i0,i1,iF,iV;
  float x,y,n;
  
  coordFace.clear();
  for(iF=i0=i1=0;i1<coordIndex.size();i1++) {
    if(coordIndex[i1]>=0) continue;
    x = y = 0.0f;
    for(i=i0;i<i1;i++) {
      iV = coordIndex[i];
      x += fWidth*coord[2*iV  ];
      y += fHeight*coord[2*iV+1];
    }
    n = (float)(i1-i0); x /= n; y /= n;
    coordFace.append(x);
    coordFace.append(y);
    i0 = i1+1; iF++;
  }
}

//////////////////////////////////////////////////////////////////////
// computes best fitting rotated parallelogram

// private static
float ImgCheckerboards::_quadness19
(int N, PartitionLists& P, int i, ImgFloat& strength, float coord[/*8*/]) {
  float quadness = 0.0f;

  int width  = strength.getWidth();
  int height = strength.getHeight();

  if(N%2==1) N++; // need an even number

  double alpha,beta,gamma;
  float  meanWeight,varWeight,w,w0,w1,x0,y0,x,y,s,c,u,q,qMax,area,det;
  float  c01,s01,c12,s12,c23,s23,c30,s30;
  float  uMin,uMax,vMin,vMax,uMin30,uMax12,vMin01,vMax23;
  int    root,row,col,row1,col1,nBdry,h,hMin,k,kMin,nPix;

  bool touchesImageBoundary = false;

  // collect all corners of boundary pixels
  meanWeight = varWeight = x0 = y0 = 0.0f;
  root = P.find(i);
  VecFloat boundary;
  for(i=P.beg(root);i>=0;i=P.next(i)) {
    col = i%width;
    row = i/width;

    // don't include image boundary pixels ?
    if(col==0 || col==width-1 || row==0 || row==height-1)
      touchesImageBoundary = true;

    // (col-1,row  )
    w0 = strength.get(col,row);
    if((col1=col-1)<0 || P.find((col1)+(row  )*width)!=root) {
      x =      ((float)(col  )); 
      y = 0.5f+((float)(row  ));
        boundary.append(x);
        boundary.append(y);
      x0+=x; y0+=y;
      w1 = strength.get((col1<0)?0:col1,row);
      w = (w0>w1)?w1:w0;
      meanWeight += w;
      varWeight  += w*w;
    }
    // (col+1,row  )
    if((col1=col+1)>=width || P.find((col1)+(row  )*width)!=root) {
      x =      ((float)(col+1)); 
      y = 0.5f+((float)(row  ));
        boundary.append(x);
        boundary.append(y);
      x0+=x; y0+=y;
      w1 = strength.get((col1>=width)?width-1:col1,row);
      w = (w0>w1)?w1:w0;
      meanWeight += w;
      varWeight  += w*w;
    }
    // (col  ,row-1)
    if((row1=row-1)<0 || P.find((col  )+(row1)*width)!=root) {
      x = 0.5f+((float)(col  )); 
      y =      ((float)(row  ));
        boundary.append(x);
        boundary.append(y);
      x0+=x; y0+=y;
      w1 = strength.get(col,(row1<0)?0:row1);
      w = (w0>w1)?w1:w0;
      meanWeight += w;
      varWeight  += w*w;
    }
    // (col  ,row+1)
    if((row1=row+1)>=height || P.find((col  )+(row1)*width)!=root) {
      x = 0.5f+((float)(col  )); 
      y =      ((float)(row+1));
        boundary.append(x);
        boundary.append(y);
      x0+=x; y0+=y;
      w1 = strength.get(col,(row1>=height)?height-1:row1);
      w = (w0>w1)?w1:w0;
      meanWeight += w;
      varWeight  += w*w;
    }
  }
  nBdry = boundary.size()/2;
  x0 /= ((float)nBdry);
  y0 /= ((float)nBdry);
  meanWeight /= ((float)nBdry);
  varWeight  /= ((float)nBdry);
  varWeight -= meanWeight*meanWeight;

  // build the table of bounds as a function of angle
  //float bounds[3*N];
  std::vector<float> bounds(3 * N);
  for(h=0;h<N;h++) {
    alpha = ((double)h)*M_PI/((double)N);
    c   = (float)cos(alpha);
    s   = (float)sin(alpha);
    uMin = uMax = 0.0f;
    for(i=0;i<nBdry;i++) {
      x   = boundary[2*i  ]-x0;
      y   = boundary[2*i+1]-y0;
      u   =  c*x+s*y;
      if(i==0 || u<uMin) uMin = u;
      if(i==0 || u>uMax) uMax = u;
    }
    bounds[3*h  ] = uMin;
    bounds[3*h+1] = uMax;
    bounds[3*h+2] = (float)alpha;
  }

  // determine the two directions for which the intersection has
  // quadness factor 
  nPix = P.size(root);
  hMin = kMin = -1;
  qMax = 0.0f;
  for(h=0;h<N;h++) {
    uMin  = bounds[3*h  ];
    uMax  = bounds[3*h+1];
    alpha = bounds[3*h+2];
    for(k=0;k<N;k++) {
      if(k==h) continue;
      vMin = bounds[3*k  ];
      vMax = bounds[3*k+1];
      beta = bounds[3*k+2];
      gamma = beta-alpha-(float)(M_PI/2.0);
      area = (uMax-uMin)*(vMax-vMin)/((float)cos(gamma));
      q = ((float)nPix)/area; // 0 < quadness <= 1.0f
      if(hMin<0 || q>qMax) { hMin = h; kMin=k; qMax=q; }
    }
  }
  if(kMin<0) kMin+=N; else if(kMin>=N) kMin-=N;
  quadness = qMax;
  // if(touchesImageBoundary==false) quadness = qMax;
  // if(meanWeight<0.90f && varWeight<0.030f) quadness = qMax;
    
  // _log("hMin    = "+hMin);
  // _log("kMin    = "+kMin);
  // _log("qMax    = "+qMax);

  uMin30 = uMin  = bounds[3*hMin  ];
  uMax12 = uMax  = bounds[3*hMin+1];
  alpha = bounds[3*hMin+2];
  c30 = c12 = (float)cos(alpha);
  s30 = s12 = (float)sin(alpha);

  vMin01  = -bounds[3*kMin+1];
  vMax23  = -bounds[3*kMin  ];
  beta  = bounds[3*kMin+2];
  c23 = c01 =  (float)sin(beta);
  s23 = s01 = -(float)cos(beta);

  // equations of lines in implicit form
  // L_01 = { (x,y) : -s01*x+c01*y = vMin01 }
  // L_12 = { (x,y) :  c12*x+s12*y = uMax12 }
  // L_23 = { (x,y) : -s23*x+c23*y = vMax23 }
  // L_30 = { (x,y) :  c30*x+s30*y = uMin30 }

  // transform four corners of the quad from (u,v) to (x,y) space

  // q0 = L_30^L_01
  //  c30*x+s30*y = uMin30
  // -s01*x+c01*y = vMin01
  det = c30*c01+s30*s01;
  coord[0] = (c01*uMin30-s30*vMin01)/det+x0;
  coord[1] = (s01*uMin30+c30*vMin01)/det+y0;
  // _log("  q0 = (x,y)=("+x+","+y+") det="+det);

  // q1 = L_01^L_12
  //  c12*x+s12*y = uMax12
  // -s01*x+c01*y = vMin01
  det = c01*c12+s01*s12;
  coord[2] = (c01*uMax12-s12*vMin01)/det+x0;
  coord[3] = (s01*uMax12+c12*vMin01)/det+y0;
  // _log("  q1 = (x,y)=("+x+","+y+") det="+det);

  // q2 = L_12^L_23
  //  c12*x+s12*y = uMax12
  // -s23*x+c23*y = vMax23
  det = c12*c23+s12*s23;
  coord[4] = (c23*uMax12-s12*vMax23)/det+x0;
  coord[5] = (s23*uMax12+c12*vMax23)/det+y0;
  // _log("  q2 = (x,y)=("+x+","+y+") det="+det);

  // q3 = L_23^L_30
  //  c30*x+s30*y = uMin30
  // -s23*x+c23*y = vMax23
  det = c23*c30+s23*s30;
  coord[6] = (c23*uMin30-s30*vMax23)/det+x0;
  coord[7] = (s23*uMin30+c30*vMax23)/det+y0;
  // _log("  q3 = (x,y)=("+x+","+y+") det="+det);

  // _log(String.format("M = %12.6f V = %12.6f Q = %12.6f",
  //                    meanWeight,varWeight,quadness));

  return quadness;
}

//////////////////////////////////////////////////////////////////////
// private static

void ImgCheckerboards::_removeUnusedVertices
(VecFloat& coord, VecInt& coordIndex) {
  int nV = 0;
  if((nV=coord.size()/2)<=0) return;

  int     i,i0,i1,iF,iV,iVnew;
  bool deleteFace,hasBadFaces,hasUnusedVertices;
  float   x,y;

  VecBool used(nV,false);

  // first pass to delete faces with duplicate vertex indices and
  // vertex indices out of range
  hasBadFaces = false;
  for(iF=i0=i1=0;i1<coordIndex.size();i1++) {
    if(coordIndex[i1]>=0) continue;
    // mark used vertices and delete bad faces
    deleteFace = (i1-i0<3)?true:false;
    for(i=i0;i<i1;i++) {
      iV = coordIndex[i];
      if(iV>=nV || used[iV])
        hasBadFaces = deleteFace = true;
      else
        used[iV] = true;
    }
    for(i=i0;i<i1;i++) {
      iV = coordIndex[i];
      if(iV<nV && used[iV])
        used[iV] = false;
    }
    // face is not actually deleted, but
    // temporarilly replaced by (i1-i0) empty faces
    if(deleteFace)
      for(i=i0;i<i1;i++)
        coordIndex[i]=-1;
    // advance to next face
    i0 = i1+1; iF++;
  }
  // second pass to mark used vertices
  for(i=0;i<coordIndex.size();i++)
    if((iV=coordIndex[i])>=0) used[iV] = true;
  // determine if all vertices are used
  hasUnusedVertices = false;
  for(iV=0;hasUnusedVertices==false && iV<nV;iV++)
    if(used[iV]==false)
      hasUnusedVertices = true;
  if(hasUnusedVertices) {
    VecInt map(nV,-1);
    VecFloat newCoord;
    for(iV=0;iV<nV;iV++) {
      if(used[iV]) {
        iVnew = newCoord.size()/2;
        map[iV] = iVnew;
        x = coord[2*iV  ];
        y = coord[2*iV+1];
        newCoord.append(x);
        newCoord.append(y);
      }
    }
    VecInt newCoordIndex;
    for(iF=i0=i1=0;i1<coordIndex.size();i1++) {
      if(coordIndex[i1]>=0) continue;
      if((i1-i0)>=3) { // skip empty faces
        for(i=i0;i<i1;i++) {
          iV = coordIndex[i];
          iVnew = map[iV];
          newCoordIndex.append(iVnew);
        }
        newCoordIndex.append(-1);
      }
      // advance to next face
      i0 = i1+1; iF++;
    }
    coord.swap(newCoord);
    coordIndex.swap(newCoordIndex);
  } else if(hasBadFaces) {
    // each bad face has been replaced by one or more empty faces,
    // which have to be removed
    VecInt newCoordIndex;
    for(iF=i0=i1=0;i1<coordIndex.size();i1++) {
      if(coordIndex[i1]>=0) continue;
      if((i1-i0)>=3) { // skip empty faces
        for(i=i0;i<i1;i++) {
          iV = coordIndex[i];
          newCoordIndex.append(iV);
        }
        newCoordIndex.append(-1);
      }
      // advance to next face
      i0 = i1+1; iF++;
    }
    coordIndex.swap(newCoordIndex);
  }
}

// public static

// private static
void ImgCheckerboards::_setMinScale(float value) {
  _minScale = value;
}
// public static
void ImgCheckerboards::_setMaxScale(float value) {
  _maxScale = value;
}
// private static
float ImgCheckerboards::_scale(float value) {
  return
    (value<_minScale)?_minScale:
    (value>_maxScale)?_maxScale:
    255.0f*(value-_minScale)/(_maxScale-_minScale);
}

//////////////////////////////////////////////////////////////////////
ImgFloat* ImgCheckerboards::_coreFloatEdgesI(Img& srcImg, int mode) {
  ImgFloat* edges = (ImgFloat*)0;

  float f00,f01,f02,f10,f11,f12,f20,f21,f22,d1,d2,d3;
  int   width,height,x,y;
      
  width  = srcImg.getWidth();
  height = srcImg.getHeight();
  edges  = new ImgFloat(width,height);

  bool dx = (mode==0 || mode==2); 
  bool dy = (mode==1 || mode==2); 

  d1 = d2 = 0.0f;
  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      f00 = _scale((float)(srcImg.getI(x-1,y-1)));
      f01 = _scale((float)(srcImg.getI(x-1,y  )));
      f02 = _scale((float)(srcImg.getI(x-1,y+1)));
      f10 = _scale((float)(srcImg.getI(x  ,y-1)));
      f11 = _scale((float)(srcImg.getI(x  ,y  )));
      f12 = _scale((float)(srcImg.getI(x  ,y+1)));
      f20 = _scale((float)(srcImg.getI(x+1,y-1)));
      f21 = _scale((float)(srcImg.getI(x+1,y  )));
      f22 = _scale((float)(srcImg.getI(x+1,y+1)));
      // x derivative
      // -1*f00+0*f10+1*f20
      // -2*f01+0*f11+2*f21
      // -1*f02+0*f12+1*f22
      if(dx)
        d1 = ((f20-f00)+2.0f*(f21-f01)+(f22-f02))/1020.0f;
      // y derivative
      // -1*f00-2*f10-1*f20
      //  0*f01+0*f11+0*f21
      //  1*f02+2*f12+1*f22
      if(dy)
        d2 = ((f02-f00)+2.0f*(f12-f10)+(f22-f20))/1020.0f;
      //
      d3 = (float)sqrt(d1*d1+d2*d2);
      if(d3<0.0f) d3=0.0f; else if(d3>1.0f) d3=1.0f;
      edges->set(x,y,1.0f-d3); // edge iff value close to 1
    }
  }
  return edges;
}

ImgFloat* ImgCheckerboards::xFloatEdgesI(Img& srcImg) {
  return _coreFloatEdgesI(srcImg,0);
}

ImgFloat* ImgCheckerboards::yFloatEdgesI(Img& srcImg) {
  return _coreFloatEdgesI(srcImg,1);
}

ImgFloat* ImgCheckerboards::sobelFloatEdgesI(Img& srcImg) {
  return _coreFloatEdgesI(srcImg,2);
}

ImgFloat* ImgCheckerboards::sobelFloatEdgesIx(Img& srcImg) {
  ImgFloat* edges = (ImgFloat*)0;

  float f00,f01,f02,f10,f11,f12,f20,f21,f22,d1;
  int   width,height,x,y;
      
  width  = srcImg.getWidth();
  height = srcImg.getHeight();
  edges  = new ImgFloat(width,height);

  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      f00 = _scale((float)(srcImg.getI(x-1,y-1)));
      f01 = _scale((float)(srcImg.getI(x-1,y  )));
      f02 = _scale((float)(srcImg.getI(x-1,y+1)));
      f10 = _scale((float)(srcImg.getI(x  ,y-1)));
      f11 = _scale((float)(srcImg.getI(x  ,y  )));
      f12 = _scale((float)(srcImg.getI(x  ,y+1)));
      f20 = _scale((float)(srcImg.getI(x+1,y-1)));
      f21 = _scale((float)(srcImg.getI(x+1,y  )));
      f22 = _scale((float)(srcImg.getI(x+1,y+1)));
      // x derivative
      // -1*f00+0*f10+1*f20
      // -2*f01+0*f11+2*f21
      // -1*f02+0*f12+1*f22
      d1 = ((f20-f00)+2.0f*(f21-f01)+(f22-f02))/1020.0f;
      edges->set(x,y,d1); // edge iff value close to 1
    }
  }
  return edges;
}

ImgFloat* ImgCheckerboards::sobelFloatEdgesIy(Img& srcImg) {
  ImgFloat* edges = (ImgFloat*)0;

  float f00,f01,f02,f10,f11,f12,f20,f21,f22,d2;
  int   width,height,x,y;
      
  width  = srcImg.getWidth();
  height = srcImg.getHeight();
  edges  = new ImgFloat(width,height);

  for(x=0;x<width;x++) {
    for(y=0;y<height;y++) {
      f00 = _scale((float)(srcImg.getI(x-1,y-1)));
      f01 = _scale((float)(srcImg.getI(x-1,y  )));
      f02 = _scale((float)(srcImg.getI(x-1,y+1)));
      f10 = _scale((float)(srcImg.getI(x  ,y-1)));
      f11 = _scale((float)(srcImg.getI(x  ,y  )));
      f12 = _scale((float)(srcImg.getI(x  ,y+1)));
      f20 = _scale((float)(srcImg.getI(x+1,y-1)));
      f21 = _scale((float)(srcImg.getI(x+1,y  )));
      f22 = _scale((float)(srcImg.getI(x+1,y+1)));
      // y derivative
      // -1*f00-2*f10-1*f20
      //  0*f01+0*f11+0*f21
      //  1*f02+2*f12+1*f22
      d2 = ((f02-f00)+2.0f*(f12-f10)+(f22-f20))/1020.0f;
      edges->set(x,y,d2);
    }
  }
  return edges;
}

//////////////////////////////////////////////////////////////////////
// private static
bool ImgCheckerboards::_areSimilarSizes
(int size0, int size1, float factor) {
  bool value = false;
  if(size0>=25 && size1>=25) {
    int   diff  = (size0>size1)?size0-size1:size1-size0;
    int   size  = (size0+size1)/2;
    float ratio = ((float)diff)/((float)size);
    value = (ratio<factor);
  }
  return value;
}
