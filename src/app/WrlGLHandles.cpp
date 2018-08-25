//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-05 21:34:19 taubin>
//------------------------------------------------------------------------
//
// WrlGLHandles.cpp
//
// Software developed for the Fall 2015 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2015, Gabriel Taubin
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
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>
#include "WrlGLHandles.hpp"

//////////////////////////////////////////////////////////////////////
WrlGLHandles::WrlGLHandles():
  _vshader((QOpenGLShader*)0),
  _fshader((QOpenGLShader*)0),
  _program((QOpenGLShaderProgram*)0),
  _vertexAttr(-1),
  _matrixAttr(-1),
  _materialAttr(-1),
  _buffer((QOpenGLBuffer*)0) {
  _matrix.setToIdentity();
}

//////////////////////////////////////////////////////////////////////
WrlGLHandles::~WrlGLHandles() {
  delete _program;
  delete _vshader;
  delete _fshader;
  if(_buffer==(QOpenGLBuffer*)0) return;
  _buffer->destroy();
  delete _buffer;
  _buffer = (QOpenGLBuffer*)0;
}

//////////////////////////////////////////////////////////////////////
QMatrix4x4& WrlGLHandles::getMatrix() {
  return _matrix;
}

void WrlGLHandles::setColor(QColor& materialColor) {
    _materialColor = materialColor;
}

//////////////////////////////////////////////////////////////////////
void WrlGLHandles::setGeometry
  (float hx0, float hy0, float hx1, float hy1) {

  const char *vShaderCode =
    "attribute highp vec4 vertex;\n"
    "uniform mediump mat4 matrix;\n"
    "uniform mediump vec4 matcolor;\n"
    "varying mediump vec4 color;\n"
    "void main(void) {\n"
    "  color = matcolor;\n"
    "  gl_Position = matrix * vertex;\n"
    "}\n";

  const char *fShaderCode =
    "varying mediump vec4 color;\n"
    "void main(void) {\n"
    "  gl_FragColor = color;\n"
    "}\n";

  const int nVertices = 6;
  _buffer = new QOpenGLBuffer();
  _buffer->create();
  _buffer->bind();
  QVector<GLfloat> buf;
  buf.resize(3*nVertices);
  GLfloat *p = buf.data();

  // (1,0) --- (1,1)
  //   |    \    |
  // (0,0) --- (0,1)

  // triangle 1
  //     glVertex3f(hx0,hy0,0.0f);
  *p++ = hx0; *p++ = hy0; *p++ = 0.0f;
  //     glVertex3f(hx0,hy1,0.0f);
  *p++ = hx0; *p++ = hy1; *p++ = 0.0f;
  //     glVertex3f(hx1,hy0,0.0f);
  *p++ = hx1; *p++ = hy0; *p++ = 0.0f;
  // triangle 2
  //     glVertex3f(hx1,hy1,0.0f);
  *p++ = hx1; *p++ = hy1; *p++ = 0.0f;
  //     glVertex3f(hx1,hy0,0.0f);
  *p++ = hx1; *p++ = hy0; *p++ = 0.0f;
  //     glVertex3f(hx0,hy1,0.0f);
  *p++ = hx0; *p++ = hy1; *p++ = 0.0f;

  _buffer->allocate(buf.constData(), buf.count() * sizeof(GLfloat));
  _buffer->release();

  // create the vertex shader
  _vshader = new QOpenGLShader(QOpenGLShader::Vertex);
  _vshader->compileSourceCode(vShaderCode);

  // create the fragment shader
  _fshader = new QOpenGLShader(QOpenGLShader::Fragment);
  _fshader->compileSourceCode(fShaderCode);

  // create the shader program
  _program = new QOpenGLShaderProgram;
  _program->addShader(_vshader);
  _program->addShader(_fshader);
  _program->link();

  _vertexAttr   = _program->attributeLocation("vertex");
  _materialAttr = _program->uniformLocation("matcolor");
  _matrixAttr   = _program->uniformLocation("matrix");
}

//////////////////////////////////////////////////////////////////////
void WrlGLHandles::paint(QOpenGLFunctions& f) {

  if(_buffer==(QOpenGLBuffer*)0) return;

  _program->bind();
  _program->setUniformValue(_matrixAttr, _matrix);
  _program->setUniformValue(_materialAttr, _materialColor);
  _program->enableAttributeArray(_vertexAttr);
  _buffer->bind();
  _program->setAttributeBuffer(_vertexAttr, GL_FLOAT, 0, 3, 3*sizeof(GLfloat));
  _buffer->release();

  f.glDrawArrays(GL_TRIANGLES, 0, 6);

  _program->disableAttributeArray(_vertexAttr);
  _program->release();

}
