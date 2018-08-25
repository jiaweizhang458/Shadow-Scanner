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

#include "util.hpp"

#include <fstream>

#ifdef QT_GUI_LIB
# include <QImage>
# include <QString>
# include <QFileInfo>
#endif //QT_GUI_LIB

#include "yuv.hpp"

bool img::save(std::string filename, Image const& image) {
  auto pos = filename.find_last_of('.');
  if (pos == std::string::npos) { //not found, no extesion, unknown image type
    return false;
  }

  auto ext = filename.substr(pos + 1);

  if (ext == "pgm") {
    return save_pgm(filename, image);
  }

#ifdef QT_GUI_LIB
  if (ext == "png") {
    return img::save_qimage(QString::fromStdString(filename), image);
  }
#endif //QT_GUI_LIB

  fprintf(stderr, "[ERROR] im::save(%s,image) image format unsupported\n",
          filename.c_str());
  return false;
}

bool img::save_pgm(std::string filename, Image const& image) {
  //check we have a supported image format
  auto *pImg = dynamic_cast<const Image1b*>(&image);
  if (!pImg) {
    fprintf(stderr, "[ERROR] im::save_pgm(%s,image) image format unsupported\n",
            filename.c_str());
    return false;
  }

  //open the output file
  std::ofstream fout(filename, std::ios::binary);
  if (!fout.is_open()) {
    fprintf(stderr, "[ERROR] im::save_pgm(%s,image) file open failed\n",
            filename.c_str());
    return false;
  }

  //write header
  fout << "P5\n" << pImg->cols() << " " << pImg->rows() << " 255\n";

  //write pixels
  pImg->forEachPixel([&fout](uint8_t const&p, size_t, size_t) { fout.put(p); });

  //close file
  fout.close();

  fprintf(stderr, "[DEBUG] im::save_pgm(%s,image) file saved\n", filename.c_str());
  return true;
}

bool img::save(std::string filename, ImageBuffer const& image) {
#ifdef QT_GUI_LIB
  return img::save_qimage(QString::fromStdString(filename), image);
#endif //QT_GUI_LIB

  fprintf(stderr, "[ERROR] im::save(%s,image) image format unsupported\n",
          filename.c_str());
  return false;
}

#ifdef QT_GUI_LIB
bool img::save(QString const& filename, ImageBuffer const& image) {
  return save(filename.toStdString(), image);
}

bool img::save_qimage(QString filename, Image const& image) {
  //check we have a supported image format
  auto *pImg = dynamic_cast<const Image1b*>(&image);
  if (!pImg) {
    fprintf(stderr, "[ERROR] im::save_qimage(%s,image) image format unsupported\n",
            qPrintable(filename));
    return false;
  }

  //create a QImage
  uchar * data =
    const_cast<uchar*>(reinterpret_cast<const uchar*>(pImg->getPixelData()));
  int width = static_cast<int>(pImg->cols());
  int height = static_cast<int>(pImg->rows());
  int bytesPerLine = static_cast<int>(pImg->getRowStride());
  QImage qImg(data, width, height, bytesPerLine, QImage::Format_Grayscale8);

  //save
  if (qImg.save(filename)) { //saved
    fprintf(stderr, "[DEBUG] im::save_qimage(%s,image) file saved\n",
            qPrintable(filename));
    return true;
  }

  fprintf(stderr, "[ERROR] im::save_qimage(%s,image) file save FAILED\n",
          qPrintable(filename));
  return false;
}

bool img::save_qimage(QString filename, ImageBuffer const& image) {
  switch (image.format) {
  case ImageBuffer::RGB24:
    return QImage(image.data, image.cols, image.rows,
                  QImage::Format_RGB888).save(filename);
  case ImageBuffer::BGR24:
    return QImage(image.data, image.cols, image.rows,
                  QImage::Format_RGB888).rgbSwapped().save(filename);
  case ImageBuffer::RGB32:
    return QImage(image.data, image.cols, image.rows,
                  QImage::Format_RGB32).save(filename);
  case ImageBuffer::BGR32:
    return QImage(image.data, image.cols, image.rows,
                  QImage::Format_RGB32).rgbSwapped().save(filename);
  case ImageBuffer::NV12:
    {
      ImageBuffer rgbImg;
      if (rgbFromNV12(image, rgbImg)) {
        return QImage(rgbImg.data, rgbImg.cols, rgbImg.rows,
                      QImage::Format_RGB888).save(filename);
      }
      break;
    }
  case ImageBuffer::JPEG:
    if (QFileInfo(filename).suffix().toLower() == "jpg") { //save directly
      FILE * fd = fopen(qPrintable(filename), "wb"); if (!fd) { return false; }
      size_t written = fwrite(image.data, 1, image.bytes, fd); fclose(fd);
      return (written == static_cast<size_t>(image.bytes));
    } else {
      QImage qimg; return (qimg.loadFromData(image.data, image.bytes, "JPEG") ?
                           qimg.save(filename) : false);
    }
  default:
    break;
  }
  return false;
}

#endif //QT_GUI_LIB

bool img::rotateImage
(const ImageBuffer & image, size_t rotation, ImageBuffer & imageRotated) {
  size_t pixelSize = 0;

  switch (image.format) {
  case ImageBuffer::RGB24:
  case ImageBuffer::BGR24:
    pixelSize = 3; break;
  case ImageBuffer::RGB32:
  case ImageBuffer::BGR32:
    pixelSize = 4; break;
  default:
    pixelSize = 0; break;
  }

  if (pixelSize>0) {
    return rotateRowImage(image, rotation, pixelSize, imageRotated);
  }

  return false;
}

bool img::rotateRowImage
(const ImageBuffer & img_src, size_t rotation, size_t pixelSize,
 ImageBuffer & img_rot) {
  if (!img_src.data) { return false; }

  int bytes = static_cast<int>(pixelSize) * img_src.rows * img_src.cols;
  if (img_src.bytes<bytes) { //invalid image format
    return false;
  }

  unsigned char * data_rot = static_cast<unsigned char *>(malloc(bytes));
  if (!data_rot) { return false; }

  if (rotation == 0) { // copy
    img_rot.init(img_src.rows, img_src.cols, img_src.format, data_rot, bytes, true);
    for (int h = 0; h<img_rot.rows; ++h) {
      const unsigned char * row1 = img_src.data + pixelSize * h*img_src.cols;
      unsigned char * row2 = data_rot       + pixelSize * h*img_rot.cols;
      for (int w = 0; w<img_rot.cols; ++w) {
        const unsigned char * px1 = row1 + pixelSize * w;
        unsigned char * px2 = row2 + pixelSize * w;
        memcpy(px2, px1, pixelSize);
      }
    }
    return true;
  } else if (rotation == 90) {
    img_rot.init(img_src.cols, img_src.rows, img_src.format, data_rot, bytes, true);
    for (int h2 = 0, w1 = img_src.cols - 1; h2<img_rot.rows; ++h2, --w1) {
      unsigned char * row2 = data_rot + pixelSize * h2*img_rot.cols;
      for (int w2 = 0, h1 = 0; w2<img_rot.cols; ++w2, ++h1) {
        const unsigned char *
          px1 = img_src.data + pixelSize * (h1*img_src.cols + w1);
        unsigned char * px2 = row2 + pixelSize * w2;
        memcpy(px2, px1, pixelSize);
      }
    }
    return true;
  } else if (rotation == 180) {
    img_rot.init(img_src.rows, img_src.cols, img_src.format, data_rot, bytes, true);
    for (int h2 = 0, h1 = img_src.rows - 1; h2<img_rot.rows; ++h2, --h1) {
      const unsigned char * row1 = img_src.data + pixelSize * h1*img_src.cols;
      unsigned char * row2 = data_rot + pixelSize * h2*img_rot.cols;
      for (int w2 = 0, w1 = 0; w2<img_rot.cols; ++w2, ++w1) {
        const unsigned char * px1 = row1 + pixelSize * w1;
        unsigned char * px2 = row2 + pixelSize * w2;
        memcpy(px2, px1, pixelSize);
      }
    }
    return true;
  } else if (rotation == 270) {
    img_rot.init(img_src.cols, img_src.rows, img_src.format, data_rot, bytes, true);
    for (int h2 = 0, w1 = 0; h2<img_rot.rows; ++h2, ++w1) {
      unsigned char * row2 = data_rot + pixelSize * h2*img_rot.cols;
      for (int w2 = 0, h1 = img_src.rows - 1; w2<img_rot.cols; ++w2, --h1) {
        const unsigned char * px1 =
          img_src.data + pixelSize * (h1*img_src.cols + w1);
        unsigned char * px2 = row2 + pixelSize * w2;
        memcpy(px2, px1, pixelSize);
      }
    }
    return true;
  }

  return false;
}

bool img::flipVertical(ImageBuffer const& image, ImageBuffer & outImage) {
  size_t pixelSize = 0;

  switch (image.format) {
  case ImageBuffer::RGB24:
  case ImageBuffer::BGR24:
    pixelSize = 3; break;
  case ImageBuffer::RGB32:
  case ImageBuffer::BGR32:
    pixelSize = 4; break;
  default:
    pixelSize = 0; break;
  }

  if (pixelSize>0) {
    return flipVerticalRowImage(image, pixelSize, outImage);
  }

  return false;
}

bool img::flipVerticalRowImage(ImageBuffer const& image, size_t pixelSize,
                               ImageBuffer & outImage) {
  if (!image.data) { return false; }

  int bytes = static_cast<int>(pixelSize) * image.rows * image.cols;
  if (image.bytes<bytes) { //invalid image format
    return false;
  }

  if (!outImage.data || outImage.cols!=image.cols ||
      outImage.rows!=image.rows || outImage.format!=outImage.format) {
    outImage.init2(image.rows, image.cols, image.format,
                   new uint8_t[image.bytes], image.stride, image.bytes, true);
  }

  std::vector<uint8_t> p(pixelSize);
  for (int h = 0; h<image.rows; ++h) {
    const uint8_t * row1 = image.data + h * image.stride;
    //const uint8_t * row2 = image.data + (image.rows-h-1) * image.stride;
    uint8_t * outRow1 = outImage.data + h * outImage.stride;
    //uint8_t * outRow2 = outImage.data + (outImage.rows-h-1) * outImage.stride;
    for (int w = 0; w<image.cols; ++w) {
      const uint8_t * px1 = row1 + pixelSize * w;
      //const uint8_t * px2 = row2 + pixelSize * (image.cols-w-1);
      const uint8_t * px2 = row1 + pixelSize * (image.cols-w-1);
      
      uint8_t * outPx1 = outRow1 + pixelSize * w;
      //uint8_t * outPx2 = outRow2 + pixelSize * (outImage.cols-w-1);
      uint8_t * outPx2 = outRow1 + pixelSize * (outImage.cols-w-1);
      
      //swap pixels
      memcpy(p.data(), px1, pixelSize);
      memcpy(outPx1, px2, pixelSize);
      memcpy(outPx2, p.data(), pixelSize);
    }
  }
  
  return true;
}

/* swap channels
bool im_util::editImageChannels(const ImageBuffer & image, size_t c0, size_t c1,
                                size_t c2, ImageBuffer & imageOut) {
  if (!image.data) { return false; }
  if (image.format!=ImageBuffer::BGR24 && image.format!=ImageBuffer::RGB24) {
    return false;
  }

  if (!imageOut.data) { //create
    int bytes = 3*image.rows*image.cols;
    unsigned char * data = static_cast<unsigned char *>(malloc(bytes));
    if (!data) { //error
      return false;
    }
    imageOut.init(image.cols, image.rows, image.format, data, bytes, true);
  }

  for (int h=0; h<imageOut.rows; ++h) {
    const unsigned char * row1 = image.data + 3*h*image.cols;
    unsigned char * row2 = imageOut.data + 3*h*image.cols;
    for (int w=0; w<imageOut.cols; ++w) {
      const unsigned char * px1 = row1 + 3*w;
      unsigned char * px2 = row2 + 3*w;
      unsigned char px[] = {px1[0], px1[1], px1[2]}; //for in-place operation
      px2[0] = px[c0]; px2[1] = px[c1]; px2[2] = px[c2];
    }
  }

  return true;
}
*/
