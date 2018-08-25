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

#include "CalibrationDataCamera.hpp"

#include <iostream>

#include <QDir>
#include <QFileInfo>

#include <matlab-io/MatlabIO.hpp>
#include <matlab-io/util.hpp>

////////////////////////////////////////////////////////////////////////////////
CalibrationDataCamera::CalibrationDataCamera() :
  CalibrationData(),
  K(),
  kc() {

}

////////////////////////////////////////////////////////////////////////////////
void CalibrationDataCamera::clear(void) {
  K = Matrix3d::Zero();
  kc = Vector5d::Zero();
  calError = -1.0;
  isCalibrated = false;
  imageCount = 0U;
  imageSize = QSize();
  imageData.clear();
}

////////////////////////////////////////////////////////////////////////////////
bool CalibrationDataCamera::loadMatlabCalib(QString filename) {
  //reset previous data
  clear();

  // create a new reader
  MatlabIO matio;
  if (!matio.open(filename, "r")) { //error
    qDebug("[loadMatlabCalib] Error: Mat file load failed: %s",
           qPrintable(filename));
    return false;
  }

  // read all of the variables in the file
  QMap<QString,QVariant> varMap = matio.read();

  // close the file
  matio.close();

  qDebug("[loadMatlabCalib] debug: loaded '%s'", qPrintable(filename));

  // display the file info
  // matio.whos(varMap);

  //check
  if ( !varMap.contains("n_ima")
       || !varMap.contains("KK")
       || !varMap.contains("kc")) { //error
    qDebug("[loadMatlabCalib] Error: %s", qPrintable(filename));
    return false;
  }
  
  // TODO Wed Mar 14 00:20:38 2018 : read
  // calError

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

  // std::cerr << "Camera Calibration Loaded {" << std::endl;
  // std::cerr << " - K:\n" <<
  //   K << std::endl;
  // std::cerr << " - kc:\n" <<
  //   kc << std::endl;
  // std::cerr << " - imageCount: " <<
  //   imageCount << std::endl;
  // std::cerr << " - imageSize: " <<
  //   imageSize.width() << "x" << imageSize.height() << std::endl;
  // std::cerr << " - worldPoints: " <<
  //   (imageData.size() ? imageData.front().worldPoints.size() : 0) << std::endl;
  // std::cerr << " - imagePoints: " <<
  //   (imageData.size() ? imageData.front().imagePoints.size() : 0) << std::endl;
  // std::cerr << " - imageName: " <<
  //   (imageData.size() ? qPrintable(imageData.front().imageName) : "") << std::endl;
  // std::cerr << "}" << std::endl;

  print();

  isCalibrated = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
bool CalibrationDataCamera::saveMatlabCalib(QString filename) {
  bool retVal = false;
  std::cout << "CalibrationDataCamera::saveMatlabCalib() {\n";
  std::cout << "  filename = \""<< qPrintable(filename) <<"\"\n";
  std::cout << "  isCalibrated = "<< isCalibrated<<"\n";

  if(isCalibrated) {

    std::cout << "  TODO Mon Mar 12 18:41:10 2018\n";

    QMap<QString,QVariant> varMap;
    // calError
    varMap.insert("n_ima",QVariant(static_cast<uint>(imageCount)));
    // matlab::get2DMatrixDouble("KK", varMap, K.data(), 3, 3, matlab::ColMajor);
    // matlab::get2DMatrixDouble("kc", varMap, kc.data(), 5, 1);
    varMap.insert("nx",QVariant(static_cast<int>(imageSize.width())));
    varMap.insert("ny",QVariant(static_cast<int>(imageSize.height())));
    // QString calib_name = varMap["calib_name"].toString();
    // QString format_image = varMap["format_image"].toString();
    // int N_slots = varMap["N_slots"].toInt();
    for (int i = 0; i < imageCount; ++i) {

      // TODO Wed Mar 14 00:22:34 2018

    }
  }

  std::cout << "}\n";
  return retVal;
}

//////////////////////////////////////////////////////////////////////
void CalibrationDataCamera::print() {

  std::cout << "CalibrationDataCamera::print() {\n";

  std::cout << "  isCalibrated = "<< isCalibrated <<"\n";
  std::cout << "  calError     = "<< calError <<"\n";
  

  // Matrix3d K;
  std::cout << "  K = [\n";
  std::cout << K << "\n";
  std::cout << "  ];\n";

  // Vector5d kc;
  std::cout << "  kc = [ "
            << kc(0) << " "
            << kc(1) << " "
            << kc(2) << " "
            << kc(3) << " "
            << kc(4) << " ]\n";

  // QSize  imageSize;
  std::cout << "  imageSize = ["
            << imageSize.width() << "," << imageSize.height() << "]\n";

  // size_t imageCount;
  std::cout << "  imageCount = " << imageCount << "\n";

  // QVector<ImageData> imageData;
  std::cout << "  imageData[" << imageCount << "] = {\n";;
  for(uint indx=0;indx<imageCount;indx++) {
    const ImageData & iData = imageData.at(indx);
    std::cout << "    imageData[" << indx << "] = {\n";
    std::cout << "      imageName    = \"" << qPrintable(iData.imageName) << "\"\n";
    std::cout << "      nImagePoints = " << iData.imagePoints.size() << "\n";
    std::cout << "      nWorldPoints = " << iData.worldPoints.size() << "\n";
    std::cout << "      rotVec       = [ "
              << iData.rotVec(0) << " "
              << iData.rotVec(1) << " "
              << iData.rotVec(2) << " ]\n";
    std::cout << "      T            = [ "
              << iData.T(0) << " "
              << iData.T(1) << " "
              << iData.T(2) << " ]\n";
    std::cout << "    };\n";
  }
  std::cout << "  }\n";;

  std::cout << "}\n";
}
