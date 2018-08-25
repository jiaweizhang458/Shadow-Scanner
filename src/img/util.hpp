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

#ifndef _util_hpp_
#define _util_hpp_

#include "Image.hpp"
#include "ImageBuffer.hpp"

#ifdef QT_GUI_LIB
class QString;
#endif //QT_GUI_LIB

namespace img
{
  bool save(std::string filename, Image const& image);
  bool save_pgm(std::string filename, Image const& image);

  bool save(std::string filename, ImageBuffer const& image);

#ifdef QT_GUI_LIB
  bool save(QString const& filename, ImageBuffer const& image);
  bool save_qimage(QString filename, Image const& image);
  bool save_qimage(QString filename, ImageBuffer const& image);
#endif //QT_GUI_LIB

  bool rotateImage(const ImageBuffer & image, size_t rotation, ImageBuffer & imageRotated);
  bool rotateRowImage(const ImageBuffer & image, size_t rotation, size_t pixelSize, ImageBuffer & imageRotated);

  bool flipVertical(ImageBuffer const& image, ImageBuffer & outImage);
  bool flipVerticalRowImage(ImageBuffer const& image, size_t pixelSize, ImageBuffer & outImage);

}//namespace img

#endif //_util_hpp_
