//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-13 18:42:33 taubin>
//------------------------------------------------------------------------
//
// WrlGLShader.cpp
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
#include "WrlGLShader.hpp"

const char *WrlGLShader::s_vsMaterial =
  "attribute highp vec4 vertex;\n"
  "uniform  mediump float pointsize;\n"
  "uniform  mediump float linewidth;\n"
  "uniform mediump mat4 mvpmatrix;\n"
  "uniform mediump vec4 matcolor;\n"
  "varying mediump vec4 color;\n"
  "void main(void) {\n"
  "  color = matcolor;\n"
  "  gl_Position = mvpmatrix * vertex;\n"
  "  gl_PointSize = pointsize;\n"
  // "  gl_LineWidth = linewidth;\n"
  "}\n";

const char *WrlGLShader::s_vsMaterialNormal =
  "attribute highp vec4 vertex;\n"
  "attribute mediump vec3 vnormal;\n"
  "uniform mediump float pointsize;\n"
  "uniform mediump float linewidth;\n"
  "uniform mediump mat4 mvpmatrix;\n"
  "uniform mediump vec4 matcolor;\n"
  "uniform mediump vec3 lightsource;\n"
  "varying mediump vec4 color;\n"
  "void main(void) {\n"
  "  vec3 toLight = normalize(lightsource);\n"
  "  float angle = max(dot(vnormal, toLight), 0.0);\n"
  "  vec3 col = vec3(matcolor);\n"
  "  color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
  "  color = clamp(color, 0.0, 1.0);\n"
  "  gl_Position = mvpmatrix * vertex;\n"
  "  gl_PointSize = pointsize;\n"
  // "  gl_LineWidth = linewidth;\n"
  "}\n";

const char *WrlGLShader::s_vsColor =
  "attribute highp vec4 vertex;\n"
  "attribute mediump vec4 vcolor;\n"
  "uniform mediump float pointsize;\n"
  "uniform mediump float linewidth;\n"
  "uniform mediump mat4 mvpmatrix;\n"
  "varying mediump vec4 color;\n"
  "void main(void) {\n"
  "  color = vcolor;\n"
  "  gl_Position = mvpmatrix * vertex;\n"
  "  gl_PointSize = pointsize;\n"
  // "  gl_LineWidth = linewidth;\n"
  "}\n";

const char *WrlGLShader::s_vsColorNormal =
  "attribute highp vec4 vertex;\n"
  "attribute mediump vec3 vnormal;\n"
  "attribute mediump vec4 vcolor;\n"
  "uniform mediump float pointsize;\n"
  "uniform mediump float linewidth;\n"
  "uniform mediump mat4 mvpmatrix;\n"
  "uniform mediump vec3 lightsource;\n"
  "varying mediump vec4 color;\n"
  "void main(void) {\n"
  "  vec3 toLight = normalize(lightsource);\n"
  "  float angle = max(dot(vnormal, toLight), 0.0);\n"
  "  vec3 col = vec3(vcolor);\n"
  "  color = vec4(col * 0.2 + col * 0.8 * angle, 1.0);\n"
  "  color = clamp(color, 0.0, 1.0);\n"
  "  gl_Position = mvpmatrix * vertex;\n"
  "  gl_PointSize = pointsize;\n"
  // "  gl_LineWidth = linewidth;\n"
  "}\n";

//////////////////////////////////////////////////////////////////////
const char *WrlGLShader::s_fsColor =
  "varying mediump vec4 color;\n"
  "void main(void) {\n"
  "  gl_FragColor = color;\n"
  "}\n";

//////////////////////////////////////////////////////////////////////
WrlGLShader::WrlGLShader(QColor& materialColor, QVector3D* lightSource):
  _vshader((QOpenGLShader*)0),
  _fshader((QOpenGLShader*)0),
  _program((QOpenGLShaderProgram*)0),
  _pointSizeAttr(-1),
  _lineWidthAttr(-1),
  _vertexAttr(-1),
  _normalAttr(-1),
  _colorAttr(-1),
  _mvpMatrixAttr(-1),
  _materialAttr(-1),
  _lightSourceAttr(-1),
  _vertexBuffer((WrlGLBuffer*)0),
  _materialColor(materialColor),
  _lightSource(lightSource),
  _pointSize(4.0f),
  _lineWidth(2.0f) {
  _mvpMatrix.setToIdentity();
}

//////////////////////////////////////////////////////////////////////
WrlGLShader::~WrlGLShader() {
  delete _program;
  delete _vshader;
  delete _fshader;
  if(_vertexBuffer==(WrlGLBuffer*)0) return;
  _vertexBuffer->destroy();
  delete _vertexBuffer;
  _vertexBuffer = (WrlGLBuffer*)0;
}

//////////////////////////////////////////////////////////////////////
void WrlGLShader::setPointSize(float pointSize) {
  _pointSize = (pointSize<0.0f)?0.0f:pointSize;
}

//////////////////////////////////////////////////////////////////////
void WrlGLShader::setLineWidth(float lineWidth) {
  _lineWidth = (lineWidth<0.0f)?0.0f:lineWidth;
}

//////////////////////////////////////////////////////////////////////
int WrlGLShader::getNumberOfVertices() {
  return (_vertexBuffer)?_vertexBuffer->getNumberOfVertices():0;
}

//////////////////////////////////////////////////////////////////////
QMatrix4x4& WrlGLShader::getMVPMatrix() {
  return _mvpMatrix;
}

//////////////////////////////////////////////////////////////////////
void WrlGLShader::setMVPMatrix(const QMatrix4x4& mvp) {
  _mvpMatrix = mvp;
}

//////////////////////////////////////////////////////////////////////
WrlGLBuffer* WrlGLShader::getVertexBuffer() const {
  return _vertexBuffer;
}

//////////////////////////////////////////////////////////////////////
void WrlGLShader::setVertexBuffer(WrlGLBuffer* vb) {

  _vertexBuffer = vb;
  if(_vertexBuffer==(WrlGLBuffer*)0) return;

  WrlGLBuffer::Type type = _vertexBuffer->getType();

  // int nVertices =  _vertexBuffer->getNumberOfVertices();
  // int nNormals  =  _vertexBuffer->getNumberOfNormals();
  // int nColors   =  _vertexBuffer->getNumberOfColors();

  // create the vertex shader
  _vshader = new QOpenGLShader(QOpenGLShader::Vertex);
  switch(type) {
  case WrlGLBuffer::Type::MATERIAL:
    _vshader->compileSourceCode(s_vsMaterial);
    break;
  case WrlGLBuffer::Type::MATERIAL_NORMAL:
    _vshader->compileSourceCode(s_vsMaterialNormal);
    break;
  case WrlGLBuffer::Type::COLOR:
    _vshader->compileSourceCode(s_vsColor);
    break;
  case WrlGLBuffer::Type::COLOR_NORMAL:
    _vshader->compileSourceCode(s_vsColorNormal);
    break;
  }

  // create the fragment shader
  _fshader = new QOpenGLShader(QOpenGLShader::Fragment);
  _fshader->compileSourceCode(s_fsColor);

  // create the shader program
  _program = new QOpenGLShaderProgram;
  _program->addShader(_vshader);
  _program->addShader(_fshader);
  _program->link();

  _pointSizeAttr       = -1;
  _lineWidthAttr       = -1;
  _vertexAttr          = -1;
  _normalAttr          = -1;
  _colorAttr           = -1;
  _mvpMatrixAttr       = -1;
  _materialAttr        = -1;
  _lightSourceAttr     = -1;

  _pointSizeAttr         = _program->uniformLocation("pointsize");
  _lineWidthAttr         = _program->uniformLocation("linewidth");
  _vertexAttr            = _program->attributeLocation("vertex");
  switch(type) {
  case WrlGLBuffer::Type::MATERIAL:
    _materialAttr        = _program->uniformLocation("matcolor");
    break;
  case WrlGLBuffer::Type::MATERIAL_NORMAL:
    _materialAttr        = _program->uniformLocation("matcolor");
    _normalAttr          = _program->attributeLocation("vnormal");
    break;
  case WrlGLBuffer::Type::COLOR:
    _colorAttr           = _program->attributeLocation("vcolor");
    break;
  case WrlGLBuffer::Type::COLOR_NORMAL:
    _colorAttr           = _program->attributeLocation("vcolor");
    _normalAttr          = _program->attributeLocation("vnormal");
    break;
  }
  _mvpMatrixAttr         = _program->uniformLocation("mvpmatrix");
  _lightSourceAttr       = _program->uniformLocation("lightsource");
}

//////////////////////////////////////////////////////////////////////
void WrlGLShader::paint(QOpenGLFunctions& f) {

  if(_vertexBuffer==(WrlGLBuffer*)0) return;

  WrlGLBuffer::Type type = _vertexBuffer->getType();

  _program->bind();

  _program->setUniformValue(_mvpMatrixAttr, _mvpMatrix);
  _program->setUniformValue(_lightSourceAttr, *_lightSource);

  _program->setUniformValue(_pointSizeAttr, _pointSize);
  _program->setUniformValue(_lineWidthAttr, _lineWidth);
  _program->enableAttributeArray(_vertexAttr);
  switch(type) {
  case WrlGLBuffer::Type::MATERIAL:
    _program->setUniformValue(_materialAttr, _materialColor);
    break;
  case WrlGLBuffer::Type::MATERIAL_NORMAL:
    _program->setUniformValue(_materialAttr, _materialColor);
    _program->enableAttributeArray(_normalAttr);
    break;
  case WrlGLBuffer::Type::COLOR:
    _program->enableAttributeArray(_colorAttr);
    break;
  case WrlGLBuffer::Type::COLOR_NORMAL:
    _program->enableAttributeArray(_colorAttr);
    _program->enableAttributeArray(_normalAttr);
    break;
  }
  
  _vertexBuffer->bind();

  switch(type) {
  case WrlGLBuffer::Type::MATERIAL:
    _program->setAttributeBuffer
      (_vertexAttr, GL_FLOAT,                 0, 3, 3*sizeof(GLfloat));
    break;
  case WrlGLBuffer::Type::MATERIAL_NORMAL:
    _program->setAttributeBuffer
      (_vertexAttr, GL_FLOAT,                 0, 3, 6*sizeof(GLfloat));
    _program->setAttributeBuffer
      (_normalAttr, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));
    break;
  case WrlGLBuffer::Type::COLOR:
    _program->setAttributeBuffer
      (_vertexAttr, GL_FLOAT,                 0, 3, 6*sizeof(GLfloat));
    _program->setAttributeBuffer
      ( _colorAttr, GL_FLOAT, 3*sizeof(GLfloat), 3, 6*sizeof(GLfloat));
    break;
  case WrlGLBuffer::Type::COLOR_NORMAL:
    _program->setAttributeBuffer
      (_vertexAttr, GL_FLOAT,                 0, 3, 9*sizeof(GLfloat));
    _program->setAttributeBuffer
      (_normalAttr, GL_FLOAT, 3*sizeof(GLfloat), 3, 9*sizeof(GLfloat));
    _program->setAttributeBuffer
      ( _colorAttr, GL_FLOAT, 6*sizeof(GLfloat), 3, 9*sizeof(GLfloat));
    break;
  }

  _vertexBuffer->release();

  int nVertices =  getNumberOfVertices();
  if(_vertexBuffer->hasFaces()) {
    f.glDrawArrays(GL_TRIANGLES, 0, nVertices);
  } else if(_vertexBuffer->hasPolylines()) {
    // TODO : move lineWidth to the vertex shader
    // glLineWidth(_lineWidth);
    f.glDrawArrays(GL_LINES, 0, nVertices);
  } else {
    f.glDrawArrays(GL_POINTS, 0, nVertices);
  }

  _program->disableAttributeArray(_vertexAttr);
  switch(type) {
  case WrlGLBuffer::Type::MATERIAL:
    break;
  case WrlGLBuffer::Type::MATERIAL_NORMAL:
    _program->disableAttributeArray(_normalAttr);
    break;
  case WrlGLBuffer::Type::COLOR:
    _program->disableAttributeArray(_colorAttr);
    break;
  case WrlGLBuffer::Type::COLOR_NORMAL:
    _program->disableAttributeArray(_colorAttr);
    _program->disableAttributeArray(_normalAttr);
    break;
  }

  _program->release();
}
