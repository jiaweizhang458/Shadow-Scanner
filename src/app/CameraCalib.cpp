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

#include "CameraCalib.hpp"

#include <iostream>

#include <QDir>
#include <QFileInfo>

#include <matlab-io/MatlabIO.hpp>
#include <matlab-io/util.hpp>

CameraCalib::CameraCalib() :
  K(),
  kc(),
  imageCount(0U),
  imageSize(),
  imageData() {
}

void CameraCalib::clear(void) {
  K = Matrix3d::Zero();
  kc = Vector5d::Zero();
  imageCount = 0U;
  imageSize = QSize();
  imageData.clear();
}

bool CameraCalib::loadMatlabCalib(QString filename) {
  //reset previous data
  clear();

  // create a new reader
  MatlabIO matio;
  if (!matio.open(filename, "r")) { //error
    qDebug("[CameraCalib::loadMatlabCalib] Error: Mat file load failed: %s",
           qPrintable(filename));
    return false;
  }

  // read all of the variables in the file
  auto varMap = matio.read();

  // close the file
  matio.close();

  qDebug("[CameraCalib::loadMatlabCalib] debug: loaded '%s'", qPrintable(filename));

  // display the file info
  // matio.whos(varMap);

  //check
  if ( !varMap.contains("n_ima")
       || !varMap.contains("KK")
       || !varMap.contains("kc")) { //error
    qDebug("[CameraCalib::loadMatlabCalib] Error: Mat file does not contains camera calibration data: %s",
           qPrintable(filename));
    return false;
  }
  
  //calib data
  imageCount = varMap["n_ima"].toUInt();
  matlab::get2DMatrixDouble("KK", varMap, K.data(), 3, 3, matlab::ColMajor);
  matlab::get2DMatrixDouble("kc", varMap, kc.data(), 5, 1);

  //image data
  imageSize.setWidth(varMap["nx"].toInt());
  imageSize.setHeight(varMap["ny"].toInt());
  QString calib_name = varMap["calib_name"].toString();
  QString format_image = varMap["format_image"].toString();
  int N_slots = varMap["N_slots"].toInt();
  QDir dir = QFileInfo(filename).absoluteDir();
  imageData.resize(static_cast<int>(imageCount));
  for (int i = 0; i < imageCount; ++i) {
    auto m = i + 1;
    auto & data = imageData[i];
    matlab::get2DMatrixDouble
      (QString("omc_%1").arg(m), varMap, data.rotVec.data(), 3, 1);
    matlab::get2DMatrixDouble
      (QString("Tc_%1").arg(m), varMap, data.T.data(), 3, 1);

    int n_sq_x = varMap[QString("n_sq_x_%1").arg(m)].toInt();
    int n_sq_y = varMap[QString("n_sq_y_%1").arg(m)].toInt();
    int n = (1+n_sq_x)*(1+n_sq_y);

    data.worldPoints.resize(n);
    data.imagePoints.resize(n);

    matlab::get2DMatrixDouble
      (QString("X_%1").arg(m), varMap,
       reinterpret_cast<double*>(data.worldPoints.data()), 3, n, matlab::ColMajor);
    matlab::get2DMatrixDouble
      (QString("x_%1").arg(m), varMap,
       reinterpret_cast<double*>(data.imagePoints.data()), 2, n, matlab::ColMajor);

    data.imageName =
      dir.absoluteFilePath(QString("%1%2.%3").arg(calib_name).arg
                           (m, N_slots, 10, QLatin1Char('0')).arg(format_image));
  }

  //debug
  std::cerr << "-------------------" << std::endl;
  std::cerr << "Camera Calibration Loaded:" << std::endl;
  std::cerr << " - K:\n" <<
    K << std::endl;
  std::cerr << " - kc:\n" <<
    kc << std::endl;
  std::cerr << " - imageCount: " <<
    imageCount << std::endl;
  std::cerr << " - imageSize: " <<
    imageSize.width() << "x" << imageSize.height() << std::endl;
  std::cerr << " - worldPoints: " <<
    (imageData.size() ? imageData.front().worldPoints.size() : 0) << std::endl;
  std::cerr << " - imagePoints: " <<
    (imageData.size() ? imageData.front().imagePoints.size() : 0) << std::endl;
  std::cerr << " - imageName: " <<
    (imageData.size() ? qPrintable(imageData.front().imageName) : "") << std::endl;
  std::cerr << "-------------------" << std::endl;

  return true;
}
