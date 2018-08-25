// Software developed for the Spring 2018 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
// Copyright (c) 2018, Gabriel Taubin
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

#ifndef _TURNTABLE_SCANNING_PANEL_HPP_
#define _TURNTABLE_SCANNING_PANEL_HPP_

#include <QWidget>
#include <QImage>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QAtomicInteger>
#include <QtSerialPort/QSerialPort>

#include "ui_TurntableScanningPanel.h"

#include <util/Vec.hpp>

#include "CalibrationDataCamera.hpp"
#include "CalibrationDataTurntable.hpp"
#include "CalibrationDataLaserLine.hpp"

class CameraInterface;
class CameraRecorder;
class SerialPortSettings;
class SerialPortCommandLine;
class LaserTriangulator;

#define SCANNER_CALIBRATION_FILENAME "Scanner_Calibration.txt"

class TurntableScanningPanel : public QWidget, protected Ui_TurntableScanningPanel
{
  Q_OBJECT;

  enum CalibrationMode {
    CalibrateCamera,
    CalibrateTurntable,
    CalibrateLaser1,
    CalibrateLaser2
  };

  static bool _registered;
  static bool registerPanel();

  void   loadSettings(void);
  void   saveSettings(void);

  void   getAndSetupCamera(void);
  void   updateCameraResolutions(void);
  void   changeCameraResolution(void);

  int    getLaserMode();

  CalibrationMode getCalibrationMode();
  void setCalibrationMode(const CalibrationMode mode);
  CalibrationData & calibrationData();

  void   updateImage(QImage image, int rotation = 0);
  void   updateImageFiles();
  void   deleteAllImageFiles();

  void   chessboardDetectCurrent(bool interactive, bool displayCorners);
  void   calibrateLaserPlane(int h);

  void   showEvent(QShowEvent * event);
  void   hideEvent(QHideEvent * event);

public:

   TurntableScanningPanel(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~TurntableScanningPanel();

  QByteArray sendTurntableCommand(QString cmd, int msec);

  void updateState();

signals:
  void setInfoLabel(QString text);

public slots:

  void _on_captureStarted();
  void _on_captureProgress();
  void _on_captureFinished();

  void _on_laserTriangulatorStarted();
  void _on_laserTriangulatorProgress();
  void _on_laserTriangulatorFinished();

protected slots:

  void _on_workDirectoryChanged();

  // Camera Group
  void on_cameraCombo_currentIndexChanged(int index);
  void on_imageRotCombo_currentIndexChanged(int index);
  void on_previewButton_clicked(bool checked = false);

  // Turntable Group
  void on_serialPortSettingsButton_clicked();
  void on_serialPortCommandButton_clicked();
  void on_serialPortButton_clicked(bool checked = false);

  // Capture Group
  // countSpin
  // frameDelaySpin
  void on_blankFrameCheck_stateChanged();
  void on_laser1FrameCheck_stateChanged();
  void on_laser2FrameCheck_stateChanged();
  void on_frameNumberLine_returnPressed();
  // frameImageLine <- disabled
  // frameTotalImageLine <- disabled
  void on_oneImageLine_returnPressed();
  void on_captureButton_clicked(bool checked = false);

  // Images Group
  void _on_currentImageChanged
  (const QModelIndex & current, const QModelIndex & previous);
  void on_imagesUpdateButton_clicked();
  void on_imagesDeleteAllButton_clicked();
  void on_imagesNextButton_clicked();
  void on_imagesPreviousButton_clicked();

  // Calibration Group
  void on_calibrateCameraCheck_stateChanged();
  void on_calibrateTurntableCheck_stateChanged();
  void on_calibrateLaser1Check_stateChanged();
  void on_calibrateLaser2Check_stateChanged();

  void on_chessboardDetectAllButton_clicked(bool checked);
  void on_calibrateButton_clicked();
  void on_loadScannerCalibrationButton_clicked();
  void on_saveScannerCalibrationButton_clicked();

  // Scanning Group
  // scanningProgress
  void on_scanningScanButton_clicked(bool checked);

  // 3D Scene Group
  // connect(wrlFileLoadButton,  SIGNAL(clicked()),
  //         mainWin, SLOT(on_wrlFileLoadButton_clicked()));
  // connect(wrlFileSaveButton,  SIGNAL(clicked()),
  //         mainWin, SLOT(on_wrlFileSaveButton_clicked()));
  
#ifdef HAVE_IMG
  void updateInfo(ImageBuffer const& image);
#endif //HAVE_IMG

public:
  void       serialPortWriteData(const QByteArray &data);
  QByteArray serialPortReadData(int msec);

private slots:
  void       serialPortUpdateName();

private:

  QSharedPointer<CameraInterface>        _camera;
  size_t                             _imageCount;
  QElapsedTimer                        _fpsTimer;
  QStandardItemModel                      _model;
  QAtomicInteger<int>           _captureOneImage;
  SerialPortSettings*        _serialPortSettings;
  SerialPortCommandLine*  _serialPortCommandLine;
  QSerialPort*                           _serial;
  CameraRecorder*                      _recorder;
  QImage                           _currentImage;
  int                           _currentRotation;
  int                                _frameIndex;
  int                                _laserIndex;
  int                             _oneImageIndex;
  CalibrationDataCamera             _cameraCalib;
  CalibrationDataTurntable       _turntableCalib;
  CalibrationDataLaserLine          _laser1Calib;
  CalibrationDataLaserLine          _laser2Calib;
  LaserTriangulator*          _laserTriangulator;
};

#endif //_TurntableScanningPanel_hpp_
