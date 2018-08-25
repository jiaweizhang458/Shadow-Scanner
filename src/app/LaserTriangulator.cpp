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

#include <iostream>
#include <QDir>
#include "LaserTriangulator.hpp"
#include <util/LineDetector6.hpp>
#include <img/ImgArgb.hpp>
#include <img/ImgCheckerboards.hpp>
#include <img/ImgDraw.hpp>
#include <homework/homework4.hpp>

LaserTriangulator::LaserTriangulator(QObject * parent):
  QThread(parent),
  _message(),
  _workDirectory(),
  _fileList(),
  _fileName(),
  _laserPlane1(),
  _laserPlane2(),
  _backroundImg(),
  _index(0),
  _stop(false),
  _worldCoord(),
  _worldRotation(),
  _worldTranslation(),
  _degreesPerFrame(0.0f) {
}

LaserTriangulator::~LaserTriangulator() {
}

int LaserTriangulator::index() const {
  return _index;
}

void LaserTriangulator::setWorkDirectory(QString dir) {
  _workDirectory = dir;
}

int LaserTriangulator::count() const {
  return _fileList.size();
}

QString LaserTriangulator::fileName() {
  return _fileName;
}

void LaserTriangulator::setFileName(QString fileName) {
  _fileName = fileName;
}

void LaserTriangulator::setLaserPlane1(Vector4d const & laserPlane1) {
  _laserPlane1 = laserPlane1;
}

void LaserTriangulator::setLaserPlane2(Vector4d const & laserPlane2) {
  _laserPlane2 = laserPlane2;
}

QVector<Vector3d>&  LaserTriangulator::worldCoord() {
  return _worldCoord;
}

void LaserTriangulator::setWorldRotation(Matrix3d const & R) {
  _worldRotation = R;
}
void LaserTriangulator::setWorldTranslation(Vector3d& T) {
  _worldTranslation = T;
}
void LaserTriangulator::setDegreesPerFrame(float deg) {
  _degreesPerFrame = deg;
}

void LaserTriangulator::setIntrinsicCameraParameters
(Matrix3d const& K, Vector5d const& kc) {
  _K = K; _kc = kc;
}

void LaserTriangulator::run() {

  _log(QString("run {"));

  _stop  = false;
  _index = 0;

  int  width   = 0;
  int  height  = 0;
  int  nPixels = 0;

  // QStringList fileList;
  QDir dir(_workDirectory);
  auto filters(QStringList() << "*.jpg" << "*.bmp" << "*.png");
  _fileList =
    dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);

  emit started();

  foreach(QFileInfo file, _fileList) {
    QImage qImgSrc;
    QString path = file.absoluteFilePath();

	std::cout << path.toUtf8().constData() << std::endl;

    if(qImgSrc.load(path)) {
      if(width==0 || height==0) {
        width  = qImgSrc.width();
        height = qImgSrc.height();
        nPixels = width*height;
      }
        
      if(width ==qImgSrc.width() && height==qImgSrc.height()) {

        // assumes that images belonging to the same frame are consecutive
        // F_000_0.png, F_000_1.png, F_000_2.png, F_001_0.png, ...

        QStringList parts = path.split('_');
        // should have three parts
        if(parts.size()!=3) continue;
        int frameNumber = parts.at(1).toInt();
        float angle = _degreesPerFrame*((float)frameNumber);

        if(path.endsWith("_0.png")) {
          _backroundImg = qImgSrc;
          _index++;
        } else if(path.endsWith("_1.png")) {
          hw4::scan
            (_K,_kc,_worldRotation,_worldTranslation,angle,
             _backroundImg, qImgSrc, _laserPlane1,_worldCoord);
        } else if(path.endsWith("_2.png")) {
          hw4::scan
            (_K,_kc,_worldRotation,_worldTranslation,angle,
             _backroundImg, qImgSrc, _laserPlane2,_worldCoord);
        } 
      
        if(_stop) break;
      
        emit progress();
      }
    
    } // if(qImgSrc.load(file.absoluteFilePath()))
  } // foreach(QFileInfo file, _fileList)

  emit finished();

  _log(QString("}"));
}

void LaserTriangulator::stop(bool error, QString message) {
  _message = message;
  _stop = true;
  // emit finished();
  // quit();
}
