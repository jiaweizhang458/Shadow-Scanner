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

#include "TurntableCalib.hpp"

#include <iostream>

#include <QDir>
#include <QFileInfo>

#include <matlab-io/MatlabIO.hpp>
#include <matlab-io/util.hpp>

TurntableCalib::TurntableCalib() :
  Rc(Matrix3d::Identity()),
  Tc(),
  center(),
  laserPlane(),
  imageAngle() {
}

void TurntableCalib::clear(void) {
  Rc = Matrix3d::Identity();
  Tc = Vector3d::Zero();
  center = Vector3d::Zero();
  laserPlane = Vector4d::Zero();
  imageAngle.clear();
}

bool TurntableCalib::loadMatlabCalib(QString filename) {
  //reset previous data
  clear();

  // create a new reader
  MatlabIO matio;
  if (!matio.open(filename, "r")) { //error
    qDebug("[TurntableCalib::loadMatlabCalib] Error: Mat file load failed: %s", 
           qPrintable(filename));
    return false;
  }

  // read all of the variables in the file
  auto varMap = matio.read();

  // close the file
  matio.close();

  qDebug("[TurntableCalib::loadMatlabCalib] debug: loaded '%s'",
         qPrintable(filename));

  // display the file info
  // matio.whos(varMap);

  //check
  if ( !varMap.contains("version")
       || !varMap.contains("R1")
       || !varMap.contains("o1")
       || !varMap.contains("c")
       || !varMap.contains("laserCalib")
       || !varMap.contains("alpha")
       || !varMap.contains("listing")) { //error
    qDebug("[TurntableCalib::loadMatlabCalib] Error: Mat file does not contains turntable calibration data: %s",
           qPrintable(filename));
    return false;
  }

  //version
  unsigned version = varMap["version"].toUInt();
  if (version!=1) { //error
    qDebug("[TurntableCalib::loadMatlabCalib] Error: Invalid version: %u", version);
    return false;
  }

  //calib data
  matlab::get2DMatrixDouble("R1", varMap, Rc.data(), 3, 3, matlab::ColMajor);
  matlab::get2DMatrixDouble("o1", varMap, Tc.data(), 3, 1);
  matlab::get2DMatrixDouble("c", varMap, center.data(), 3, 1);

  typedef QVector<QMap<QString,QVariant>> StructListType;
  auto laserListStruct = varMap["laserCalib"].value<StructListType>();
  if (!laserListStruct.isEmpty()) {
    matlab::get2DMatrixDouble("plane",
                              laserListStruct.first(), laserPlane.data(), 4, 1);
  }

  //image data
  typedef QVector<QVector<QVector<double>>> MatrixType;
  auto alpha = varMap["alpha"].value<MatrixType>();
  auto listing = varMap["listing"].value<StructListType>();
  if (alpha.size() && alpha.first().size() &&
      alpha.first().first().size()==listing.size()) {
    auto const& alphaVector = alpha.first().first();
    auto nImages = alphaVector.size();
    for (int i=0; i<nImages; ++i) {
      double angle = alphaVector[i];
      QString imageName = listing[i]["name"].toString();
      imageAngle.insert(imageName, angle);
    }
  }

  //debug
  std::cerr << "-------------------" << std::endl;
  std::cerr << "Turntable Calibration Loaded:" << std::endl;
  std::cerr << " - version: " <<
    version << std::endl;
  std::cerr << " - Rc:\n" <<
    Rc << std::endl;
  std::cerr << " - Tc:\n" <<
    Tc << std::endl;
  std::cerr << " - center:\n" <<
    center << std::endl;
  std::cerr << " - laserPlane:\n" <<
    laserPlane << std::endl;
  std::cerr << " - imageAngle: " <<
    imageAngle.keys().size() << std::endl;
  std::cerr << " - imageName: " <<
    (imageAngle.size() ? qPrintable(imageAngle.keys().first()) : "") << std::endl;
  std::cerr << "-------------------" << std::endl;

  return true;
}
