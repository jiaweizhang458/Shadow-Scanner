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

#ifndef _CameraRecorder_hpp_
#define _CameraRecorder_hpp_

#include <QThread>

//TMP: requires ImageBuffer: HAVE_IMG defined

#include <vector>

#include <QElapsedTimer>
#include <QSharedPointer>

#include <img/ImageBuffer.hpp>
#include <cam/CameraInterface.hpp>
#include <app/TurntableScanningPanel.hpp>

class CameraRecorder : public QThread
{
  Q_OBJECT;

private: // variables

  int                             _count;
  int                             _interval;
  QString                         _path;
  QString                         _namePattern;
  QStringList                     _imageNames;
  int                             _rotation;
  QSharedPointer<CameraInterface> _camera;
  QString                         _cameraName;
  QString                         _format;
  QString                         _message;
  bool                            _error;
  int                             _skip;
  int                             _laserMode;
  int                             _indexRun;
  int                             _indexFirstImage;
  QElapsedTimer                   _timer;
  int                             _turntableMsec;
  TurntableScanningPanel*         _turntableScanningPanel;

public: // private: // methods

  void    stop(bool error = false, QString message = QString());

public:

          CameraRecorder(QObject * parent = 0);
         ~CameraRecorder();

  void    setImageCount(int value) { _count = value; }
  void    setCaptureInterval(int value) { _interval = value; }
  void    setCameraName(QString cameraName);
  void    setCameraFormat(QString cameraFormat) { _format = cameraFormat; }
  void    setImageRotation(int angle) { _rotation = angle; }
  void    setImageName(QString namePattern) { _namePattern = namePattern; }
  void    setImagePath(QString path) { _path = path; }
  void    setTurntableScanningPanel(TurntableScanningPanel* panel);
  void    setLaserMode(int mode) { _laserMode = (mode<0)?0:(mode>8)?8:mode; }
  int     getImageCount(void) const { return _count; }
  int     getCaptureInterval(void) const { return _interval; }
  QString getCameraName(void) const { return _cameraName; }
  QString getCameraFormat(void) const { return _format; }
  int     getImageRotation(void) const { return _rotation; }
  QString getImageName(void) const { return _namePattern; }
  QString getImagePath(void) const { return _path; }
  int     getLaserMode() { return _laserMode; }
  int     getIndex();
  void    setIndexFirstImage(int indexFirstImage);

  QStringList const&
          getImageNameList(void) const { return _imageNames; }

  bool    getError(void) { return _error; }
  QString getErrorMessage(void) { return _message; }

protected:
  void    run();

protected slots:
  // signal sent by CameraInterface::newImage()
  void    on_camera_newImage(ImageBuffer const& image);

signals:
  void    started();
  void    progress();
  void    finished();
};

#endif  // _CameraRecorder_hpp_
