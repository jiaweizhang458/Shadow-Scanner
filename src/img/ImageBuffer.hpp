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

#ifndef __ImageBuffer_hpp__
#define __ImageBuffer_hpp__

#include <stdlib.h>
#include <memory>

namespace img 
{
  inline void NoDelete(void *)
  {
    //do nothing
  }
}

class ImageBuffer
{
public:
  int rows;
  int cols;
  int format;
  uint8_t * data;
  std::shared_ptr<uint8_t> sharedData;
  int bytes;
  int stride; //TODO

  enum FormatType {
        UnknownFormat=0, 
        BGR24, 
        BGR32, 
        RGB24, 
        RGB32, 
        YUV422, //deprecated use YUYV
        YUYV,
        NV12,
        JPEG, 
        FormatTypeCount
        };

  ImageBuffer(int rows=0, int cols=0, int format=0, uint8_t *data=0) :
    rows(rows),
    cols(cols),
    format(format),
    data(data),
    sharedData(),
    bytes(0),
    stride(cols*sizeOfFormat(format))
  {
  }

  void init(int rows=0, int cols=0, int format=0, unsigned char *data=0, int bytes=0, bool mustFree=false)
  {
    this->rows = rows;
    this->cols = cols;
    this->format = format;
    this->data = data;
    if (mustFree) { sharedData.reset(data); }
    this->bytes = bytes;
    this->stride = cols;
  }
  
  void init2(int rows=0, int cols=0, int format=0, unsigned char *data=0, int stride = 0, int bytes=0, bool mustFree=false)
  {
    this->rows = rows;
    this->cols = cols;
    this->format = format;
    this->data = data;
    if (mustFree) { sharedData.reset(data); }
    this->bytes = bytes;
    this->stride = stride;
  }

  const char * formatName(int f = -1) const
  {
    switch((f<0?format:f))
    {
    case BGR24: return "BGR24";
    case BGR32: return "BGR32";
    case RGB24: return "RGB24";
    case RGB32: return "RGB32";
    case YUV422: return "YUV422"; //deprecated
    case YUYV: return "YUVV";
    case NV12: return "NV12";
    case JPEG: return "JPEG";
    default: break;
    }
    return "UNKN";
  }
  
  static int sizeOfFormat(int format)
  {
    switch(format)
    {
      case BGR24: return 3;
      case BGR32: return 4;
      case RGB24: return 3;
      case RGB32: return 4;
      case YUV422: return 2; //deprecated
      case YUYV: return 2;
      case NV12: return 1;
        
      case JPEG:
      default: break;
    }
    return -1;
  }
};

#endif //__ImageBuffer_hpp__
