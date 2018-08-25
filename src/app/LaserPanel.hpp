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

#ifndef _LaserPanel_hpp_
#define _LaserPanel_hpp_

#include <QWidget>
#include <QImage>
#include <QStandardItemModel>

#include "ui_LaserPanel.h"

#include "CalibrationDataCamera.hpp"
#include "TurntableCalib.hpp"
#include "ChessboardCorners.hpp"

class QShowEvent;
class QHideEvent;

class LaserPanel : public QWidget, protected Ui_LaserPanel
{
  Q_OBJECT;

private:

  static bool _registered;
  static bool registerPanel();

  QStandardItemModel                _model;
  CalibrationDataCamera       _cameraCalib;
  TurntableCalib           _turntableCalib;
  ChessboardCorners     _chessboardCorners;

  void   loadSettings(void);
  void   saveSettings(void);
  void   enableGUI(bool enable);

  QImage loadCurrentImage(bool displayError = true);
  void   updateImage(QImage image);
  void   updateImageFiles();

  bool   loadCameraCalib(QString calibFile = QString());
  void   cameraCalibGroupUpdate(void);

  bool   loadTurntableCalib(QString calibFile = QString());
  bool   loadChessboardCorners(QString calibFile = QString());
  void   turntableCalibGroupUpdate(void);
  void   chessboardButtonsSetEnable(bool value);

  bool   doDetection(QString imageFilePath, QString imageFilePath1);

  QVector<Vector3d> doTriangulate(QString imageFilePath);

  bool   savePointcloud(QVector<Vector3d> const& pointcloud,
                        QString defaultFilePath = QString());

public:

  LaserPanel(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~LaserPanel();

protected:

  void showEvent(QShowEvent * e);
  void hideEvent(QHideEvent * e);

signals:

public slots:

protected slots:

  void _on_workDirectoryChanged();
  void _on_currentImageChanged
  (const QModelIndex & current, const QModelIndex & previous);

  // Plot panel
  void on_xminSpin_valueChanged(int i);
  void on_xmaxSpin_valueChanged(int i);
  void on_ySpin_valueChanged(int i);
  void on_drawLinePlot_stateChanged();

  // Filter panel
  void on_smoothButton_clicked();

  // Detection panel
  void on_displayDetectionCheck_stateChanged();
  void on_detectCurrentButton_clicked();
  void on_detectAllButton_clicked();

  // Camera Calibration panel
  void on_cameraCalibLoadButton_clicked();

  // Turntable Calibration panel
  void on_turntableCalibLoadButton_clicked();
  void on_chessboardCornersLoadButton_clicked();
  void on_turntableCalibrateSelectedButton_clicked();
  void on_turntableCalibrateAllButton_clicked();
  void on_estimateCenterOfRotationButton_clicked();
  void on_saveTurntablePointsButton_clicked();

  // Triangulate panel
  void on_triangulateCurrentButton_clicked();
  void on_triangulateAllButton_clicked();

  // Laser Plane panel
  void on_laserPlaneSelectedButton_clicked();
  void on_laserPlaneAllButton_clicked();

};

#endif //_LaserPanel_hpp_
