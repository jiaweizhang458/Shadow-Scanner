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

#include "CameraRecorder.hpp"

#include <QDir>
#include <QTime>
#include <QVariant>
#include <QApplication>

#include <cam/cam.hpp>
#include <img/util.hpp>
#include <img/yuv.hpp>

CameraRecorder::CameraRecorder(QObject * parent) :
  QThread(parent),
  _count(0),
  _interval(0),
  _path(),
  _namePattern("F_%03d_%01d.png"),
  _imageNames(),
  _rotation(0),
  _camera(),
  _cameraName(),
  _format(),
  _message(),
  _error(false),
  _skip(40), // number of initial frames to skip 
  _laserMode(1),
  _indexRun(0),
  _indexFirstImage(0),
  _timer(),
  _turntableMsec(30000),
  _turntableScanningPanel((TurntableScanningPanel*)0) {
  // qDebug(" -- CameraRecorder created -- ");
}

CameraRecorder::~CameraRecorder() {
  // qDebug(" -- CameraRecorder destroyed -- ");
}

void CameraRecorder::run() {

  _error = false;
  _imageNames.clear();
  _indexRun = -_skip;

  if(_turntableScanningPanel) {
    bool blank  = (_laserMode & 0x01)!=0;
    bool laser1 = (_laserMode & 0x02)!=0;
    // bool laser2 = (_laserMode & 0x04)!=0;
    int  laser  = (blank)?0:(laser1)?1:2;
    _turntableScanningPanel->sendTurntableCommand(QString("GF 0 %1 ").arg(laser),_turntableMsec);
  }

  emit started();

  if (_cameraName.isEmpty()) { //no camera
    stop(true, "Camera was not set");
    return;
  }

  //check if we have a camera
  if (_camera.isNull()) {
    stop(true, "Failed to get camera '" + _cameraName + "'");
    return;
  }

  //set camera format
  if (!_format.isEmpty()) {
    _camera->setCameraProperty(CAM_PROP_FORMAT, _format);
  }

  //connect
  connect(_camera.data(), &CameraInterface::newImage,
          this,           &CameraRecorder::on_camera_newImage, Qt::DirectConnection);

  //start the camera
  if (!_camera->start()) { //failed
    stop(true, "Failed to start camera '" + _cameraName + "'");
    return;
  }

  //start the image timer
  _timer.start();

  //start event loop
  exec();
}

void CameraRecorder::setCameraName(QString cameraName) { 
  _cameraName = cameraName; 
  if (!_cameraName.isEmpty()) {
    //get camera (do it here so that the camera is created in the caller's thread)
    _camera = cam::getCamera(_cameraName);
  }
}

void CameraRecorder::setTurntableScanningPanel(TurntableScanningPanel* panel) {
  _turntableScanningPanel = panel;
}

void CameraRecorder::on_camera_newImage(ImageBuffer const& image) {

  if (_indexRun<0) { //skip images
    ++_indexRun;
    return;
  }

  if (_indexRun >= _count) { //finished
    stop();
    return;
  }

  if (_interval && _timer.elapsed() < _interval) { //must wait, skip this image
    return;
  }

  qDebug("CameraRecorder::on_camera_newImage() {");
  qDebug("  _indexFirstImage = %d",_indexFirstImage);

  ImageBuffer outImage;
  if (image.format==ImageBuffer::NV12) {
    ImageBuffer rgbImage;
    img::rgbFromNV12(image, rgbImage);
    // image = rgbImage;
    img::rotateImage(rgbImage,0,outImage);
  } else {
    img::rotateImage(image,0,outImage);
  }
  if (_rotation!=0) {
    ImageBuffer rotatedImage;
    img::rotateImage(outImage, _rotation, rotatedImage);
    // image = rotatedImage;
    img::rotateImage(rotatedImage,0,outImage);
  }

  // compose command to move to teh next position nd send it to the turntable

  bool blank  = (_laserMode & 0x01)!=0;
  bool laser1 = (_laserMode & 0x02)!=0;
  bool laser2 = (_laserMode & 0x04)!=0;

  qDebug("  blank            = %d",blank);
  qDebug("  laser1           = %d",laser1);
  qDebug("  laser2           = %d",laser2);

  int imagesPerFrame = 0;
  if(blank ) imagesPerFrame++;
  if(laser1) imagesPerFrame++;
  if(laser2) imagesPerFrame++;
  if(imagesPerFrame==0) {
    imagesPerFrame = 1;
    blank = true;
    _laserMode = 1;
  }

  qDebug("  imagesPerFrame   = %d",imagesPerFrame);

  int  index     = _indexFirstImage+_indexRun;
  int  frame     = index/imagesPerFrame;
  int  indx      = index%imagesPerFrame;

  int  laser     = 0;
  switch(imagesPerFrame) {
  case 1: // indx==0
    laser = (laser2)?2:(laser1)?1:0;
    break;
  case 2: // indx==0 || indx==1
    // blank+laser1, blank+laser2, laser1+laser2 
    if(blank && laser1) {
      laser = (indx==0)?0:1;
    } else if(blank && laser2) {
      laser = (indx==0)?0:2;
    } else /* if(laser1 && laser2) */ {
      laser = (indx==0)?1:2;
    }
    break;
  case 3: // indx==0 || indx==1 || indx==2
    laser = indx;
    break;
  }

  qDebug("  index            = %d",index);
  qDebug("  frame            = %d",frame);
  qDebug("  laser            = %d",laser);

  // advance turntable to the next position and/or switch lasers on/off
  if(_turntableScanningPanel) {
    int F = frame;
    int L = laser;
    switch(imagesPerFrame) {
    case 1:
      /* if(indx==0) { */ F++; /* } */
      break;
    case 2:
      if(indx==1) { 
        if(blank && laser1) { // L==1
          L = 0;
        } else if(blank && laser2) { // L==2
          L = 0;
        } else /* if(laser1 && laser2) */ { // L==2
          L = 1;
        }
        F++;
      } else  { 
        if(blank && laser1) { // L==0
          L = 1;
        } else if(blank && laser2) { // L==0
          L = 2;
        } else /* if(laser1 && laser2) */ { // L==1
          L = 2;
        }
      } 
      break;
    case 3:
      if(++L==3) { F++; L = 0; }
      break;
    }

    QString cmd = QString("GF %1 %2 ").arg(F).arg(L);
      qDebug("turntable command \"%s\"",qPrintable(cmd));
    _turntableScanningPanel->sendTurntableCommand(cmd,_turntableMsec);
  }

  QDir dir(_path);
  QString basename = QString::asprintf(qPrintable(_namePattern), frame,laser);
  QString filename = dir.absoluteFilePath(basename);
  if (img::save(filename.toStdString(), outImage)) {
    _imageNames.append(basename);
    qDebug("saved image: %s", qPrintable(filename));
  } else {
    qDebug("saved image failed %d", index);
    stop(true, QString("Save image %1 failed").arg(index));
    return;
  }

  emit progress();
  _timer.restart();
  ++_indexRun;

  qDebug("}");
}

int CameraRecorder::getIndex() {
  return _indexFirstImage+_indexRun;
}

void CameraRecorder::setIndexFirstImage(int indexFirstImage) {
  _indexFirstImage = indexFirstImage;
}

void CameraRecorder::stop(bool error, QString message) {

  if (!_camera.isNull()) { //stop
    disconnect(_camera.data(), &CameraInterface::newImage,
               this,           &CameraRecorder::on_camera_newImage);
    QThread::msleep(100);
    _camera.clear();
  }

  _error = error;
  _message = message;

  // if(_turntableScanningPanel) {
  //   _turntableScanningPanel->sendTurntableCommand("S ",_turntableMsec);
  // }

  emit finished();
  quit();
}
