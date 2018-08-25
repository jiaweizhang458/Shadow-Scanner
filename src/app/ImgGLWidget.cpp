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

#include <iostream>

#include "ImgGLWidget.hpp"

#include <QThread>
#include <QOpenGLPixelTransferOptions>

#include <img/yuv.hpp>

const GLfloat ImgGLWidget::arrayBuffer[VERTEX_COUNT*(VERTEX_COORD_TUPLE + TEXTURE_COORD_TUPLE)] = {
  /* 1) vertex coord: */ 0.f, 0.f, 1.f, /* texture coord: */ 0.f, 0.f,
  /* 2) vertex coord: */ 1.f, 0.f, 1.f, /* texture coord: */ 1.f, 0.f,
  /* 3) vertex coord: */ 0.f, 1.f, 1.f, /* texture coord: */ 0.f, 1.f,
  /* 4) vertex coord: */ 0.f, 1.f, 1.f, /* texture coord: */ 0.f, 1.f,
  /* 5) vertex coord: */ 1.f, 0.f, 1.f, /* texture coord: */ 1.f, 0.f,
  /* 6) vertex coord: */ 1.f, 1.f, 1.f, /* texture coord: */ 1.f, 1.f
};

const GLfloat * ImgGLWidget::vertexArray = arrayBuffer;
const GLfloat * ImgGLWidget::textureArray = arrayBuffer + VERTEX_COORD_TUPLE;

ImgGLWidget::ImgGLWidget(QWidget * parent, Qt::WindowFlags f) :
  QOpenGLWidget(parent, f),
  QOpenGLFunctions(),
  _texture(),
  _program(),
  _programColorize(),
  _rotation(0.f),
  _clearColor(242, 242, 242, 255)
{
#ifdef HAVE_IMG
  _imageToLoad = nullptr;
#endif //HAVE_IMG
}

ImgGLWidget::~ImgGLWidget()
{
#ifdef HAVE_IMG
  if (_imageToLoad)
  {
    delete _imageToLoad;
  }
#endif //HAVE_IMG
}

void ImgGLWidget::setRotation(float angle)
{
  _rotation = angle;
}

void ImgGLWidget::setQImage(QImage const& image)
{
  // std::cerr << "ImgGLWidget::setQImage(QImage&)\n";
  std::unique_ptr<QOpenGLTexture> texture;
  texture.reset(new QOpenGLTexture(image, QOpenGLTexture::DontGenerateMipMaps));
  texture->setMinificationFilter(QOpenGLTexture::Linear);
  texture->setMagnificationFilter(QOpenGLTexture::Linear);
  //update
  _texture.swap(texture);
  update();
}

#ifdef _DIS
void ImgGLWidget::setImage(im::Image const& image)
{
  // std::cerr << "ImgGLWidget::setImage(im::Image&)\n";

  std::unique_ptr<QOpenGLTexture> texture;
  if (!image.isNull())
  { //copy the new image
    texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    //check we have a supported image format
    if (dynamic_cast<const im::Image1b*>(&image))
    {
      size_t rowStride = dynamic_cast<const im::Image1b*>(&image)->getRowStride();
      int alignment = 1;
      if (rowStride % 8 == 0) { alignment = 8; } 
      else if (rowStride % 4 == 0) { alignment = 4; }
      else if (rowStride % 2 == 0) { alignment = 2; }

      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(alignment);

      texture->create();
      texture->setSize(static_cast<int>(image.cols()), static_cast<int>(image.rows()));
      texture->setFormat(QOpenGLTexture::LuminanceFormat);
      texture->allocateStorage(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8);
      texture->setData(QOpenGLTexture::Luminance, QOpenGLTexture::UInt8, image.getPixelData(), &transferOptions);
      //qDebug("texture id %d", texture->textureId());
    }
    else if (dynamic_cast<const im::Image1f*>(&image))
    {
      size_t rowStride = dynamic_cast<const im::Image1f*>(&image)->getRowStride();
      int alignment = 1;
      if (rowStride % 8 == 0) { alignment = 8; }
      else if (rowStride % 4 == 0) { alignment = 4; }
      else if (rowStride % 2 == 0) { alignment = 2; }

      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(alignment);

      texture->create();
      texture->setSize(static_cast<int>(image.cols()), static_cast<int>(image.rows()));
      texture->setFormat(QOpenGLTexture::LuminanceFormat);
      texture->allocateStorage(QOpenGLTexture::Luminance, QOpenGLTexture::Float32);
      texture->setData(QOpenGLTexture::Luminance, QOpenGLTexture::Float32, image.getPixelData(), &transferOptions);
      //qDebug("texture id %d", texture->textureId());
    }
    /*else if (dynamic_cast<const im::Image1f*>(&image))
    { //float image, treat it as a decoded image and colorize it
      renderColorized(image, texture.get());

      //TODO !!!!!!!
    }*/
    else
    {
      qWarning("ImgGLWidget::setImage image format unsupported");
    }
  }

  //update
  _texture.swap(texture);
  update();
}
#endif //_DIS

#ifdef HAVE_IMG
void ImgGLWidget::setImage(ImageBuffer const& image)
{
  // std::cerr << "ImgGLWidget::setImage(ImageBuffer&)\n";

  /*if (this->thread()==QThread::currentThread())
  { //load image directly
    loadTexture(image);
  }
  else*/
  { //copy to a temporary buffer
    if (!_mutex.tryLock(50))
    { //couldn't lock: discard
      return;
    }
    
    if (!_imageToLoad)
    { //create image
      _imageToLoad = new ImageBuffer();
    }
    if (_imageToLoad->bytes<image.bytes)
    { //allocate new data
      _imageToLoad->data = new uint8_t[image.bytes];
      _imageToLoad->sharedData.reset(_imageToLoad->data);
      _imageToLoad->bytes = image.bytes;
    }
    //copy image
    _imageToLoad->rows = image.rows;
    _imageToLoad->cols = image.cols;
    _imageToLoad->format = image.format;
    _imageToLoad->stride = image.stride;
    memcpy(_imageToLoad->data, image.data, image.bytes);

    _mutex.unlock();
  }

  update();
}

// GT: only called by ImgGLWidget::paintGL()
void ImgGLWidget::loadTexture(ImageBuffer const& image)
{
  // std::cerr << "ImgGLWidget::loadTexture(ImageBuffer&)\n";

  std::unique_ptr<QOpenGLTexture> texture;
  if (image.data)
  { //copy the new image
    texture.reset(new QOpenGLTexture(QOpenGLTexture::Target2D));
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    //check we have a supported image format
    if (image.format==ImageBuffer::RGB24)
    {
      int alignment = 1;

      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(alignment);

      texture->create();
      texture->setSize(image.cols, image.rows);
      texture->setFormat(QOpenGLTexture::RGBFormat);
      texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
      texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, image.data, &transferOptions);
      //qDebug("texture id %d", texture->textureId());
    }
    else if (image.format==ImageBuffer::BGR24)
    {
      int alignment = 1;

      QOpenGLPixelTransferOptions transferOptions;
      transferOptions.setAlignment(alignment);

      texture->create();
      texture->setSize(image.cols, image.rows);
      texture->setFormat(QOpenGLTexture::RGBFormat);
      texture->allocateStorage(QOpenGLTexture::BGR, QOpenGLTexture::UInt8);
      texture->setData(QOpenGLTexture::BGR, QOpenGLTexture::UInt8, image.data, &transferOptions);
      //qDebug("texture id %d", texture->textureId());
    }
    else if (image.format == ImageBuffer::NV12)
    {
      ImageBuffer rgbImg;
      if (img::rgbFromNV12(image, rgbImg))
      {
        int alignment = 1;
        
        QOpenGLPixelTransferOptions transferOptions;
        transferOptions.setAlignment(alignment);
        
        texture->create();
        texture->setSize(rgbImg.cols, rgbImg.rows);
        texture->setFormat(QOpenGLTexture::RGBFormat);
        texture->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt8);
        texture->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt8, rgbImg.data, &transferOptions);
      }
    }
    else if (image.format == ImageBuffer::RGB32)
    {
      QImage qImg(image.data, image.cols, image.rows, QImage::Format_RGB32);
      texture.reset(new QOpenGLTexture(qImg, QOpenGLTexture::DontGenerateMipMaps));
      texture->setMinificationFilter(QOpenGLTexture::Linear);
      texture->setMagnificationFilter(QOpenGLTexture::Linear);
    }
    else if (image.format==ImageBuffer::YUV422)
    {
      //TODO move this to an image lib XXX

      //sanity check
      int expectedSize = 2 * image.cols * image.rows;
      if (image.bytes == expectedSize)
      {
        QImage qImg(image.cols, image.rows, QImage::Format_RGB32);
        for (int h = 0; h < image.rows; ++h)
        {
          uchar * qRow = qImg.scanLine(h);
          //const uchar * row = image.data + 2 * image.cols*h;
          const uchar * row = image.data + 2*image.cols*(image.rows-h-1); //reverse y
          for (int w = 0; w < image.cols; w += 2)
          {
            // Extract YUV components
            const uchar * yuv = row + 2 * w;

            auto y1 = yuv[0];
            auto u = yuv[1];
            auto y2 = yuv[2];
            auto v = yuv[3];

            int Cb = u - 128;
            int Cr = v - 128;
            int dr = Cr + (Cr >> 2) + (Cr >> 3) + (Cr >> 5);
            int dg = -((Cb >> 2) + (Cb >> 4) + (Cb >> 5)) - ((Cr >> 1) + (Cr >> 3) + (Cr >> 4) + (Cr >> 5));
            int db = Cb + (Cb >> 1) + (Cb >> 2) + (Cb >> 6);

            int r1 = y1 + dr, r2 = y2 + dr;
            int g1 = y1 + dg, g2 = y2 + dg;
            int b1 = y1 + db, b2 = y2 + db;

            if (r1 < 0) { r1 = 0; }
            if (r1 > 255) { r1 = 255; }
            if (g1 < 0) { g1 = 0; }
            if (g1 > 255) { g1 = 255; }
            if (b1 < 0) { b1 = 0; }
            if (b1 > 255) { b1 = 255; }

            if (r2 < 0) { r2 = 0; }
            if (r2 > 255) { r2 = 255; }
            if (g2 < 0) { g2 = 0; }
            if (g2 > 255) { g2 = 255; }
            if (b2 < 0) { b2 = 0; }
            if (b2 > 255) { b2 = 255; }

            uchar * q = qRow + 4 * w;
            q[0] = b1; q[1] = g1; q[2] = r1; q[3] = 0;
            q[4] = b2; q[5] = g2; q[6] = r2; q[7] = 0;
          }
        }

        texture.reset(new QOpenGLTexture(qImg, QOpenGLTexture::DontGenerateMipMaps));
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
      } //if expectedSize
    }
    else
    {
      //try to load from raw data
      QImage qImg;
      if (qImg.loadFromData(image.data, image.bytes))
      {
        texture.reset(new QOpenGLTexture(qImg, QOpenGLTexture::DontGenerateMipMaps));
        texture->setMinificationFilter(QOpenGLTexture::Linear);
        texture->setMagnificationFilter(QOpenGLTexture::Linear);
      }
      else {
        qWarning("ImgGLWidget::setImage image format unsupported");
      }
    }
  }

  //update
  _texture.swap(texture);
  update();
}
#endif //HAVE_IMG

void ImgGLWidget::clearImage(void)
{
  // std::cerr << "ImgGLWidget::clearImage()\n";
  std::unique_ptr<QOpenGLTexture> texture;
  _texture.swap(texture);
  update();
}

bool ImgGLWidget::hasTexture(void) {
  // std::cerr << "ImgGLWidget::hasTexture(void) {\n";
  bool value = false;
  if(_texture) {
    int width  = _texture->width();
    int height = _texture->height();
    // std::cerr << "  width  = " << width  << "\n";
    // std::cerr << "  height = " << height << "\n";
    // std::cerr << "}\n";
    value = (width*height>0);
  }
  return value;
}

QOpenGLShaderProgram * ImgGLWidget::loadProgram(QString basename) const
{
  //create program
  auto program = new QOpenGLShaderProgram();
  if (!program) {
    return nullptr;
  }

  if (!program->addShaderFromSourceFile(QOpenGLShader::Vertex, basename + ".vert"))
  { //compile failed
    qDebug("[ImgGLWidget::loadProgram] vertex shader error");
    qCritical("%s", qPrintable(program->log()));
    delete program;
    return nullptr;
  }

  if (!program->addShaderFromSourceFile(QOpenGLShader::Fragment, basename + ".frag"))
  { //compile failed
    qDebug("[ImgGLWidget::loadProgram] fragment shader error");
    qCritical("%s", qPrintable(program->log()));
    delete program;
    return nullptr;
  }

  if (!program->link())
  { //link failed
    qDebug("[ImgGLWidget::loadProgram] program link error");
    qCritical("%s", qPrintable(program->log()));
    delete program;
    return nullptr;
  }

  return program;
}

void ImgGLWidget::initializeGL()
{
  initializeOpenGLFunctions();
  
  qDebug("ImgGLWidget: %s", qPrintable(objectName()));
  qDebug("  [OpenGL] vendor: %s", glGetString(GL_VENDOR));
  qDebug("  [OpenGL] renderer: %s", glGetString(GL_RENDERER));
  qDebug("  [OpenGL] version: %s", glGetString(GL_VERSION));

  // Set up the rendering context, define display lists etc.
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glDisable(GL_DITHER);
  glEnable(GL_TEXTURE_2D);

  //programs
  _program.reset(loadProgram(":/Standard_vTexture"));
}

void ImgGLWidget::paintGL()
{
  if (!_program.get() || !_program->bind())
  { //error (paint red)
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return;
  }

#ifdef HAVE_IMG
  //load texture
  if (_imageToLoad && _imageToLoad->rows>0)
  {
    _mutex.lock();
    loadTexture(*_imageToLoad); // only place where this method is called
    _imageToLoad->rows = 0; //mark as loaded
    _mutex.unlock();
  }
#endif //HAVE_IMG

  //clear viewport
  glClearColor(_clearColor.redF(), _clearColor.greenF(), _clearColor.blueF(), _clearColor.alphaF());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  if (!_texture.get())
  { //no image to display
    return;
  }

  glBindTexture(GL_TEXTURE_2D, 0);
  _texture->bind();

  //projection matrix
  QMatrix4x4 projectionModelView;

  //aspect ratio
  GLfloat view_aspect = static_cast<float>(this->width()) / static_cast<float>(this->height());
  GLfloat img_aspect = static_cast<float>(_texture->width()) / static_cast<float>(_texture->height());
  GLfloat sx = 1.f, sy = 1.f;
  if ((_rotation>45.f && _rotation<135.f) || (_rotation>225.f && _rotation<315.f))
  { //invert the aspect ratio
    img_aspect = 1.f / img_aspect;
  }
  if (view_aspect <= img_aspect) { sy = view_aspect / img_aspect; }
  else { sx = img_aspect / view_aspect; }
  projectionModelView.scale(sx, sy);

  //rotation
  projectionModelView.rotate(_rotation, 0.f, 0.f, 1.f);

  //projection
  projectionModelView.ortho(0.f, 1.f, 1.f, 0.f, -1.f, 1.f);

  //setup shader params
  _program->setUniformValue("uProjectionModelView", projectionModelView);

  int aVertexLocation = _program->attributeLocation("aVertex");
  _program->enableAttributeArray(aVertexLocation);
  _program->setAttributeArray(aVertexLocation, vertexArray, VERTEX_COORD_TUPLE, VERTEX_COORD_STRIDE);

  int aTexCoordLocation = _program->attributeLocation("aTexCoord");
  _program->enableAttributeArray(aTexCoordLocation);
  _program->setAttributeArray(aTexCoordLocation, textureArray, TEXTURE_COORD_TUPLE, TEXTURE_COORD_STRIDE);

  int uTextureLocation = _program->uniformLocation("uTexture");
  _program->setUniformValue(uTextureLocation, 0); // 0 ==> GL_TEXTURE0

  //draw: no indices
  glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

  //unbind texture
  _texture->release();

  //deactivate shader
  _program->disableAttributeArray(aVertexLocation);
  _program->disableAttributeArray(aTexCoordLocation);
  _program->release();
}

#ifdef _DIS
void ImgGLWidget::renderColorized(im::Image const& image, QOpenGLTexture * outTexture)
{
  if (image.isNull() || !outTexture)
  { //error
    return;
  }

  //check we have a supported image format
  auto imgf = dynamic_cast<const im::Image1f*>(&image);
  if (!imgf)
  {
    qWarning("ImgGLWidget::renderColorized image format unsupported");
    return;
  }

  //make sure the context is current
  makeCurrent();
 
  //load program
  if (!_programColorize.get())
  { //init colorize program
    _programColorize.reset(loadProgram(":/Colorize"));
  }
  if (!_programColorize.get())
  { //error
    return;
  }

  //create input texture
  QOpenGLTexture inTexture(QOpenGLTexture::Target2D);
  inTexture.setMinificationFilter(QOpenGLTexture::Nearest);
  inTexture.setMagnificationFilter(QOpenGLTexture::Nearest);
  if (!inTexture.create())
  { //error
    return;
  }
  inTexture.setSize(static_cast<int>(image.cols()), static_cast<int>(image.rows()));
  inTexture.setFormat(QOpenGLTexture::LuminanceFormat);
  inTexture.allocateStorage(QOpenGLTexture::Luminance, QOpenGLTexture::Float32);
  inTexture.setData(QOpenGLTexture::Luminance, QOpenGLTexture::Float32, image.getPixelData());

  //create out texture
  if (!outTexture->create())
  { //error
    return;
  }
  outTexture->setSize(static_cast<int>(image.cols()), static_cast<int>(image.rows()));
  outTexture->setFormat(QOpenGLTexture::RGBAFormat);
  outTexture->allocateStorage();

  //create fboA and attach outTexture to it
  GLuint fboA;
  glGenFramebuffers(1, &fboA);
  glBindFramebuffer(GL_FRAMEBUFFER, fboA);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outTexture->textureId(), 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) 
  { //error
    qDebug("ImgGLWidget::renderColorized: failed to make complete framebuffer object %x", status);
    //delete fboA
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fboA);
    return;
  }

  // --- render ---

  //bind fboA to set outTexture as the output texture.
  glBindFramebuffer(GL_FRAMEBUFFER, fboA);

  //set the viewport to the texture size
  glViewport(0, 0, static_cast<int>(image.cols()), static_cast<int>(image.rows()));

  if (_programColorize->bind())
  {
    //clear viewport
    glClearColor(_clearColor.redF(), _clearColor.greenF(), _clearColor.blueF(), _clearColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, 0);
    inTexture.bind(0);

    //projection matrix
    QMatrix4x4 projectionModelView;

    //projection
    projectionModelView.ortho(0.f, 1.f, 0.f, 1.f, -1.f, 1.f);

    //setup shader params
    _programColorize->setUniformValue("uProjectionModelView", projectionModelView);

    int aVertexLocation = _programColorize->attributeLocation("aVertex");
    _programColorize->enableAttributeArray(aVertexLocation);
    _programColorize->setAttributeArray(aVertexLocation, vertexArray, VERTEX_COORD_TUPLE, VERTEX_COORD_STRIDE);

    int aTexCoordLocation = _programColorize->attributeLocation("aTexCoord");
    _programColorize->enableAttributeArray(aTexCoordLocation);
    _programColorize->setAttributeArray(aTexCoordLocation, textureArray, TEXTURE_COORD_TUPLE, TEXTURE_COORD_STRIDE);

    int uTextureLocation = _programColorize->uniformLocation("uTexture");
    _programColorize->setUniformValue(uTextureLocation, 0); // 0 ==> GL_TEXTURE0

    //draw: no indices
    glDrawArrays(GL_TRIANGLES, 0, VERTEX_COUNT);

    //unbind texture
    inTexture.release();

    //deactivate shader
    _programColorize->disableAttributeArray(aVertexLocation);
    _programColorize->disableAttributeArray(aTexCoordLocation);
    _programColorize->release();
  }
  else
  { //error (paint red)
    glClearColor(1.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  }

  //delete fboA
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fboA);
}
#endif //_DIS
