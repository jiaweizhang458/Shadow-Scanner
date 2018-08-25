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

#include "yuv.hpp"

//Info about YUV formats:
//    https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750%28v=vs.85%29.aspx

namespace img
{
  inline uint8_t clamp(int v)
  {
    if (v<000) { return 000; }
    if (v>255) { return 255; }
    return static_cast<uint8_t>(v);
  }
  
  inline void rgbPixelFromYuv(int y1, int y2, int u, int v, uint8_t * rgb1, uint8_t * rgb2)
  {
    auto C1 = 298 * (y1 - 16);
    auto C2 = 298 * (y2 - 16);
    auto D  = u - 128;
    auto E  = v - 128;
    auto dr = (           + 409 * E + 128);
    auto dg = ( - 100 * D - 208 * E + 128);
    auto db = (   516 * D           + 128);
    rgb1[0] = clamp((C1 + dr) >> 8);
    rgb1[1] = clamp((C1 + dg) >> 8);
    rgb1[2] = clamp((C1 + db) >> 8);
    rgb2[0] = clamp((C2 + dr) >> 8);
    rgb2[1] = clamp((C2 + dg) >> 8);
    rgb2[2] = clamp((C2 + db) >> 8);
  }
}

bool img::rgbFromYuv(ImageBuffer const& yuvImg, ImageBuffer & rgbImg)
{
  switch (yuvImg.format)
  {
    case ImageBuffer::NV12:
      return rgbFromNV12(yuvImg, rgbImg);
    default:
      break;
  }
  return false;
}

bool img::rgbFromNV12(ImageBuffer const& yuvImg, ImageBuffer & rgbImg)
{
  if (yuvImg.format!=ImageBuffer::NV12 || !yuvImg.data)
  { //error
    return false;
  }
  if (yuvImg.bytes<(yuvImg.rows+yuvImg.rows/2)*yuvImg.stride)
  { //error
    return false;
  }
  
  //NV12 is a planar YUV 4:2:0 format
  // +-----------------------------------+
  // | Y0 Y1 ...                         |
  // |       ...                         |
  // |       ...                         |
  // +-----------------------------------+
  // | U0 V0 U1 V1 ...                   |
  // |             ...                   |
  // +-----------------------------------+
  
  //A format in which all Y samples are found first in memory as an array of unsigned char with an even number of lines (possibly with a larger stride for memory alignment). This is followed immediately by an array of unsigned char containing interleaved U(Cb) and V(Cr) samples. If these samples are addressed as a little-endian WORD type, U(Cb) would be in the least significant bits and V(Cr) would be in the most significant bits with the same total stride as the Y samples. NV12 is the preferred 4:2:0 pixel format.
  
  int rows = yuvImg.rows;
  int cols = yuvImg.cols;
  int alignedRows = 2*((rows+1)/2);
  const uint8_t * yData = yuvImg.data;
  const uint8_t * uvData = yuvImg.data + alignedRows * yuvImg.stride;
  
  size_t stride = 3*cols;
  size_t bytes = stride*rows;
  uint8_t * data = new uint8_t[bytes];
  if (!data) {
    return false;
  }
  
  auto halfRows = (rows>>1); // = rows/2
  
  for (int h=0; h<rows; /*++h*/ h+=2)
  {
    uint8_t * row0 = data + (h+0)*stride;
    const uint8_t * yRow0 = yData + (h+0)*yuvImg.stride;
    
    uint8_t * row1 = nullptr;
    const uint8_t * yRow1 = nullptr;
    if (h+1<rows) {
      row1 = data + (h+1)*stride;
      yRow1 = yData + (h+1)*yuvImg.stride;
    } else {
      row1 = row0;
      yRow1 = yRow0;
    }
    
    auto h2 = (h >> 1); // = (h/2)
    const uint8_t * uvRowCurr = uvData + (h2+0)*yuvImg.stride;
    
    const uint8_t * uvRowPrev = nullptr;
    if (h2-1<0) { uvRowPrev = uvRowCurr;                     }
    else        { uvRowPrev = uvData + (h2-1)*yuvImg.stride; }
    
    const uint8_t * uvRowNext = nullptr;
    if (h2+1<halfRows) { uvRowNext = uvData + (h2+1)*yuvImg.stride; }
    else               { uvRowNext = uvRowCurr;                     }
    
    const uint8_t * uvRowPost = nullptr;
    if (h2+2<halfRows) { uvRowPost = uvData + (h2+1)*yuvImg.stride; }
    else               { uvRowPost = uvRowNext;                     }
    
    for (int w=0; w<cols; /*++w*/ w+=2)
    {

      auto w0 = w+0;
      auto w1 = w+1;
      
      auto y00 = yRow0[w0];
      auto y01 = yRow0[w1];
      auto y10 = yRow1[w0];
      auto y11 = yRow1[w1];
      
      auto uPrev = uvRowPrev[w0];
      auto uCurr = uvRowCurr[w0];
      auto uNext = uvRowNext[w0];
      auto uPost = uvRowPost[w0];
      
      auto vPrev = uvRowPrev[w1];
      auto vCurr = uvRowCurr[w1];
      auto vNext = uvRowNext[w0];
      auto vPost = uvRowPost[w0];
      
      auto u0 = uCurr;
      auto v0 = vCurr;
      
      //Cout[2*i+1] = clip((9 * (Cin[i] + Cin[i+1]) - (Cin[i-1] + Cin[i+2]) + 8) >> 4);
      auto u1 = clamp((9 * (uCurr + uNext) - (uPrev + uPost) + 8) >> 4);
      auto v1 = clamp((9 * (vCurr + vNext) - (vPrev + vPost) + 8) >> 4);
      
      auto w3 = 3*w;
      auto rgb00 = row0 + (w3+0);
      auto rgb01 = row0 + (w3+3);
      auto rgb10 = row1 + (w3+0);
      auto rgb11 = row1 + (w3+3);
      
      rgbPixelFromYuv(y00, y01, u0, v0, rgb00, rgb01);
      rgbPixelFromYuv(y10, y11, u1, v1, rgb10, rgb11);
    }
  }
  
  rgbImg.init2(rows, cols, ImageBuffer::RGB24, data, static_cast<int>(stride), static_cast<int>(bytes), true);

  return true;
}