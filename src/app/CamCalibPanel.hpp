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

#ifndef _CamCalibPanel_hpp_
#define _CamCalibPanel_hpp_

#include <QWidget>
#include <QImage>
#include <QStandardItemModel>

#include "ui_CamCalibPanel.h"

#include "CalibrationDataCamera.hpp"

#include <opencv2/core/core.hpp>

class QShowEvent;
class QHideEvent;

class CamCalibPanel : public QWidget, protected Ui_CamCalibPanel
{
  Q_OBJECT;

  static bool _registered;
  static bool registerPanel();

  CalibrationDataCamera        _calib;
  QStandardItemModel           _model;
  QImage                _currentImage;

  struct Reprojection { QVector<Vector2d> imagePoints; double rmsError;};
  QVector<Reprojection> _reprojection;

  void loadSettings(void);
  void saveSettings(void);

  void updateImage(QImage image);
  void updateImageFiles(void);
  void deleteAllImageFiles();

  void calibGroupUpdate(void);
  void updateMinMaxError(void);

  void reprojectImage(int imageIndex);

  void resetPanel(void);

  bool loadMatlabCalib(void);
  bool saveMatlabCalib(void);

  // int detectChessboardCorners
  // (cv::Mat1b image, cv::Size2i cornerCount, cv::Size2f cornerSize,
  //  std::vector<cv::Point3f> & cornersWorld,
  //  std::vector<cv::Point2f> & cornersCamera);

  void chessboardDetectCurrent(bool interactive);

public:

  CamCalibPanel(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~CamCalibPanel();

protected:

  void showEvent(QShowEvent * e);
  void hideEvent(QHideEvent * e);

protected slots:

  void _on_workDirectoryChanged();

  // Images Group
  void _on_currentImageChanged
       (const QModelIndex & current, const QModelIndex & previous);
  void on_imagesUpdateButton_clicked();
  void on_imagesDeleteAllButton_clicked();
  void on_imagesNextButton_clicked();
  void on_imagesPreviousButton_clicked();

  // Chessboard Group
  void on_chessboardDetectButton_clicked();
  void on_chessboardDetectPreviousButton_clicked();
  void on_chessboardDetectNextButton_clicked();
  void on_chessboardDetectAllButton_clicked(bool checked);
  // <widget class="QProgressBar" name="chessboardProgress">
  // void on_chessboardColsSpin_valueChanged(int i);
  // void on_chessboardRowsSpin_valueChanged(int i);
  // void on_chessboardCellWidthValue_returnPressed();
  // void on_chessboardCellHeightValue_returnPressed();
  
  // Calibration Group
  void on_calibrateCameraButton_clicked();
  void on_saveCalibrationButton_clicked();
  void on_loadCalibrationButton_clicked();
  void on_clearCalibrationButton_clicked();
  
  // Reprojection Group
  void on_displayReprojectionCheck_stateChanged();
  void on_displayCornersCheck_stateChanged();
  void on_reprojectCurrentButton_clicked();
  void on_reprojectAllButton_clicked();
};

#endif //_CamCalibPanel_hpp_
