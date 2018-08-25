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

#ifndef _QtCamera_hpp_
#define _QtCamera_hpp_

#include "CameraInterface.hpp"

#include <QCamera>
#include <QAbstractVideoSurface>

#include <QMap>
#include <QString>
#include <QStringList>
#include <QScopedPointer>

#include "Log.hpp"

#ifdef HAVE_IMG
# include <img/ImageBuffer.hpp>
#endif //HAVE_IMG


class QtCameraViewfinder;

class QtCamera : public CameraInterface {

private:

  ADD_LOG;
  static const bool silent = false; //for log functions

  static QMap<QString,QString> _globalCameraList;

  QCamera     _camera;
  QString     _displayName;
  QString     _format;
  QStringList _formatList;

  QScopedPointer<QtCameraViewfinder> _viewfinder;

  bool stopVideo(void);

  QtCamera(QString deviceName, QString displayName = "QtCamera");

public:

  inline bool isValid(void) const { return (_camera.error()==QCamera::NoError); }

  //-- camera interface BEGIN ---------------------------------------------
  bool        start();
  inline void stop() { stopVideo(); }
  QStringList getPropertyList(void);
  QVariant    getCameraProperty(QString propertyName);
  void        setCameraProperty(QString propertyName, QVariant value);
  inline QStringList const&
              getFormatList(void) const { return _formatList; }
  //-- camera interface END -----------------------------------------------

  ~QtCamera();

  double      getExposure(void) const;
  void        setExposure(double value);
  double      getContrast(void) const;
  void        setContrast(double value);
  double      getBrightness(void) const;
  void        setBrightness(double value);
  QString     getFormat(void) const;
  void        setFormat(QString format);

  static QStringList getCameraList(void);
  static QSharedPointer<CameraInterface> getCamera(QString cameraName);
#ifdef HAVE_IMG
  static ImageBuffer::FormatType
  getImageBufferFormatType(QVideoFrame::PixelFormat videoFormat);
  static QVideoFrame::PixelFormat
  getPixelFormat(QString format);
#endif //HAVE_IMG

protected slots:
  void on_camera_stateChanged(QCamera::State state);
};

class QtCameraViewfinder : public QAbstractVideoSurface {
private:

  QtCamera & _camera;

public:

  QtCameraViewfinder(QtCamera & camera);
  bool present(const QVideoFrame & frame);
  QList<QVideoFrame::PixelFormat>
  supportedPixelFormats(QAbstractVideoBuffer::HandleType type =
                        QAbstractVideoBuffer::NoHandle) const;
};

#endif //_QtCamera_hpp_
