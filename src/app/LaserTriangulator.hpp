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

#ifndef _LaserTriangulator_hpp_
#define _LaserTriangulator_hpp_

#include <iostream>
#include <QThread>
#include <QFileInfoList>
#include <QImage>
#include <util/Vec.hpp>
#include <app/matrix_util.hpp>

class LaserTriangulator : public QThread
{
  Q_OBJECT;

private: // static

  static void _log(QString s) {
    std::cerr << "LaserTriangulator | " << s.toUtf8().constData() << std::endl;
  }
  
public:
  
  LaserTriangulator(QObject * parent = 0);
  ~LaserTriangulator();

  int                 count() const;
  int                 index() const;
  QString             fileName();
  QVector<Vector3d>&  worldCoord();

  void                setWorkDirectory(QString dir);
  void                stop(bool error = false, QString message = QString());
  void                setFileName(QString fileName);
  void                setLaserPlane1(Vector4d const & laserPlane1);
  void                setLaserPlane2(Vector4d const & laserPlane2);
  void                setWorldRotation(Matrix3d const & R);
  void                setWorldTranslation(Vector3d& T);
  void                setDegreesPerFrame(float deg);
  void setIntrinsicCameraParameters(Matrix3d const& K, Vector5d const& kc);

protected:

  void                run();
  
signals:
  void                started();
  void                progress();
  void                finished();
  
private: // variables

  QString                   _message;
  QString             _workDirectory;
  QFileInfoList            _fileList;
  QString                  _fileName;
  Vector4d              _laserPlane1;
  Vector4d              _laserPlane2;
  QImage               _backroundImg;
  int                         _index;
  bool                         _stop;
  QVector<Vector3d>      _worldCoord;
  Matrix3d            _worldRotation;
  Vector3d         _worldTranslation;
  float             _degreesPerFrame;
  Matrix3d                        _K;
  Vector5d                       _kc;

};

#endif  // _LaserTriangulator_hpp_
