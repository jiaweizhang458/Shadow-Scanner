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

#include "Image.hpp"

static img::Image1b EmptyImage1b;
static img::Image1f EmptyImage1f;

namespace img {
  void __instanciate(void)
  {
    EmptyImage1b.fill([](size_t, size_t) { return uint8_t(0); });
    EmptyImage1b.forEachPixel([](uint8_t, size_t, size_t) { });
    const_cast<const Image1b*>(&EmptyImage1b)->forEachPixel([](uint8_t const&, size_t, size_t) {});
    EmptyImage1f.fill([](size_t, size_t) { return float(0); });
    EmptyImage1f.forEachPixel([](float, size_t, size_t) {});
    const_cast<const Image1f*>(&EmptyImage1f)->forEachPixel([](float const&, size_t, size_t) {});
  }
}

/*static void NoDelete(void *)
{
  //do nothing
}*/

template <typename PixelType>
img::RowImageImp<PixelType>::RowImageImp(size_t rowNum, size_t colNum) :
  _rows(rowNum),
  _cols(colNum),
  _stride(DEFAULT_ALIGNMENT*((DEFAULT_ALIGNMENT+(colNum*sizeof(PixelType))-1)/DEFAULT_ALIGNMENT)),  //align rows
  _data()
{
  auto bytes = _rows*_stride*sizeof(PixelType);
  if (bytes)
  {
    fprintf(stderr, "[DEBUG] RowImageImp constructor allocating %zu bytes\n", bytes);
    _data.reset(new uint8_t[bytes], std::default_delete<uint8_t[]>());
  }
}

template <typename PixelType>
PixelType * img::RowImageImp<PixelType>::operator[](size_t row)
{
  return reinterpret_cast<PixelType*>(_data.get() + _stride*row);
}

template <typename PixelType>
const PixelType * img::RowImageImp<PixelType>::operator[](size_t row) const
{
  return reinterpret_cast<const PixelType*>(_data.get() + _stride*row);
}

template <typename PixelType>
img::RowImageImp<PixelType> img::RowImageImp<PixelType>::fill(std::function<PixelType(size_t, size_t)> pixelValue)
{
  for (size_t h = 0U; h < rows(); ++h)
  {
    auto * row = operator[](h);
    for (size_t w = 0U; w < cols(); ++w)
    {
      row[w] = pixelValue(w, h);
    }
  }
  return (*this);
}

template <typename PixelType>
img::RowImageImp<PixelType> img::RowImageImp<PixelType>::forEachPixel(std::function<void(PixelType&, size_t, size_t)> pixelOp)
{
  for (size_t h = 0U; h < rows(); ++h)
  {
    auto * row = operator[](h);
    for (size_t w = 0U; w < cols(); ++w)
    {
      pixelOp(row[w], w, h);
    }
  }
  return (*this);
}

template <typename PixelType>
img::RowImageImp<PixelType> img::RowImageImp<PixelType>::forEachPixel(std::function<void(PixelType const&, size_t, size_t)> pixelOp) const
{
  for (size_t h = 0U; h < rows(); ++h)
  {
    auto * row = operator[](h);
    for (size_t w = 0U; w < cols(); ++w)
    {
      pixelOp(row[w], w, h);
    }
  }
  return (*this);
}

int img::sizeOf(FormatType format)
{
  switch (format)
  {
  case GRAY:
    return 1;

  case BGR24: 
  case RGB24:
    return 3;

  case BGR32:
  case RGB32:
   return 4;
  
  case YUYV:
    //TODO
  
  default: 
    break;
  }
  return -1;
}

