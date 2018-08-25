//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 21:33:36 taubin>
//------------------------------------------------------------------------
//
// WrlGLBuffer.hpp
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

#ifndef _WRL_GL_BUFFER_HPP_
#define _WRL_GL_BUFFER_HPP_

#include <QColor>
#include <QVector>
#include <QVector3D>
#include <QOpenGLBuffer>
#include "wrl/IndexedFaceSet.h"
#include "wrl/IndexedLineSet.h"

class WrlGLBuffer : public QOpenGLBuffer {

public:

  enum Type {
    MATERIAL, MATERIAL_NORMAL, COLOR, COLOR_NORMAL
  };

  WrlGLBuffer();
  WrlGLBuffer(IndexedFaceSet* pIfs, QColor& materialColor);
  WrlGLBuffer(IndexedLineSet* pIls, QColor& materialColor);

  Type     getType() const             { return                       _type; } 
  unsigned getNumberOfVertices() const { return                  _nVertices; }
  unsigned getNumberOfNormals()  const { return                   _nNormals; }
  unsigned getNumberOfColors()   const { return                    _nColors; }

  bool     hasFaces()            const { return                   _hasFaces; }
  bool     hasPolylines()        const { return               _hasPolylines; }
  bool     hasPoints()           const { return !(_hasFaces||_hasPolylines); }
  bool     hasColor()            const { return                   _hasColor; }
  bool     hasNormal()           const { return                  _hasNormal; }

protected:

  Type     _type;
  unsigned _nVertices;
  unsigned _nNormals;
  unsigned _nColors;
  bool     _hasFaces;
  bool     _hasPolylines;
  bool     _hasColor;
  bool     _hasNormal;

};

#endif // _WRL_GL_BUFFER_HPP_
