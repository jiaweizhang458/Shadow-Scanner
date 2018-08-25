//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-13 18:40:33 taubin>
//------------------------------------------------------------------------
//
// WrlGLShader.hpp
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

#ifndef _WRL_GLSHADER_HPP
#define _WRL_GLSHADER_HPP

#include <QColor>
#include <QVector3D>
#include <QMatrix4x4>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include "WrlGLBuffer.hpp"

class WrlGLShader {

private:

  static const char *s_vsMaterial;
  static const char *s_vsMaterialNormal;
  static const char *s_vsColor;
  static const char *s_vsColorNormal;
  static const char *s_fsColor;

public:

  // constructor for IndexedFaceSet : lightSource!=(QVector3D*)0
  // constructor for IndexedLineSet : lightSource==(QVector3D*)0

  WrlGLShader(QColor& materialColor, QVector3D* lightSource=(QVector3D*)0);

  ~WrlGLShader();

  int            getNumberOfVertices();
  WrlGLBuffer*   getVertexBuffer() const;
  QMatrix4x4&    getMVPMatrix();

  void           setPointSize(float pointSize);
  void           setLineWidth(float lineWidth);
  void           setVertexBuffer(WrlGLBuffer* vb);
  void           setMVPMatrix(const QMatrix4x4& mvp);

  void           paint(QOpenGLFunctions& f);

private:

  QOpenGLShader        *_vshader;
  QOpenGLShader        *_fshader;
  QOpenGLShaderProgram *_program;

  int                   _pointSizeAttr;
  int                   _lineWidthAttr;
  int                   _vertexAttr;
  int                   _normalAttr;
  int                   _colorAttr;
  int                   _mvpMatrixAttr ;
  int                   _materialAttr;
  int                   _lightSourceAttr;

  WrlGLBuffer          *_vertexBuffer;
  QColor                _materialColor;
  QVector3D*            _lightSource;
  QMatrix4x4            _mvpMatrix; // viewport * projection * modelView
  float                 _pointSize;
  float                 _lineWidth;

};

#endif // _WRL_GLSHADER_HPP
