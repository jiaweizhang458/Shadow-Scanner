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

#ifndef _ImgGLWidget_hpp_
#define _ImgGLWidget_hpp_

#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QMutex>

#include <memory>

#ifdef HAVE_IMG
# include <img/ImageBuffer.hpp>
#endif //HAVE_IMG

class ImgGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT;

  static const size_t VERTEX_COUNT = 6;
  static const size_t VERTEX_COORD_OFFSET = 0;
  static const size_t VERTEX_COORD_TUPLE = 3;
  static const size_t VERTEX_COORD_STRIDE = 5 * sizeof(GLfloat);
  static const size_t TEXTURE_COORD_OFFSET = VERTEX_COORD_TUPLE*sizeof(GLfloat);
  static const size_t TEXTURE_COORD_TUPLE = 2;
  static const size_t TEXTURE_COORD_STRIDE = 5 * sizeof(GLfloat);

  static const GLfloat arrayBuffer[VERTEX_COUNT*(VERTEX_COORD_TUPLE + TEXTURE_COORD_TUPLE)];
  static const GLfloat * vertexArray;
  static const GLfloat * textureArray;

  std::unique_ptr<QOpenGLTexture> _texture;
  std::unique_ptr<QOpenGLShaderProgram> _program;
  std::unique_ptr<QOpenGLShaderProgram> _programColorize;

  GLfloat _rotation;
  QColor _clearColor;

  //tmp
#ifdef HAVE_IMG
  QMutex _mutex;
  ImageBuffer * _imageToLoad;
#endif //HAVE_IMG

  //void renderColorized(im::Image const& image, QOpenGLTexture * outTexture);

public:
  ImgGLWidget(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~ImgGLWidget();

public slots:
  void setRotation(float angle);
  void setQImage(QImage const& image);

#ifdef HAVE_IMG
  void setImage(ImageBuffer const& image);
  void loadTexture(ImageBuffer const& image); // only called by paintGL()
#endif //HAVE_IMG

  void clearImage(void);
  bool hasTexture(void);

protected:
  QOpenGLShaderProgram * loadProgram(QString basename) const;
  void initializeGL();
  void paintGL();
};

#endif //_ImgGLWidget_hpp_
