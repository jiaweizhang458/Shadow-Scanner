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

#include "ChessboardCorners.hpp"

#include <iostream>

#include <QDir>
#include <QFileInfo>

#include <matlab-io/MatlabIO.hpp>
#include <matlab-io/util.hpp>

ChessboardCorners::ChessboardCorners():
  _centerOfRotation(),
  _R(), _T(), _wPt(), _iPt() {
  _centerOfRotation << 0,0;
  _worldRotation << 1,0,0,0,1,0,0,0,1;
  _worldTranslation << 0,0,0;
}

void ChessboardCorners::clear(void) {
  _centerOfRotation(0) = 0; _centerOfRotation(1) = 0;
  _worldRotation(0,0)  = 1; _worldRotation(0,1)  = 0; _worldRotation(0,2)  = 0;
  _worldRotation(1,0)  = 0; _worldRotation(1,1)  = 1; _worldRotation(1,2)  = 0;
  _worldRotation(2,0)  = 0; _worldRotation(2,1)  = 0; _worldRotation(2,2)  = 1;
  _worldTranslation(0) = 0; _worldTranslation(1) = 0; _worldTranslation(2) = 0;
  _R.clear();
  _T.clear();
  _wPt.clear();
  _iPt.clear();
}

unsigned ChessboardCorners::imageCount() {
  unsigned n,m; 
  n = _R.size();
  if((m = _T.size())<n) n = m; 
  if((m = _wPt.size())<n) n = m;
  if((m = _iPt.size())<n) n = m;
  return n;
}

bool ChessboardCorners::isEmpty() {
  return (imageCount()==0);
}

bool ChessboardCorners::hasValidTransform(unsigned indx) {
  bool value = false;
  // - check whether R[index] is equal to zero or not
  // - although it should check whether R^t*R==I 
  if(indx < imageCount()) {
    Matrix3d& R = _R[indx];
    value = (R(0,0)!=0.0 || R(0,1)!=0.0 || R(0,2)!=0.0 ||
             R(1,0)!=0.0 || R(1,1)!=0.0 || R(1,2)!=0.0 ||
             R(2,0)!=0.0 || R(2,1)!=0.0 || R(2,2)!=0.0);
  }
  return value;
}

bool ChessboardCorners::hasValidTransforms() {
  bool value = true;
  unsigned nImages = imageCount();
  for(unsigned indx=0;value==true && indx<nImages;indx++)
    value = hasValidTransform(indx);
  return value;
}

Vector2d&                   ChessboardCorners::centerOfRotation()
{ return   _centerOfRotation; }
Matrix3d&                   ChessboardCorners::worldRotation()
{ return   _worldRotation; }
Vector3d&                   ChessboardCorners::worldTranslation()
{ return   _worldTranslation; }
QVector<Matrix3d>&          ChessboardCorners::R()   { return   _R; }
QVector<Vector3d>&          ChessboardCorners::T()   { return   _T; }
QVector<QVector<Vector2d>>& ChessboardCorners::wPt() { return _wPt; }
QVector<QVector<Vector2d>>& ChessboardCorners::iPt() { return _iPt; }

bool ChessboardCorners::loadMatlabCalib(QString filename, bool zero_RT) {
    
  //reset previous data
  clear();
  
  // create a new reader
  MatlabIO matio;
  if (!matio.open(filename, "r")) { //error
    qDebug("[ChessboardCorners::loadMatlabCalib] Error: Mat file load failed: %s", 
           qPrintable(filename));
    return false;
  }

  // read all of the variables in the file
  // typedef QMap<QString, QVariant> QVariantMap;
  QVariantMap varMap = matio.read(); 

  // close the file
  matio.close();

  qDebug("[ChessboardCorners::loadMatlabCalib] debug: loaded '%s'",
         qPrintable(filename));

  // display the file info
  matio.whos(varMap);

  //check
  if ( !varMap.contains("R") ||
       !varMap.contains("T") ||
       !varMap.contains("wPt") ||
       !varMap.contains("iPt") ) { //error
    qDebug("[ChessboardCorners::loadMatlabCalib] Error: bad MAT file %s",
           qPrintable(filename));
    return false;
  }

  qDebug("[ChessboardCorners::loadMatlabCalib] contains R, T, wPt, and iPt"); 

  typedef QVector<QVariant> QVariantVec;

  QVariantVec R   = varMap["R"].value<QVariantVec>();
  QVariantVec T   = varMap["T"].value<QVariantVec>();
  QVariantVec wPt = varMap["wPt"].value<QVariantVec>();
  QVariantVec iPt = varMap["iPt"].value<QVariantVec>();

  std::cerr << "-------------------" << std::endl;

  std::cerr << "Loading Chessboard Corners:" << std::endl;

  unsigned N = R.size();
  qDebug("  N = %d",N);
  if(N==0) {
    return false;
  } else if(T.size()!=N) {
    qDebug("  T.size() = %d",N);
    return false;
  } else if(wPt.size()!=N) {
    qDebug("  wPt.size() = %d",N);
    return false;
  } else if(iPt.size()!=N) {
    qDebug("  iPt.size() = %d",N);
    return false;
  }

  // private:
  // QVector<Matrix3d> _R;
  // QVector<Vector3d> _T;
  // QVector<QVector<Vector2d>> _wPt;
  // QVector<QVector<Vector2d>> _iPt;

  // typedef QVector<QVector<QVector<double>>> MatlabMatrixType;
  MatlabMatrixType m2d;
  unsigned rows,cols;
  for(unsigned i=0;i<N;i++) {
    std::cerr << "["<< i << "] = {" << std::endl;
     
    m2d = R[i].value<MatlabMatrixType>();
    cols = m2d.first().size(); // assert(cols==3);
    rows = m2d.first().first().size(); // assert(rows==3);
    std::cerr << " R[" << rows << "][" << cols << "] = [" << std::endl;
    Matrix3d Ri;
    if(zero_RT) {
        Ri << 0,0,0,0,0,0,0,0,0;
    } else {
      matlab::get2DMatrixDouble(m2d, Ri.data(), rows, cols, matlab::ColMajor);
    }
    std::cerr << Ri << std::endl;
    std::cerr << " ];" << std::endl;
    _R.push_back(Ri);
     
    m2d = T[i].value<MatlabMatrixType>();
    cols = m2d.first().size(); // assert(cols==3);
    rows = m2d.first().first().size(); // assert(rows==1);
    std::cerr << " T[" << rows << "][" << cols << "] = [" << std::endl; 
    Vector3d Ti;
    if(zero_RT) {
        Ti << 0,0,0;
    } else {
      matlab::get2DMatrixDouble(m2d, Ti.data(), rows, cols, matlab::ColMajor);
    }
    std::cerr << Ti << std::endl;
    std::cerr << " ];" << std::endl;
    _T.push_back(Ti);
    
    m2d = wPt[i].value<MatlabMatrixType>();
    cols = m2d.first().size(); // assert(cols==2);
    rows = m2d.first().first().size(); // assert(rows>0);
    std::cerr << " wPt[" << rows << "][" << cols << "] = [" << std::endl;
    QVector<Vector2d> wPti;
    matlab::get2DMatrixVector2d(m2d, wPti, rows);
    for(int i=0;i<3;i++)
      std::cerr << wPti[i](0) << " " << wPti[i](1) << std::endl;
    std::cerr << "..." << std::endl;
    std::cerr << " ];" << std::endl;
    _wPt.push_back(wPti);
     
    m2d = iPt[i].value<MatlabMatrixType>();
    cols = m2d.first().size(); // assert(cols==2);
    rows = m2d.first().first().size(); // assert(rows>0);
    std::cerr << " iPt[" << rows << "][" << cols << "] = [" << std::endl;
    QVector<Vector2d> iPti;
    matlab::get2DMatrixVector2d(m2d, iPti, rows);
    for(int i=0;i<3;i++)
      std::cerr << iPti[i](0) << " " << iPti[i](1) << std::endl;
    std::cerr << "..." << std::endl;
    std::cerr << " ];" << std::endl;
    _iPt.push_back(iPti);
     
    std::cerr << "}" << std::endl;
  }

  std::cerr << "-------------------" << std::endl;

  return true;
}
