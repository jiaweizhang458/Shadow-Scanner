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

#ifndef app_ChessboardCorners_hpp_
#define app_ChessboardCorners_hpp_

#include <QString>
#include <QVector>
#include <QSize>
#include <QMap>

#include "matrix_util.hpp"

#define CHESSBOARD_CORNERS_FILENAME "ChessboardCorners.mat"

class ChessboardCorners {

public:

  ChessboardCorners();

  void     clear(void);
  bool     isEmpty();
  unsigned imageCount();
  bool     hasValidTransform(unsigned indx);
  bool     hasValidTransforms();

  bool     loadMatlabCalib(QString filename, bool zero_RT);

  Vector2d& centerOfRotation();
  Matrix3d& worldRotation();
  Vector3d& worldTranslation();

  QVector<Matrix3d>&            R();
  QVector<Vector3d>&            T();
  QVector<QVector<Vector2d>>& wPt();
  QVector<QVector<Vector2d>>& iPt();

private:

  Vector2d  _centerOfRotation;
  Matrix3d  _worldRotation;
  Vector3d  _worldTranslation;

  QVector<Matrix3d>          _R;
  QVector<Vector3d>          _T;
  QVector<QVector<Vector2d>> _wPt;
  QVector<QVector<Vector2d>> _iPt;

};


#endif //app_ChessboardCorners_hpp_
