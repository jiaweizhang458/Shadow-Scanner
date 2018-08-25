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

#ifndef _Image_hpp_
#define _Image_hpp_

#include <memory>
#include <functional>

namespace img
{
  //Image base class with minimal functionality (creates a NULL image)
  class Image
  {
  public:
    virtual size_t rows(void) const { return 0; };
    virtual size_t cols(void) const { return 0; };
    virtual const void * getPixelData(void) const { return nullptr; };

    inline bool isNull(void) const { return rows() == 0; }
  };

  //Typed Image
  template <typename PixelType>
  class TypedImage : public Image
  {
  public:
    virtual PixelType const& operator()(size_t col, size_t row) const = 0;
    virtual PixelType & operator()(size_t col, size_t row) = 0;
  };

  //Image stored row-by-row
  template <typename PixelType> 
  class RowImage : public TypedImage<PixelType>
  {
  public:
    virtual size_t getRowStride(void) const = 0;
    virtual PixelType * operator[](size_t row) = 0;
    virtual const PixelType * operator[](size_t row) const = 0;
  };

  template <typename PixelType> class RowImageImp : public RowImage <PixelType>
  {
    static const size_t DEFAULT_ALIGNMENT = 4;
    size_t _rows;
    size_t _cols;
    size_t _stride;
    std::shared_ptr<uint8_t> _data;
  public:
    RowImageImp(size_t rowNum = 0, size_t colNum = 0);

    //Image
    inline size_t rows(void) const { return _rows; }
    inline size_t cols(void) const { return _cols; }
    inline const void * getPixelData(void) const { return _data.get(); }

    //Typed Image
    PixelType const& operator()(size_t col, size_t row) const {
      return operator[](row)[col];
    }
    PixelType & operator()(size_t col, size_t row) {
      return operator[](row)[col];
    }

    //RowImage
    size_t getRowStride(void) const {
      return _stride;
    }
    PixelType * operator[](size_t row);
    const PixelType * operator[](size_t row) const;

    //methods
    RowImageImp<PixelType>
    fill(std::function<PixelType(size_t, size_t)> pixelValue); //mabe delete

    RowImageImp<PixelType>
    forEachPixel(std::function<void(PixelType&, size_t, size_t)> pixelOp);
    RowImageImp<PixelType>
    forEachPixel(std::function<void(PixelType const&, size_t, size_t)> pixelOp) const;
    // RowImageImp<PixelType>
    // forEachPixel(std::function<void(PixelType, size_t, size_t)> pixelOp) const;

  };

  typedef RowImageImp<uint8_t> Image1b;
  typedef RowImageImp<float> Image1f;

  enum FormatType {
    UnknownFormat = 0,
    GRAY,
    RGB24,
    RGB32,
    BGR24,
    BGR32,
    YUYV,
    JPEG,
    FormatTypeCount
  };

  int sizeOf(FormatType format);

}//namespace img

#endif //_Image_hpp_
