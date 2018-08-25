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

#include "LaserPanel.hpp"

#include <QItemSelectionModel>
#include <QShowEvent>
#include <QHideEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QPainter>
#include <QTimer>
#include <QTime>
#include <QDir>

#include "Application.hpp"
#include "MainWindow.hpp"
#include "PlotMaker.hpp"

#include "gui_util.hpp"

#ifdef HAVE_HOMEWORK
#include <homework/homework1.hpp>
#include <homework/homework2.hpp>
#include <homework/homework3.hpp>
#endif //HAVE_HOMEWORK

//////////////////////////////////////////////////////////////////////

bool LaserPanel::_registered = LaserPanel::registerPanel();
bool LaserPanel::registerPanel() {
  qDebug("LaserPanel: register");
  return MainWindow::registerPanel
    ("Laser", [](QWidget *parent) -> QWidget*{ return new LaserPanel(parent); });
  return true;
}

LaserPanel::LaserPanel(QWidget * parent, Qt::WindowFlags f) :
  QWidget(parent, f),
  _model(),
  _cameraCalib(),
  _turntableCalib(),
  _chessboardCorners()
{

  setupUi(this);

  //load other settings
  loadSettings();

  //list view model
  imageList->setModel(&_model);
  auto selectionModel = imageList->selectionModel();
  connect(selectionModel, &QItemSelectionModel::currentChanged,
          this,           &LaserPanel::_on_currentImageChanged);

  //progress bar
  detectProgress->setVisible(false);
  triangulateProgress->setVisible(false);
  laserPlaneProgress->setVisible(false);

  qDebug(" -- LaserPanel created -- ");
}

LaserPanel::~LaserPanel() {
  saveSettings();
  qDebug(" -- LaserPanel destroyed -- ");
}

void LaserPanel::loadSettings(void) {
  QSettings config;

  xminSpin->setMaximum(10000);
  xminSpin->setValue(config.value("LaserPanel/plotXMin", 0).toInt());

  xmaxSpin->setMaximum(10000);
  xmaxSpin->setValue(config.value("LaserPanel/plotXMax", 10000).toInt());

  ySpin->setMaximum(10000);
  ySpin->setValue(config.value("LaserPanel/plotY", 300).toInt());

  drawLinePlot->setChecked(config.value("LaserPanel/drawLine", false).toBool());
  
  smoothSpin->setMinimum(1);
  smoothSpin->setValue(config.value("LaserPanel/smoothCount", 1).toInt());
}

void LaserPanel::saveSettings(void) {
  QSettings config;

  config.setValue("LaserPanel/plotXMin", xminSpin->value());
  config.setValue("LaserPanel/plotXMax", xmaxSpin->value());
  config.setValue("LaserPanel/plotY", ySpin->value());
  config.setValue("LaserPanel/drawLine", drawLinePlot->isChecked());
  config.setValue("LaserPanel/smoothCount", smoothSpin->value());
}

void LaserPanel::showEvent(QShowEvent * e) {
  if (e && !e->spontaneous()) {
    // std::cerr << "void LaserPanel::showEvent(QShowEvent * event) {\n";

    auto mainWin = getApp()->getMainWindow();

    auto glWidget = mainWin->getImgGLWidget();
    glWidget->clearImage();
    glWidget->setRotation(0);

    displayDetectionCheck->setChecked(false);

    connect(mainWin, &MainWindow::workDirectoryChanged,
            this,    &LaserPanel::_on_workDirectoryChanged);

    updateImageFiles();
    loadCameraCalib();
    loadTurntableCalib();
    loadChessboardCorners();

    mainWin->showImgGLWidget(true);
    mainWin->showWrlGLWidget(false);
    
    // std::cerr << "}\n";
  }
}

void LaserPanel::hideEvent(QHideEvent * e) {
  if (e && !e->spontaneous()) {
    auto mainWin = getApp()->getMainWindow();
    disconnect(mainWin, &MainWindow::workDirectoryChanged,
               this,    &LaserPanel::_on_workDirectoryChanged);
  }
}

void LaserPanel::enableGUI(bool enable) {
  ENSURE_VISIBLE(_VOID_)
  auto mainWin = getApp()->getMainWindow();
  mainWin->setChangesEnabled(enable);
  imageList->setEnabled(enable);
  plotGroup->setEnabled(enable);
  detectionGroup->setEnabled(enable);
  filterGroup->setEnabled(enable);
  calibrationGroup->setEnabled(enable);
  turntableGroup->setEnabled(enable);
  chessboardButtonsSetEnable(enable);
  triangulateGroup->setEnabled(enable);
}

void LaserPanel::_on_workDirectoryChanged() {
  getApp()->getMainWindow()->getImgGLWidget()->clearImage();
  displayDetectionCheck->setChecked(false);
  updateImageFiles();
  loadCameraCalib();
  loadTurntableCalib();
  loadChessboardCorners();
}

//////////////////////////////////////////////////////////////////////
// images

void LaserPanel::updateImageFiles() {
  //MainWindow
  auto mainWin = getApp()->getMainWindow();
  
  //get files
  QStringList fileList;
  QDir dir(mainWin->getWorkDirectory());
  auto filters(QStringList() << "*.jpg" << "*.bmp" << "*.png");
  auto list =
    dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);
  foreach(auto file, list) {
    fileList.append(file.absoluteFilePath());
  }
  
  //update the model
  gui_util::updateFilesItems(&_model, fileList);
}

void LaserPanel::_on_currentImageChanged
(const QModelIndex & current, const QModelIndex & previous) {
  Q_UNUSED(current);
  Q_UNUSED(previous);
  updateImage(loadCurrentImage());
}

QImage LaserPanel::loadCurrentImage(bool displayError) {
  QString subdir =
    (displayDetectionCheck->isChecked() ? QString("detection") : QString());
  return gui_util::loadCurrentImage(imageList, displayError, subdir);
}

void LaserPanel::updateImage(QImage image) {
  auto mainWin = getApp()->getMainWindow();
  if (!mainWin) {
    return;
  }
  auto glWidget = mainWin->getImgGLWidget();

  if (image.isNull()) {
    glWidget->clearImage();
    return;
  }

  //update plot widgets
  xminSpin->blockSignals(true);
  xminSpin->setMaximum(image.width() - 1);
  xminSpin->blockSignals(false);
  
  xmaxSpin->blockSignals(true);
  xmaxSpin->setMaximum(image.width() - 1);
  xmaxSpin->blockSignals(false);
  
  ySpin->blockSignals(true);
  ySpin->setMaximum(image.height() - 1);
  ySpin->blockSignals(false);

  //update plot (before modifying qImg)
  int x0 = xminSpin->value();
  int x1 = std::max(x0, xmaxSpin->value());
  int y = ySpin->value();
  int width = plotWidget->width();
  int height = plotWidget->height();
  if (width && height) {
    PlotMaker plot(width, height);
    QVector<float> data;
    for (int i = x0; i < x1; ++i) {
      data.append(qGray(image.pixel(i, y)));
    }
    plot.setData(data);
    plotWidget->setQImage(plot.getImage());
    QTimer::singleShot(10, plotWidget, SLOT(update()));
  }

  //draw line on image
  if (drawLinePlot->isChecked()) {
    QPainter painter(&image);
    painter.setPen(Qt::yellow);
    painter.drawLine(x0, y, x1, y);
  }

  //update image
  glWidget->setQImage(image);
  QTimer::singleShot(10, glWidget, SLOT(update()));
}

//////////////////////////////////////////////////////////////////////
// Plot panel

void LaserPanel::on_xminSpin_valueChanged(int i) {
  Q_UNUSED(i);
  updateImage(loadCurrentImage(false));
}

void LaserPanel::on_xmaxSpin_valueChanged(int i) {
  Q_UNUSED(i);
  updateImage(loadCurrentImage(false));
}

void LaserPanel::on_ySpin_valueChanged(int i) {
  Q_UNUSED(i);
  updateImage(loadCurrentImage(false));
}

void LaserPanel::on_drawLinePlot_stateChanged() {
  updateImage(loadCurrentImage(false));
}

//////////////////////////////////////////////////////////////////////
// Filter panel

void LaserPanel::on_smoothButton_clicked() {
  //get selected image
  QImage image = loadCurrentImage();
  if (image.isNull()) {
    return;
  }

  //disable gui
  enableGUI(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  //check format
  if (image.format() != QImage::Format_RGB32) {
    image = image.convertToFormat(QImage::Format_RGB32);
  }
  
  //filter image
  QImage outImg = image;
  int count = smoothSpin->value();
  QVector<int> filter(QVector<int>() << 1 << 4 << 6 << 4 << 1);
  for (int i=0; i<count; ++i) {
    outImg = hw1::filterImage(outImg, filter);
  }

  //check result
  if (outImg.isNull()) { //invalid image
    qCritical("[ERROR] Filter failed.");
    return;
  }

  //enable gui
  enableGUI(true);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //display
  updateImage(outImg);
}

//////////////////////////////////////////////////////////////////////
// Detection panel

void LaserPanel::on_displayDetectionCheck_stateChanged() {
  updateImage(loadCurrentImage(false));
}

void LaserPanel::on_detectCurrentButton_clicked() {
  ////get current item
  //auto item = gui_util::getCurrentItem(imageList);
  //if (!item) { //no item selected
  //  qCritical("[ERROR] No item is selected.");
  //  return;
  //}

  ////get the image file path
  //QString imageFilePath = item->data().toString();
  //if (imageFilePath.isEmpty()) {
  //  qCritical("[ERROR] imageFilePath is EMPTY");
  //  return;
  //}

  ////disable gui
  //enableGUI(false);

  ////busy cursor
  //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  //QApplication::processEvents();

  ////run detection
  //doDetection(imageFilePath);

  ////enable gui
  //enableGUI(true);

  ////restore cursor
  //QApplication::restoreOverrideCursor();
	return;
}

void LaserPanel::on_detectAllButton_clicked() {
  //disable gui
  enableGUI(false);
  detectionGroup->setEnabled(true);
  displayDetectionCheck->setEnabled(false);
  detectCurrentButton->setEnabled(false);
  detectAllButton->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  int rows = _model.rowCount();
  detectProgress->setMaximum(rows);
  detectProgress->setVisible(true);

  for (int i = 0; i < rows; ++i) {
	QStandardItem* item = _model.item(i, 0);
	QStandardItem* item1;
	if (i % 2 == 0) // Scanning Image
	{
		item1 = _model.item(i + 1, 0);
	}
	else // Reference Image
	{
		item1 = _model.item(i - 1, 0);
	}

    if (!item) { //invalid item
      qCritical("[ERROR] Invalid item.");
      break;
    }

    //get the image file path
    QString imageFilePath = item->data().toString();
	QString imageFilePath1 = item1->data().toString();
    if (imageFilePath.isEmpty()) {
      qCritical("[ERROR] imageFilePath is EMPTY");
      break;
    }
	
	//Compute Max Illuminated Image
	QImage qImg, qImg1;
	qImg.load(imageFilePath);
	qImg1.load(imageFilePath1);
	QImage Max = hw1::ComputeMaxIllumination(qImg, qImg1);
	if (i % 2 == 0)
	{
		std::ostringstream name;
		name << "MaxIllumination_" << i/2 << ".png";
		Max.save(name.str().c_str());
	}


    //run detection
    if (!doDetection(imageFilePath, imageFilePath1)) { //error
      break;
    }

    detectProgress->setValue(i);
    QApplication::processEvents();
  }
  detectProgress->setVisible(false);

  //enable gui
  enableGUI(true);
  displayDetectionCheck->setEnabled(true);
  detectCurrentButton->setEnabled(true);
  detectAllButton->setEnabled(true);

  //restore cursor
  QApplication::restoreOverrideCursor();
}

bool LaserPanel::doDetection(QString imageFilePath, QString imageFilePath1) {
  //load image
  QImage qImg, qImg1;
  if (!qImg.load(imageFilePath)) {
    QMessageBox::critical
      (this, "Error", QString("Load failed: %1").arg(imageFilePath));
    return false;
  }
  if (qImg.format() != QImage::Format_RGB32) {
    qImg = qImg.convertToFormat(QImage::Format_RGB32);
  }

  if (!qImg1.load(imageFilePath1)) {
	  QMessageBox::critical
	  (this, "Error", QString("Load failed: %1").arg(imageFilePath1));
	  return false;
  }
  if (qImg1.format() != QImage::Format_RGB32) {
	  qImg1 = qImg1.convertToFormat(QImage::Format_RGB32);
  }

  qDebug("loaded %s", qPrintable(imageFilePath));
  qDebug("loaded %s", qPrintable(imageFilePath1));

  //detect laser
  QImage outImg = hw1::detectShadow(qImg, qImg1);

  //check result
  if (outImg.isNull()) { //invalid image
    QMessageBox::critical
      (this, "Error", QString("Detection failed: %1").arg(imageFilePath));
    return false;
  }

  //create output dir
  QString outdir("detection");
  QFileInfo info(imageFilePath);
  QDir dir = info.absoluteDir();
  if (!dir.mkpath(outdir)) { //invalid image
    QMessageBox::critical
      (this, "Error",
       QString("Create directory failed: %1").arg(dir.absoluteFilePath(outdir)));
    return false;
  }

  //save result
  dir.cd(outdir);
  QString outName = dir.absoluteFilePath(info.fileName());
  if (!outImg.save(outName)) { //save failed
    QMessageBox::critical
      (this, "Error", QString("Save image failed: %1").arg(outName));
    return false;
  }

  qDebug("saved %s", qPrintable(outName));

  displayDetectionCheck->blockSignals(true);
  displayDetectionCheck->setChecked(true);
  displayDetectionCheck->blockSignals(false);

  //display
  updateImage(outImg);
  return true;
}

//////////////////////////////////////////////////////////////////////
// Camera Calibration panel

void LaserPanel::on_cameraCalibLoadButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
  QString fileName =
    QFileDialog::getOpenFileName
    (this, "Open Camera Calibration",
     dir.absoluteFilePath(MATLAB_CALIB_FILENAME),
     "Matlab Calibration (" MATLAB_CALIB_FILENAME ")");
  if (!fileName.isEmpty()) {
    loadCameraCalib(fileName);
  }
}

bool LaserPanel::loadCameraCalib(QString calibFile) {
  ENSURE_VISIBLE(false)

    //reset
    _cameraCalib.clear();

  //calib file
  if (calibFile.isEmpty()) {
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    calibFile = dir.absoluteFilePath(MATLAB_CALIB_FILENAME);
  }

  //load
  bool rv = _cameraCalib.loadMatlabCalib(calibFile);

  //update GUI
  cameraCalibGroupUpdate();  
  turntableCalibGroupUpdate();  

  return rv;
}

void LaserPanel::cameraCalibGroupUpdate(void) {
  ENSURE_VISIBLE(_VOID_)

    QString cameraMsg;
  if (!_cameraCalib.imageCount || _cameraCalib.K(2,2)==0.0) {
    cameraMsg = "Camera calibration not loaded.";
  } else {
    cameraMsg = "Camera calibration found: '" MATLAB_CALIB_FILENAME "'";
  }
  cameraCalibLabel->setText(cameraMsg);
}

//////////////////////////////////////////////////////////////////////
// Turntable Calibration panel

void LaserPanel::on_turntableCalibLoadButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
  QString fileName =
    QFileDialog::getOpenFileName
    (this, "Open Turntable Calibration",
     dir.absoluteFilePath(TURNTABLE_CALIB_FILENAME),
     "Matlab Calibration (" TURNTABLE_CALIB_FILENAME ")");
  if (!fileName.isEmpty()) {
    loadTurntableCalib(fileName);
  }
}

//////////////////////////////////////////////////////////////////////
bool LaserPanel::loadTurntableCalib(QString calibFile) {
  ENSURE_VISIBLE(false)

    //reset
    _turntableCalib.clear();

  //calib file
  if (calibFile.isEmpty()) {
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    calibFile = dir.absoluteFilePath(TURNTABLE_CALIB_FILENAME);
  }

  //load
  bool rv = _turntableCalib.loadMatlabCalib(calibFile);

  //update GUI
  cameraCalibGroupUpdate();  
  turntableCalibGroupUpdate();  

  return rv;
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_chessboardCornersLoadButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
  QString fileName =
    QFileDialog::getOpenFileName
    (this, "Open Chessboard Corners",
     dir.absoluteFilePath(CHESSBOARD_CORNERS_FILENAME),
     "Matlab Calibration (" CHESSBOARD_CORNERS_FILENAME ")");
  if (!fileName.isEmpty()) {
    loadChessboardCorners(fileName);
  }
}

//////////////////////////////////////////////////////////////////////
bool LaserPanel::loadChessboardCorners(QString calibFile) {
  ENSURE_VISIBLE(false)

  //reset
  _chessboardCorners.clear();

  //calib file
  if (calibFile.isEmpty()) {
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    calibFile = dir.absoluteFilePath(CHESSBOARD_CORNERS_FILENAME);
  }

  //load
  bool rv = _chessboardCorners.loadMatlabCalib(calibFile,true);

  //update GUI
  cameraCalibGroupUpdate();  
  turntableCalibGroupUpdate();  

  return rv;
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::turntableCalibGroupUpdate(void) {
  ENSURE_VISIBLE(_VOID_)

  QString msg;

  if (_turntableCalib.imageAngle.isEmpty()) {
    msg = "Turntable calibration not loaded.";
  } else {
    msg = "Turntable calibration found: '" TURNTABLE_CALIB_FILENAME "'";
  }
  turntableCalibLabel->setText(msg);

  if (_chessboardCorners.R().isEmpty()) {
    msg = "Chessboard Corners not loaded.";
  } else {
    msg =
      "Chessboard Corners found: '" CHESSBOARD_CORNERS_FILENAME "'";
  }
  chessboardCornersLabel->setText(msg);

  chessboardButtonsSetEnable(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::chessboardButtonsSetEnable(bool value) {
  turntableCalibrateSelectedButton->setEnabled(false);
  turntableCalibrateAllButton->setEnabled(false);
  estimateCenterOfRotationButton->setEnabled(false);
  saveTurntablePointsButton->setEnabled(false);
  if(value && _chessboardCorners.isEmpty()==false) {
    turntableCalibrateSelectedButton->setEnabled(true);
    turntableCalibrateAllButton->setEnabled(true);
    if(_chessboardCorners.hasValidTransforms()) {
      estimateCenterOfRotationButton->setEnabled(true);
      saveTurntablePointsButton->setEnabled(true);
    }
  }
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_turntableCalibrateSelectedButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

  //get current item
  QModelIndex selectedIndex;
  auto item = gui_util::getCurrentItem(imageList,&selectedIndex);
  if (!item) { //no item selected
    qCritical("[ERROR] No image is selected.");
    return;
  }
  // int selectedIndex_col = selectedIndex.column();
  // int selectedIndex_row = selectedIndex.row();
  int indx =  selectedIndex.row();

  //get the image file path
  QString imageFilePath = item->data().toString();
  if (imageFilePath.isEmpty()) {
    qCritical("[ERROR] imageFilePath is EMPTY");
    return;
  }

  int nImages = _cameraCalib.imageCount;
  if (nImages<=0 || _cameraCalib.K(2,2)==0.0) {
    qCritical("[ERROR] Camera calibration not found.");
    return;
  }

  if (_chessboardCorners.isEmpty()) {
    qCritical( "[ERROR] Chessboard Corners not found.");
    return;
  }

  //disable gui
  enableGUI(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  // get intrinsic camera calibration parameters
  Matrix3d const& K  = _cameraCalib.K;
  Vector5d const& kc = _cameraCalib.kc;

  std::cerr << "K = [\n";
  std::cerr << K << "\n";
  std::cerr << "]\n";
  std::cerr << "kc = [\n";
  std::cerr << kc << "\n";
  std::cerr << "]\n";

  QVector<Vector2d> const& worldPoints = _chessboardCorners.wPt()[indx];
  QVector<Vector2d> const& imagePoints = _chessboardCorners.iPt()[indx];

  int nWP = worldPoints.size();
  int nIP = imagePoints.size();
  std::cerr << "nWP["<<indx<<"] = " << nWP << "\n";
  std::cerr << "nIP["<<indx<<"] = " << nIP << "\n";
  // assert(nWP==nIP);

  // where to save the result
  Matrix3d&   R  = _chessboardCorners.R()[indx];
  Vector3d&   T  = _chessboardCorners.T()[indx];

  hw3::estimateExtrinsics(K,kc,worldPoints,imagePoints,R,T);

  std::cerr << "R[" << indx << "] = [\n";
  std::cerr << R << "\n";
  std::cerr << "]\n";
  std::cerr << "T[" << indx << "] = [\n";
  std::cerr << T << "\n";
  std::cerr << "]\n";

  //restore cursor
  QApplication::restoreOverrideCursor();

  //enable gui
  enableGUI(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_turntableCalibrateAllButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

  int nImages = _cameraCalib.imageCount;

  if (nImages<=0 || _cameraCalib.K(2,2)==0.0) {
    qCritical("[ERROR] Camera Calibration not found.");
    return;
  }
  
  if (_chessboardCorners.isEmpty()) {
    qCritical("[ERROR] Chessboard Corners not found.");
    return;
  }

  //disable gui
  enableGUI(false);
  // turntableGroup->setEnabled(true);
  // turntableCalibrateSelectedButton->setEnabled(false);
  // turntableCalibrateAllButton->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  // get intrinsic camera calibration parameters
  Matrix3d const& K  = _cameraCalib.K;
  Vector5d const& kc = _cameraCalib.kc;
    
  // std::cerr << "K = [\n";
  // std::cerr << K << "\n";
  // std::cerr << "]\n";
  // std::cerr << "kc = [\n";
  // std::cerr << kc << "\n";
  // std::cerr << "]\n";
    
  int rows = _model.rowCount();

  // TODO Wed Mar  9 20:59:52 2016
  // add new turntableProgress
  // triangulateProgress->setMaximum(rows);
  // triangulateProgress->setVisible(true);

  for (int indx = 0; indx < rows; ++indx) {
    auto item = _model.item(indx, 0);
    if (!item) {
      qCritical("[ERROR] Invalid item.");
      break;
    }

    //get the image file path
    QString imageFilePath = item->data().toString();
    if (imageFilePath.isEmpty()) {
      qCritical("[ERROR] imageFilePath is EMPTY");
      break;
    }
    
    QVector<Vector2d> const& worldPoints = _chessboardCorners.wPt()[indx];
    QVector<Vector2d> const& imagePoints = _chessboardCorners.iPt()[indx];
      
    int nWP = worldPoints.size();
    int nIP = imagePoints.size();
    std::cerr << "nWP["<<indx<<"] = " << nWP << "\n";
    std::cerr << "nIP["<<indx<<"] = " << nIP << "\n";
    // assert(nWP==nIP);
      
    // where to save the result
    Matrix3d&   R  = _chessboardCorners.R()[indx];
    Vector3d&   T  = _chessboardCorners.T()[indx];
      
    hw3::estimateExtrinsics(K,kc,worldPoints,imagePoints,R,T);
      
    std::cerr << "R[" << indx << "] = [\n";
    std::cerr << R << "\n";
    std::cerr << "]\n";
    std::cerr << "T[" << indx << "] = [\n";
    std::cerr << T << "\n";
    std::cerr << "]\n";

    // triangulateProgress->setValue(i);
    QApplication::processEvents();
  }
    
  // triangulateProgress->setVisible(false);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //enable gui
  enableGUI(true);
  // turntableCalibrateSelectedButton->setEnabled(true);
  // turntableCalibrateAllButton->setEnabled(true);
  // estimateCenterOfRotationButton->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_estimateCenterOfRotationButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

  if (_chessboardCorners.isEmpty()) {
    qCritical("[ERROR] Chessboard Corners not found.");
    return;
  }

  // TODO Wed Mar  9 23:51:32 2016
  // make sure that all the rotations and translations have been computed

  //disable gui
  enableGUI(false);
  // turntableGroup->setEnabled(true);
  // turntableCalibrateSelectedButton->setEnabled(false);
  // turntableCalibrateAllButton->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  QVector<Matrix3d>& R = _chessboardCorners.R();
  QVector<Vector3d>& T = _chessboardCorners.T();
  // center of rotation in chessboard plane coordinates
  // it should be the same for every index
  Vector2d& center = _chessboardCorners.centerOfRotation();
  center << 0,0;
  Matrix3d& R_world = _chessboardCorners.worldRotation();
  R_world << 1,0,0,0,1,0,0,0,1; // identity
  Vector3d& T_world = _chessboardCorners.worldTranslation();
  T_world << 0,0,0;

  hw3::estimateTurntableCenter(R,T,center,R_world,T_world);

  // // World Coordinate System
  // // we should have, for all values of indx
  // // R_world = T[0];
  // R_world(0,0) = R[0](0,0); R_world(0,1) = R[0](0,1); R_world(0,2) = R[0](0,2);
  // R_world(1,0) = R[0](1,0); R_world(1,1) = R[0](1,1); R_world(1,2) = R[0](1,2);
  // R_world(2,0) = R[0](2,0); R_world(2,1) = R[0](2,1); R_world(2,2) = R[0](2,2);
  // // T_world = R[indx]*C+T[indx];
  // T_world(0) = R[0](0,0)*C(0)+R[0](0,1)*C(1)+T[0](0); 
  // T_world(1) = R[0](1,0)*C(0)+R[0](1,1)*C(1)+T[0](1); 
  // T_world(2) = R[0](2,0)*C(0)+R[0](2,1)*C(1)+T[0](2); 

  //restore cursor
  QApplication::restoreOverrideCursor();

  //enable gui
  enableGUI(true);
  // turntableCalibrateSelectedButton->setEnabled(true);
  // turntableCalibrateAllButton->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_saveTurntablePointsButton_clicked() {
  ENSURE_VISIBLE(_VOID_)
    
  if (_chessboardCorners.isEmpty()) {
    qCritical("[ERROR] Chessboard Corners not found.");
    return;
  }

  enableGUI(false);
  // turntableGroup->setEnabled(true);
  // turntableCalibrateSelectedButton->setEnabled(false);
  // turntableCalibrateAllButton->setEnabled(false);

  // get intrinsic camera calibration parameters
  Matrix3d const& K  = _cameraCalib.K;
  Vector5d const& kc = _cameraCalib.kc;
    
  // std::cerr << "K = [\n";
  // std::cerr << K << "\n";
  // std::cerr << "]\n";
  // std::cerr << "kc = [\n";
  // std::cerr << kc << "\n";
  // std::cerr << "]\n";
    
  int rows = _model.rowCount();

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  QVector<Matrix3d>& R = _chessboardCorners.R();
  QVector<Vector3d>& T = _chessboardCorners.T();
  // Vector2d& center = _chessboardCorners.centerOfRotation();
  Matrix3d& R_world = _chessboardCorners.worldRotation();
  Vector3d& T_world = _chessboardCorners.worldTranslation();

  QVector<Vector3d> pointcloud;

  for (int indx = 0; indx < rows; ++indx) {
    auto item = _model.item(indx, 0);
    if (!item) {
      qCritical("[ERROR] Invalid item.");
      break;
    }

    //get the image file path
    QString imageFilePath = item->data().toString();
    if (imageFilePath.isEmpty()) {
      qCritical("[ERROR] imageFilePath is EMPTY");
      break;
    }
    
    QVector<Vector2d> const& chessboardPoints = _chessboardCorners.wPt()[indx];
    QVector<Vector2d> const& imagePoints = _chessboardCorners.iPt()[indx];
      
    // int nCBP = chessboardPoints.size();
    // int nIP = imagePoints.size();
    // std::cerr << "nCBP["<<indx<<"] = " << nCBP << "\n";
    // std::cerr << "nIP["<<indx<<"]  = " << nIP << "\n";
    // assert(nWP==nIP);

    QVector<Vector2d> projPoints;
    QVector<Vector3d> worldPoints;
    hw3::reprojectPoints(K,kc,R[indx],T[indx],
                         chessboardPoints,imagePoints,projPoints,
                         R_world,T_world,worldPoints);

    pointcloud.append(worldPoints);
      
    // triangulateProgress->setValue(i);
    QApplication::processEvents();
  }
    
  // triangulateProgress->setVisible(false);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //check result
  if (!pointcloud.isEmpty()) {
    //save
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    savePointcloud(pointcloud, dir.absoluteFilePath("turntablepoints.xyz"));
  } else { //error
      qCritical("[ERROR] No points were generated");
  }

  enableGUI(true);

  turntableCalibrateSelectedButton->setEnabled(true);
  turntableCalibrateAllButton->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////
// Triangulate panel

void LaserPanel::on_triangulateCurrentButton_clicked() {
  ENSURE_VISIBLE(_VOID_)
  
  //get current item
  auto item = gui_util::getCurrentItem(imageList);
  if (!item) { //no item selected
    qCritical("[ERROR] No item is selected.");
    return;
  }

  //get the image file path
  QString imageFilePath = item->data().toString();
  if (imageFilePath.isEmpty()) {
    qCritical("[ERROR] imageFilePath is EMPTY");
    return;
  }

  if (!_cameraCalib.imageCount || _cameraCalib.K(2,2)==0.0) { //no item selected
    qCritical("[ERROR] Camera calibration not found.");
    return;
  }

  if (_turntableCalib.imageAngle.isEmpty()) { //no item selected
    qCritical("[ERROR] Turntable calibration not found.");
    return;
  }

  //disable gui
  enableGUI(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  //triangulate points
  auto const& pointcloud = doTriangulate(imageFilePath);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //check result
  if (!pointcloud.isEmpty()) {
    //save
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    savePointcloud(pointcloud, dir.absoluteFilePath("pointcloud.xyz"));
  } else { //error
    QMessageBox::critical
      (this, "Error", QString("No points were generated: %1").arg(imageFilePath));
  }

  //enable gui
  enableGUI(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_triangulateAllButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

    //disable gui
    enableGUI(false);
  triangulateGroup->setEnabled(true);
  triangulateCurrentButton->setEnabled(false);
  triangulateAllButton->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  int rows = _model.rowCount();
  triangulateProgress->setMaximum(rows);
  triangulateProgress->setVisible(true);
  QVector<Vector3d> pointcloud;
  for (int i = 0; i < rows; ++i) {
    auto item = _model.item(i, 0);
    if (!item) { //invalid item
      qCritical("[ERROR] Invalid item.");
      break;
    }

    //get the image file path
    QString imageFilePath = item->data().toString();
    if (imageFilePath.isEmpty()) {
      qCritical("[ERROR] imageFilePath is EMPTY");
      break;
    }

    //triangulate points
    auto const& points = doTriangulate(imageFilePath);

    //append
    pointcloud.append(points);

    triangulateProgress->setValue(i);
    QApplication::processEvents();
  }
  triangulateProgress->setVisible(false);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //check result
  if (!pointcloud.isEmpty()) {
    //save
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    savePointcloud(pointcloud, dir.absoluteFilePath("pointcloud.xyz"));
  } else { //error
    qCritical("[ERROR] No points were generated.");
  }

  //enable gui
  enableGUI(true);
  triangulateCurrentButton->setEnabled(true);
  triangulateAllButton->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////
QVector<Vector3d> LaserPanel::doTriangulate(QString imageFilePath) {
  ENSURE_VISIBLE(QVector<Vector3d>())

  //load image
  QImage qImg;
  if (!qImg.load(imageFilePath)) {
    QMessageBox::critical
      (this, "Error", QString("Load failed: %1").arg(imageFilePath));
    return QVector<Vector3d>();
  }
  if (qImg.format() != QImage::Format_RGB32) {
    qImg = qImg.convertToFormat(QImage::Format_RGB32);
  }

  qDebug("loaded %s", qPrintable(imageFilePath));

  //camera calib
  auto const& K = _cameraCalib.K;
  auto const& kc = _cameraCalib.kc;

  //turntable calib
  auto const& R = _turntableCalib.Rc;
  auto const& T = _turntableCalib.Rc*(_turntableCalib.center-_turntableCalib.Tc);
  auto const& laserPlane = _turntableCalib.laserPlane;
  double turntableAngle =
    _turntableCalib.imageAngle[QFileInfo(imageFilePath).fileName()];

  //triangulate points
  return hw2::triangulate(K, kc, R, T, laserPlane, turntableAngle, qImg);
}

//////////////////////////////////////////////////////////////////////
// Laser Plane panel

void LaserPanel::on_laserPlaneSelectedButton_clicked() {
  ENSURE_VISIBLE(_VOID_)
  
  //get current item
  QModelIndex selected;
  auto item = gui_util::getCurrentItem(imageList,&selected);
  if (!item) { //no item selected
    qCritical("[ERROR] No item is selected.");
    return;
  }
  int indx = selected.row();

  //get the image file path
  QString imageFilePath = item->data().toString();
  if (imageFilePath.isEmpty()) {
    qCritical("[ERROR] imageFilePath is EMPTY");
    return;
  }

  unsigned nImages = _cameraCalib.imageCount;
  if (nImages==0 || _cameraCalib.K(2,2)==0.0) { //no item selected
    qCritical("[ERROR] Camera calibration not found.");
    return;
  }

  //load image
  QImage qImg;
  if (!qImg.load(imageFilePath)) {
    QMessageBox::critical
      (this, "Error", QString("Load failed: %1").arg(imageFilePath));
    return;
  }
  if (qImg.format() != QImage::Format_RGB32) {
    qImg = qImg.convertToFormat(QImage::Format_RGB32);
  }

  //disable gui
  enableGUI(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  // get the camera calibration intrinsic parameters
  Matrix3d& K   = _cameraCalib.K;
  Vector5d& kc  = _cameraCalib.kc;

  // get the chessboard corners coordinates and corresponding pixel
  // coordinates
  QVector<Vector2d>& imageCoord = _cameraCalib.imageData[indx].imagePoints;
  QVector<Vector3d>& wPts       = _cameraCalib.imageData[indx].worldPoints;
  QVector<Vector2d> chessboardCornersCoord;
  unsigned nPoints = wPts.size();
  for(unsigned i=0;i<nPoints;i++) {
    Vector2d wPt; wPt << wPts[i](0),wPts[i](1);
    chessboardCornersCoord.push_back(wPt);
  }

  // compute the camera extrinsic parameters with respect to this chessboard
  Matrix3d R;
  Vector3d T;
  hw3::estimateExtrinsics(K, kc, chessboardCornersCoord, imageCoord, R, T);

  // detect pixels corresponding to on chessboard points illuminated
  // by the laser; these pixels include lens distortion
  QVector<Vector2d> laserLinePixelCoord;
  hw3::detectLaser(qImg,laserLinePixelCoord);

  // determine the implicit equation of the chessboard plane
  Vector4d chessboardPlane;
  // hw3::chessboardPlane(R, T, chessboardPlane);
  hw3::checkerboardPlane(R, T, chessboardPlane);

  // triangulate the detected pixels; lens distortion should be
  // removed prior to triangulation
  QVector<Vector3d> laserpoints;
  hw3::triangulate(K, kc, chessboardPlane, laserLinePixelCoord, laserpoints);

  //restore cursor
  QApplication::restoreOverrideCursor();

  //check result
  if (!laserpoints.isEmpty()) {
    //save
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    savePointcloud(laserpoints, dir.absoluteFilePath("laserpoints.xyz"));
  } else { //error
    QMessageBox::critical
      (this, "Error", QString("No points were generated: %1").arg(imageFilePath));
  }

  //enable gui
  enableGUI(true);
}

//////////////////////////////////////////////////////////////////////
void LaserPanel::on_laserPlaneAllButton_clicked() {
  ENSURE_VISIBLE(_VOID_)

  unsigned nImages = _cameraCalib.imageCount;
  if (nImages==0 || _cameraCalib.K(2,2)==0.0) { //no item selected
    qCritical("[ERROR] Camera calibration not found.");
    return;
  }

  // get the camera calibration intrinsic parameters
  Matrix3d& K   = _cameraCalib.K;
  Vector5d& kc  = _cameraCalib.kc;

  //disable gui
  enableGUI(false);
  laserPlaneGroup->setEnabled(true);
  laserPlaneSelectedButton->setEnabled(false);
  laserPlaneAllButton->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  laserPlaneProgress->setMaximum(nImages);
  laserPlaneProgress->setVisible(true);
  QVector<Vector3d> pointcloud;
  for (unsigned indx = 0; indx < nImages; ++indx) {

    //get the image file path
    auto item = _model.item(indx, 0);
    if (!item) { //invalid item
      qCritical("[ERROR] Invalid item.");
      break;
    }
    QString imageFilePath = item->data().toString();
    if (imageFilePath.isEmpty()) {
      qCritical("[ERROR] imageFilePath is EMPTY");
      break;
    }

    //load image
    QImage qImg;
    if (!qImg.load(imageFilePath)) {
      QMessageBox::critical
        (this, "Error", QString("Load failed: %1").arg(imageFilePath));
      return;
    }
    if (qImg.format() != QImage::Format_RGB32) {
      qImg = qImg.convertToFormat(QImage::Format_RGB32);
    }

    // get the chessboard corners coordinates and corresponding pixel
    // coordinates
    QVector<Vector2d>& imageCoord = _cameraCalib.imageData[indx].imagePoints;
    QVector<Vector3d>& wPts       = _cameraCalib.imageData[indx].worldPoints;
    QVector<Vector2d> chessboardCornersCoord;
    unsigned nPoints = wPts.size();
    for(unsigned i=0;i<nPoints;i++) {
      Vector2d wPt; wPt << wPts[i](0),wPts[i](1);
      chessboardCornersCoord.push_back(wPt);
    }

    // compute the camera extrinsic parameters with respect to this chessboard
    Matrix3d R;
    Vector3d T;
    hw3::estimateExtrinsics(K, kc, chessboardCornersCoord, imageCoord, R, T);

    // detect pixels corresponding to on chessboard points illuminated
    // by the laser; these pixels include lens distortion
    QVector<Vector2d> laserLinePixelCoord;
    hw3::detectLaser(qImg,laserLinePixelCoord);

    // determine the implicit equation of the chessboard plane
    Vector4d chessboardPlane;
    // hw3::chessboardPlane(R, T, chessboardPlane);
    hw3::checkerboardPlane(R, T, chessboardPlane);

    // triangulate the detected pixels; lens distortion should be
    // removed prior to triangulation
    QVector<Vector3d> laserpoints;
    hw3::triangulate(K, kc, chessboardPlane, laserLinePixelCoord, laserpoints);

    std::cerr << "img["<< indx <<"] -> " << laserpoints.size() << " points\n";

    // append
    pointcloud.append(laserpoints);

    laserPlaneProgress->setValue(indx);
    QApplication::processEvents();
  }
  laserPlaneProgress->setVisible(false);

  // estimate the laser plane
  Vector4d& laserPlane = _turntableCalib.laserPlane;
  laserPlane << 0,0,0,0;
  hw3::estimatePlane(pointcloud,laserPlane);

  std::cerr << "laserPlane = [ "
            << laserPlane(0) << " "
            << laserPlane(1) << " "
            << laserPlane(2) << " "
            << laserPlane(3) << "]\n";

  //restore cursor
  QApplication::restoreOverrideCursor();

  //check result
  if (!pointcloud.isEmpty()) {
    //save
    QDir dir(getApp()->getMainWindow()->getWorkDirectory());
    savePointcloud(pointcloud, dir.absoluteFilePath("laserPoints.xyz"));
  } else { //error
    qCritical("[ERROR] No points were generated.");
  }

  //enable gui
  enableGUI(true);
  laserPlaneSelectedButton->setEnabled(true);
  laserPlaneAllButton->setEnabled(true);
}

//////////////////////////////////////////////////////////////////////
bool LaserPanel::savePointcloud
(QVector<Vector3d> const& pointcloud, QString defaultFilePath) {
  ENSURE_VISIBLE(false)

    QString fileName =
    QFileDialog::getSaveFileName
    (this, "Save Pointcloud", defaultFilePath, "Pointcloud (*.xyz)");
  if (fileName.isEmpty()) {
    return false;
  }

  FILE * fp = fopen(qPrintable(fileName), "w");
  if (!fp) {
    return false;
  }

  foreach(auto const& p, pointcloud) {
    fprintf(fp, "%lg %lg %lg\n", p.x(), p.y(), p.z());
  }

  fclose(fp);
  qDebug("Saved: %s", qPrintable(fileName));
  return true;
}
