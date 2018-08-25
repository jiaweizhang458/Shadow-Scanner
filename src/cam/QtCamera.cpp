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

#include "QtCamera.hpp"

#include <QSize>
#include <QThread>
#include <QVariant>
#include <QMetaType>
#include <QStringList>
#include <QSharedPointer>

#include <QCameraInfo>
#include <iostream>

QMap<QString,QString> QtCamera::_globalCameraList;

QStringList QtCamera::getCameraList(void) {
  QMap<QString,QString> globalCameraList;

  //query cameras
  QList<QCameraInfo> list = QCameraInfo::availableCameras();
  foreach(auto info, list) {
    globalCameraList.insert(info.description(), info.deviceName());
  }

  //update global list
  _globalCameraList.swap(globalCameraList);

  return _globalCameraList.keys();
}

QSharedPointer<CameraInterface> QtCamera::getCamera(QString cameraName) {
  auto it = _globalCameraList.find(cameraName);
  if (it!=_globalCameraList.end()) { //found
    QtCamera * camera = new QtCamera(it.value(), it.key());
    if (camera && camera->isValid()) { //ok
      //TMP: REVIEW
      return QSharedPointer<CameraInterface>(camera, &QObject::deleteLater);
    }
    
    //error
    if (camera) {
      delete camera;
      camera = NULL;
    }
  }

  //not found
  log("[ERROR] Camera not found: %s\n", qPrintable(cameraName));
  return QSharedPointer<CameraInterface>();
}

QtCamera::QtCamera(QString deviceName, QString displayName) :
  _camera(deviceName.toLocal8Bit(), this),
  _displayName(displayName),
  _format(),
  _formatList(),
  _viewfinder() {
  connect(&_camera, &QCamera::stateChanged, this, &QtCamera::on_camera_stateChanged);
  start();
  stop();

#ifdef HAVE_IMG
  qRegisterMetaType<ImageBuffer>("ImageBuffer");
#endif //HAVE_IMG

  //info
  log("[INFO] Camera created: '%s' (%d formats)\n",
      qPrintable(_displayName), _formatList.size());
}

QtCamera::~QtCamera() {
  stop();
  _camera.unload();
  _viewfinder.reset();
  log("[INFO] Camera destroyed: %s\n", qPrintable(_displayName));
}

void QtCamera::on_camera_stateChanged(QCamera::State state) {
  if (state == QCamera::LoadedState) {
    //build the supported format list
    QStringList formatList;
    QList<QVideoFrame::PixelFormat> pixelFormatList =
      _camera.supportedViewfinderPixelFormats();
    foreach(auto format, pixelFormatList) {
      ImageBuffer::FormatType imgFmt = getImageBufferFormatType(format);
      if (imgFmt != ImageBuffer::UnknownFormat) {
        QCameraViewfinderSettings settings;
        settings.setPixelFormat(format);
        QList<QSize> resList = _camera.supportedViewfinderResolutions();

        auto fmtName = ImageBuffer().formatName(imgFmt);
        foreach(auto res, resList) {
          formatList.append
            (QString("%1:%2x%3").arg(fmtName).arg(res.width()).arg(res.height()));
        }
      }
    }

    //update the class list
    _formatList.swap(formatList);
  }

  if (state == QCamera::ActiveState) { //update the current format string
    auto settings = _camera.viewfinderSettings();
    auto resolution = settings.resolution();
    auto imgFmt = getImageBufferFormatType(settings.pixelFormat());
    auto fmtName = ImageBuffer().formatName(imgFmt);
    auto currFormat = QString("%1:%2x%3")
                      .arg(fmtName).arg(resolution.width()).arg(resolution.height());
    if (_format!=currFormat) { _format = currFormat; }
  }
}

bool QtCamera::start() {
  if (!_format.isEmpty() && _formatList.contains(_format.trimmed())) {
    QCameraViewfinderSettings settings;
    auto sf = _format.split(':');
    if (sf.size() > 1) {
      auto f = getPixelFormat(sf[0]);
      if (f != QVideoFrame::Format_Invalid) {
        settings.setPixelFormat(f);
      }
      auto sr = sf[1].split('x');
      if (sr.size() > 1) {
        int cols = sr[0].toInt();
        int rows = sr[1].toInt();
        settings.setResolution(cols, rows);
      }
    }
    _camera.setViewfinderSettings(settings);
  }

  /*QCameraViewfinderSettings settings;
  settings.setResolution(1920, 1080);
  _camera.setViewfinderSettings(settings);*/

  //setup a new viewfinder
  QScopedPointer<QtCameraViewfinder> viewfinder(new QtCameraViewfinder(*this));
  _camera.setViewfinder(viewfinder.data());
  _viewfinder.swap(viewfinder);

  //start the camera
  _camera.start();

  //wait
  auto state = _camera.state();
  for (int i = 0; i < 100 && state != QCamera::ActiveState; ++i) {
    QThread::msleep(10);
    state = _camera.state();
  }
  return (state == QCamera::ActiveState);
}

bool QtCamera::stopVideo(void) {
  _camera.stop();
  return true;
}

QStringList QtCamera::getPropertyList(void) {
  return QStringList() << CAM_PROP_FORMAT << CAM_PROP_EXPOSURE
                       << CAM_PROP_CONTRAST << CAM_PROP_BRIGHTNESS;
}

QVariant QtCamera::getCameraProperty(QString propertyName) {
  if (propertyName==CAM_PROP_FORMAT)     return QVariant(getFormat());
  if (propertyName==CAM_PROP_EXPOSURE)   return QVariant(getExposure());
  if (propertyName==CAM_PROP_CONTRAST)   return QVariant(getContrast());
  if (propertyName==CAM_PROP_BRIGHTNESS) return QVariant(getBrightness());
  log("[WARN] Unknow property: %s\n", qPrintable(propertyName));
  return QVariant();
}

void QtCamera::setCameraProperty(QString propertyName, QVariant value) {
  if (propertyName==CAM_PROP_FORMAT) {
    setFormat(value.toString());
  } else if (propertyName==CAM_PROP_EXPOSURE) {
    setExposure(value.toDouble());
  } else if (propertyName==CAM_PROP_CONTRAST) {
    setContrast(value.toDouble());
  } else if (propertyName==CAM_PROP_BRIGHTNESS) {
    setBrightness(value.toDouble());
  } else {
    log("[WARN] Unknow property: %s\n", qPrintable(propertyName));
  }
}

double QtCamera::getExposure(void) const {
  return 0.0;
  //if (!checkSourceFilter()) { return 0.0; }

  //convert to seconds
  //log("dshow getExposure: %lf\n",
  //    std::exp(static_cast<double>(value)*std::log(2.0)));
  //return std::exp(static_cast<double>(value)*std::log(2.0));
}

void QtCamera::setExposure(double value) {
  //if (!checkSourceFilter()) { return; }
  //log("dshow setExposure: %lf\n", value);
}

double QtCamera::getContrast(void) const {
  return 0.0;
  //if (!checkSourceFilter()) { return 0.0; }
  //log("Contrast %d\n", value);
  //scale to 0.0 - 1.0
  //return static_cast<double>(value+Min)/static_cast<double>(Max);
}

void QtCamera::setContrast(double value) {
  //if (!checkSourceFilter()) { return; }
}

double QtCamera::getBrightness(void) const {
  return 0.0;
  //if (!checkSourceFilter()) { return 0.0; }
  //log("Brightness %d\n", value);
  //scale to 0.0 - 1.0
  //return static_cast<double>(value+Min)/static_cast<double>(Max);
}

void QtCamera::setBrightness(double value) {
  //accepts values between 0.0 and 1.0
  //if (!checkSourceFilter()) { return; }
}

QString QtCamera::getFormat(void) const {
  return _format;
}

void QtCamera::setFormat(QString format) {
  _format = format;
}

#ifdef HAVE_IMG
ImageBuffer::FormatType
QtCamera::getImageBufferFormatType(QVideoFrame::PixelFormat videoFormat) {
  ImageBuffer::FormatType imgFmt = ImageBuffer::UnknownFormat;

  switch (videoFormat) {
  case QVideoFrame::Format_Invalid: break;
  case QVideoFrame::Format_ARGB32: break;
  case QVideoFrame::Format_ARGB32_Premultiplied: break;
  case QVideoFrame::Format_RGB32: imgFmt = ImageBuffer::RGB32; break;
  case QVideoFrame::Format_RGB24: imgFmt = ImageBuffer::RGB24; break;
  case QVideoFrame::Format_RGB565: break;
  case QVideoFrame::Format_RGB555: break;
  case QVideoFrame::Format_ARGB8565_Premultiplied: break;
  case QVideoFrame::Format_BGRA32: break;
  case QVideoFrame::Format_BGRA32_Premultiplied: break;
  case QVideoFrame::Format_BGR32: imgFmt = ImageBuffer::BGR32; break;
  case QVideoFrame::Format_BGR24: imgFmt = ImageBuffer::BGR24; break;
  case QVideoFrame::Format_BGR565: break;
  case QVideoFrame::Format_BGR555: break;
  case QVideoFrame::Format_BGRA5658_Premultiplied: break;

  case QVideoFrame::Format_AYUV444: break;
  case QVideoFrame::Format_AYUV444_Premultiplied: break;
  case QVideoFrame::Format_YUV444: break;
  case QVideoFrame::Format_YUV420P: break;
  case QVideoFrame::Format_YV12: break;
  case QVideoFrame::Format_UYVY: break;
  case QVideoFrame::Format_YUYV: imgFmt = ImageBuffer::YUV422; break;
  case QVideoFrame::Format_NV12: imgFmt = ImageBuffer::NV12; break;
  case QVideoFrame::Format_NV21: break;
  case QVideoFrame::Format_IMC1: break;
  case QVideoFrame::Format_IMC2: break;
  case QVideoFrame::Format_IMC3: break;
  case QVideoFrame::Format_IMC4: break;
  case QVideoFrame::Format_Y8: break;
  case QVideoFrame::Format_Y16: break;

  case QVideoFrame::Format_Jpeg: break;

  case QVideoFrame::Format_CameraRaw: break;
  case QVideoFrame::Format_AdobeDng: break;
          
  default:
    break;
  }

  return imgFmt;
}

QVideoFrame::PixelFormat QtCamera::getPixelFormat(QString format) {
  QVideoFrame::PixelFormat pixelFormat = QVideoFrame::Format_Invalid;
  if (format == "RGB32")
    pixelFormat = QVideoFrame::Format_RGB32;
  else if (format == "RGB24")
    pixelFormat = QVideoFrame::Format_RGB24;
  else if (format == "BGR32")
    pixelFormat = QVideoFrame::Format_BGR32;
  else if (format == "BGR24")
    pixelFormat = QVideoFrame::Format_BGR24;
  else if (format == "YUYV" || format == "YUV422")
    pixelFormat = QVideoFrame::Format_YUYV;
  else if (format == "NV12")
    pixelFormat = QVideoFrame::Format_NV12;
  else /* if (pixelFormat==QVideoFrame::Format_Invalid) */ {
    log("[WARN] format '%s'unknown\n", qPrintable(format));
  }
  return pixelFormat;
}
#endif //HAVE_IMG

//----------------------------------------------------------------------------------
// QtCameraViewfinder
//----------------------------------------------------------------------------------

QtCameraViewfinder::QtCameraViewfinder(QtCamera & camera) :
  _camera(camera) {
}

bool QtCameraViewfinder::present(const QVideoFrame & frame) {
  if (!frame.isValid()) { //invalid frame, ignore
    return false;
  }

#ifdef HAVE_IMG
  auto format = QtCamera::getImageBufferFormatType(frame.pixelFormat());
  if (format != ImageBuffer::UnknownFormat &&
      const_cast<QVideoFrame&>(frame).map(QAbstractVideoBuffer::ReadOnly)) {
    int rows = frame.height();
    int cols = frame.width();
    uint8_t * data = const_cast<uint8_t*>(frame.bits());
    int bytes = frame.mappedBytes();
    int stride = frame.bytesPerLine();
    
    ImageBuffer buffer;
    buffer.init2(rows, cols, format, data, stride, bytes, false);
    
    emit _camera.newImage(buffer);
    
    const_cast<QVideoFrame&>(frame).unmap();
  }
#endif //HAVE_IMG

  return false;
}

QList<QVideoFrame::PixelFormat>
QtCameraViewfinder::supportedPixelFormats
(QAbstractVideoBuffer::HandleType type) const {
  QList<QVideoFrame::PixelFormat> list;
  list << QVideoFrame::Format_RGB32 ;
  return list;
}
