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
#include <math.h>

#include "TurntableScanningPanel.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QPainter>
#include <QTimer>
#include <QElapsedTimer>
#include <QDir>
// #include <QtSerialPort/QSerialPort>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "Application.hpp"
#include "MainWindow.hpp"
#include "CameraRecorder.hpp"
#include "SerialPortSettings.hpp"
#include "SerialPortCommandLine.hpp"
#include "LaserTriangulator.hpp"

#include <cam/cam.hpp>
#include <cam/CameraInterface.hpp>

#include <img/util.hpp>
#include <img/yuv.hpp>
#include <img/ImgArgb.hpp>
#include <img/ImgBit.hpp>
#include <img/ImgBitplane.hpp>
#include <img/ImgDraw.hpp>

#include "gui_util.hpp"
#include "matrix_util.hpp"

#include <wrl/SceneGraph.h>

#include <homework/homework4.hpp>
#include <Eigen/Core>
#include <opencv2/core/eigen.hpp>

//////////////////////////////////////////////////////////////////////
// QWidget {

#define DISABLE_SMALL_PREVIEW

bool TurntableScanningPanel::_registered = TurntableScanningPanel::registerPanel();
bool TurntableScanningPanel::registerPanel() {
  qDebug("TurntableScanningPanel: register");
  return MainWindow::registerPanel
    ("Turntable Scanning",
     [](QWidget *parent) -> QWidget*{ return new TurntableScanningPanel(parent); });
  return true;
}

TurntableScanningPanel::TurntableScanningPanel(QWidget * parent, Qt::WindowFlags f) :
  QWidget(parent, f),
  _camera(),
  _imageCount(0),
  _fpsTimer(),
  _model(),
  _captureOneImage(0),
  _serialPortSettings((SerialPortSettings*)0),
  _serialPortCommandLine((SerialPortCommandLine*)0),
  _serial((QSerialPort*)0),
  _recorder((CameraRecorder*)0),
  _currentImage(),
  _currentRotation(0),
  _frameIndex(0),
  _laserIndex(0),
  _oneImageIndex(0),
  _cameraCalib(),
  _turntableCalib(),
  _laser1Calib(),
  _laser2Calib(),
  _laserTriangulator() {

  qDebug("TurntableScanningPanel::TurntableScanningPanel() {");

  setupUi(this);

#ifdef DISABLE_SMALL_PREVIEW
  camWidget->setVisible(false);
#endif // DISABLE_SMALL_PREVIEW

  QSettings config;

  _serial = new QSerialPort(this);
  _serialPortSettings = new SerialPortSettings();
  _serialPortCommandLine = new SerialPortCommandLine(this);

  serialPortButton->setEnabled(true);
  serialPortSettingsButton->setEnabled(true);
  serialPortCommandButton->setEnabled(false);

  // TODO Thu Mar 22 12:01:34 2018
  // bool portIsOpen = !serialPortButton->isChecked();
  // captureButton->setEnabled(portIsOpen);

  serialPortUpdateName();
  connect(_serialPortSettings,&SerialPortSettings::applyPressed,
          this,               &TurntableScanningPanel::serialPortUpdateName);

  // fill camera combo
  gui_util::fillCombo(cameraComboBox, cam::getCameraList(),
                      config.value("TurntableScanningPanel/camera").toString());

  // fill rotation combo
  QStringList rotationList; rotationList << "0" << "90" << "180" << "270";
  gui_util::fillCombo(imageRotCombo, rotationList,
                      config.value("TurntableScanningPanel/rotation").toString());
  camWidget->setRotation(imageRotCombo->currentText().toFloat());

  // signal to update camera info label from threads
  connect(     this, &TurntableScanningPanel::setInfoLabel,
          imageInfo, &QLineEdit::setText);

  // capture group
  countSpin->setRange(1, 1000);
  frameDelaySpin->setRange(0, 10000);
  frameDelaySpin->setSingleStep(100);
  // captureProgress->setVisible(false);
  captureProgress->setVisible(true);
  captureProgress->setValue(0);

  // load other settings
  loadSettings();

  // list view model
  imageList->setModel(&_model);
  auto selectionModel = imageList->selectionModel();
  connect(selectionModel, &QItemSelectionModel::currentChanged,
          this,           &TurntableScanningPanel::_on_currentImageChanged);

  auto mainWin  = getApp()->getMainWindow();
  if(mainWin) {
    auto glWidget = mainWin->getImgGLWidget();
    glWidget->clearImage();
  }

  qDebug("}");
}

TurntableScanningPanel::~TurntableScanningPanel() {
  if (!_camera.isNull()) { //stop

#ifdef DISABLE_SMALL_PREVIEW
    auto camWidget =  getApp()->getMainWindow()->getImgGLWidget();
#endif //DISABLE_SMALL_PREVIEW
#ifdef HAVE_IMG
    disconnect(_camera.data(), &CameraInterface::newImage,
               camWidget, &ImgGLWidget::setImage);
    disconnect(_camera.data(), &CameraInterface::newImage,
               this, &TurntableScanningPanel::updateInfo);
#endif //HAVE_IMG
    gui_util::wait(100); //milliseconds
    _camera.clear();
  }

  saveSettings();
  qDebug(" -- TurntableScanningPanel destroyed -- ");
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::updateState() {

  // TBD ...
  // enable/disable GUI components

}

void TurntableScanningPanel::showEvent(QShowEvent * event) {

  frameNumberLine->setText(QString("%1").arg(_frameIndex));
  frameImageLine->setText(QString("%1").arg(_laserIndex));
  oneImageLine->setText(QString("%1").arg(_oneImageIndex));

  // this block is here because in the constructor
  // getApp()->getMainWindow()==NULL
  static bool firstShow = true;
  std::cerr << "  firstShow = "<< firstShow << "\n";
  auto mainWin = getApp()->getMainWindow();
  if(firstShow) {
    connect(wrlFileLoadButton,  SIGNAL(clicked()),
            mainWin, SLOT(on_wrlFileLoadButton_clicked()));
    connect(wrlFileSaveButton,  SIGNAL(clicked()),
            mainWin, SLOT(on_wrlFileSaveButton_clicked()));
    // connect(turntableFileLoadButton, SIGNAL(clicked()),
    //         mainWin, SLOT(on_turntableFileLoadButton_clicked()));
    firstShow = false;
  }

  mainWin->showImgGLWidget(true);
  mainWin->showWrlGLWidget(true);
  // mainWin->resizeSplitter();

  connect(mainWin, &MainWindow::workDirectoryChanged,
          this,    &TurntableScanningPanel::_on_workDirectoryChanged);

  updateImageFiles();
}

void TurntableScanningPanel::hideEvent(QHideEvent *e) {
  if (e && !e->spontaneous()) {
    if (previewButton->isChecked()) {
      previewButton->setChecked(false);
    }
    auto mainWin = getApp()->getMainWindow();
    disconnect(mainWin, &MainWindow::workDirectoryChanged,
               this,    &TurntableScanningPanel::_on_workDirectoryChanged);
  }
}

// } QWidget
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Settings {

void TurntableScanningPanel::loadSettings(void) {

  QSettings config;

  //current camera
  if (cameraComboBox->currentText() ==
      config.value("TurntableScanningPanel/camera").toString()) {
  }

  //capture group
  countSpin->setValue
    (config.value("TurntableScanningPanel/count", 1).toUInt());
  frameDelaySpin->setValue
    (config.value("TurntableScanningPanel/interval", 500).toUInt());

  QString name              =
    config.value("SerialPortSettings/name","").toString();
  QString baudRate    =
    config.value("SerialPortSettings/baudRate","").toString();
  QString dataBits    =
    config.value("SerialPortSettings/dataBits","").toString();
  QString parity      =
    config.value("SerialPortSettings/parity","").toString();
  QString stopBits    =
    config.value("SerialPortSettings/stopBits","").toString();
  QString flowControl =
    config.value("SerialPortSettings/flowControl","").toString();

  std::cerr << "SerialPortSettings/name = \""
            << name.toUtf8().constData() << "\"" << std::endl;
  std::cerr << "SerialPortSettings/baudRate = \""
            << baudRate.toUtf8().constData() << "\"" << std::endl;
  std::cerr << "SerialPortSettings/dataBits = \""
            << dataBits.toUtf8().constData() << "\"" << std::endl;
  std::cerr << "SerialPortSettings/parity = \""
            << parity.toUtf8().constData() << "\"" << std::endl;
  std::cerr << "SerialPortSettings/stopBits = \""
            << stopBits.toUtf8().constData() << "\"" << std::endl;
  std::cerr << "SerialPortSettings/flowControl = \""
            << flowControl.toUtf8().constData() << "\"" << std::endl;

  _serialPortSettings->selectName(name);
  _serialPortSettings->selectBaudRate(baudRate);
  _serialPortSettings->selectDataBits(dataBits);
  _serialPortSettings->selectParity(parity);
  _serialPortSettings->selectStopBits(stopBits);
  _serialPortSettings->selectFlowControl(flowControl);
  _serialPortSettings->apply();

  // chessboard group
  chessboardColsSpin->setValue
    (config.value("TurntableScanningPanel/chessboard/cols",7).toInt());
  chessboardRowsSpin->setValue
    (config.value("TurntableScanningPanel/chessboard/rows",9).toInt());
  double cellWidth = 
    config.value("TurntableScanningPanel/chessboard/width",20.0).toDouble();
  double cellHeight = 
    config.value("TurntableScanningPanel/chessboard/height",20.0).toDouble();
  chessboardCellWidthValue->setText(QString("%1").arg(cellWidth));
  chessboardCellHeightValue->setText(QString("%1").arg(cellHeight));
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::saveSettings(void) {
  QSettings config;

  //current camera
  config.setValue("TurntableScanningPanel/camera", cameraComboBox->currentText());
  config.setValue("TurntableScanningPanel/rotation", imageRotCombo->currentText());

  //capture group
  config.setValue("TurntableScanningPanel/count", countSpin->value());
  config.setValue("TurntableScanningPanel/interval", frameDelaySpin->value()); 

  // serial port
  SerialPortSettings::Settings spSettings = _serialPortSettings->settings();
  config.setValue("SerialPortSettings/name"       , spSettings.name);
  config.setValue("SerialPortSettings/baudRate"   , spSettings.stringBaudRate);
  config.setValue("SerialPortSettings/dataBits"   , spSettings.stringDataBits);
  config.setValue("SerialPortSettings/parity"     , spSettings.stringParity);
  config.setValue("SerialPortSettings/stopBits"   , spSettings.stringStopBits);
  config.setValue("SerialPortSettings/flowControl", spSettings.stringFlowControl);

  // chessboard group
  config.setValue("TurntableScanningPanel/chessboard/cols",
                  chessboardColsSpin->value());
  config.setValue("TurntableScanningPanel/chessboard/rows",
                  chessboardRowsSpin->value());
  config.setValue("TurntableScanningPanel/chessboard/width",
                  chessboardCellWidthValue->text().toDouble());
  config.setValue("TurntableScanningPanel/chessboard/height",
                  chessboardCellHeightValue->text().toDouble());

}

// } Settings
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Camera Group {

void TurntableScanningPanel::on_previewButton_clicked(bool checked) {

  auto mainWin = getApp()->getMainWindow();

  //update the GUI
  cameraComboBox->setEnabled(!checked);
  countSpin->setEnabled(!checked);
  frameDelaySpin->setEnabled(!checked);
  captureButton->setText((checked?"ONE":"START"));
  mainWin->setChangesEnabled(!checked);

#ifdef DISABLE_SMALL_PREVIEW
  auto camWidget = mainWin->getImgGLWidget();
#endif //DISABLE_SMALL_PREVIEW

  if (checked) { //start
    getAndSetupCamera();
    bool error = _camera.isNull();
    if (!error) {
      imageList->setEnabled(false);
#ifdef HAVE_IMG
      connect(_camera.data(), &CameraInterface::newImage,
              this, &TurntableScanningPanel::updateInfo, Qt::DirectConnection);
      connect(_camera.data(), &CameraInterface::newImage,
              camWidget, &ImgGLWidget::setImage, Qt::DirectConnection);
#endif //HAVE_IMG
      _imageCount = 0; _fpsTimer.start();
      camWidget->setRotation(imageRotCombo->currentText().toFloat());
      error = (_camera->start() != true);
      _captureOneImage.store(0);
    }
    if (error) { //cannot get a camera: abort
      qDebug("[ERROR] Cannot find camera or camera didn't start");
      previewButton->blockSignals(true);
      previewButton->setChecked(false);
      previewButton->blockSignals(false);
    }
  } else { //stop
#ifdef HAVE_IMG
    disconnect(_camera.data(), &CameraInterface::newImage,
               camWidget, &ImgGLWidget::setImage);
    disconnect(_camera.data(), &CameraInterface::newImage,
               this, &TurntableScanningPanel::updateInfo);
#endif //HAVE_IMG
    gui_util::wait(100); //milliseconds
    imageList->setEnabled(true);
    _camera.clear();
  }
}

void TurntableScanningPanel::getAndSetupCamera(void) {

  //get camera
  _camera = cam::getCamera(cameraComboBox->currentText());

  //setup camera
  if (!_camera.isNull()) {
    //add formats to context menu
    updateCameraResolutions();
  }
}

void TurntableScanningPanel::updateCameraResolutions(void) {

  QStringList resolutionList;
  if (!_camera.isNull()) {
    resolutionList = _camera->getFormatList();
  }

  //add new actions
  QMap<QString,bool> used;
  foreach (auto res, resolutionList) {
    //parse fmt:widthxheight
    auto sp = res.split(":");
    //if (sp.size()>1 && sp.at(0).contains("RGB") || sp.at(0).contains("BGR"))
    QString fmt = sp.at(0)+":"+sp.at(1);
    if (!used[fmt]) {
      used[fmt] = true;
      QAction * action = new QAction(fmt, this);
      connect(action, &QAction::triggered,
              this, &TurntableScanningPanel::changeCameraResolution);
    }
  }
}

void TurntableScanningPanel::changeCameraResolution() {
  QAction * action = qobject_cast<QAction *>(sender());
  if (!action) { return; }
}

#ifdef HAVE_IMG
void TurntableScanningPanel::updateInfo(ImageBuffer const& image) {
  QString infoText = "No image.";
  if (image.data) {
    double fps =
      1000.0*static_cast<double>
      (_imageCount)/static_cast<double>(_fpsTimer.elapsed());

    ++_imageCount;

    // .arg(++_imageCount)
    // infoText =
    //   // QString("FPS %1 FORMAT %2 SIZE %3x%4")
    //   QString("%1:%2:%3x%4")
    //   .arg(fps)
    //   .arg(image.formatName(image.format))
    //   .arg(image.cols)
    //   .arg(image.rows);
    
    imageFormat->setText(QString("%1").arg(image.formatName(image.format)));
    imageSize->setText(QString("%1x%2").arg(image.cols).arg(image.rows));

    infoText = QString("%1").arg(fps);

    if (_fpsTimer.elapsed()>30000) {
      _imageCount = 0;
      _fpsTimer.start();
    }

    if (_captureOneImage.testAndSetRelaxed(1,0)) { //save image

      // if(resetImageCounter->isChecked()) _oneImageIndex = 0;

      QString defaultImageName =
        QString::asprintf(qPrintable("F_%03d.png"),_oneImageIndex++);

      QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
      QString fileName =
        QFileDialog::getSaveFileName
        (this, "Save Image", workDir.absoluteFilePath(defaultImageName),
         "Images (*.png *.jpg)");

      if (!fileName.isEmpty()) {
        int rotation = imageRotCombo->currentText().toInt();

        ImageBuffer outImage;
        if (image.format==ImageBuffer::NV12) {
          ImageBuffer rgbImage;
          img::rgbFromNV12(image, rgbImage);
          img::rotateImage(rgbImage,0,outImage);
        } else {
          img::rotateImage(image,0,outImage);
        }

        if (rotation!=0) {
          ImageBuffer rotatedImage;
          img::rotateImage(outImage,rotation, rotatedImage);
          img::rotateImage(rotatedImage,0,outImage);
        }

        img::save(fileName,outImage);

        qDebug("Saved '%s'", qPrintable(fileName));
      }
      updateImageFiles();
    }
  }

  //use a signal so that it gets updated in the main thread
  emit setInfoLabel(infoText);

  //update image rotation
  camWidget->setRotation(imageRotCombo->currentText().toFloat());

}
#endif //HAVE_IMG

void TurntableScanningPanel::on_cameraCombo_currentIndexChanged(int index) {
  updateCameraResolutions();
}

void TurntableScanningPanel::on_imageRotCombo_currentIndexChanged(int index) {
#ifdef DISABLE_SMALL_PREVIEW
  auto mainWin = getApp()->getMainWindow();
  auto camWidget = this->camWidget;
  if (mainWin) { camWidget = mainWin->getImgGLWidget(); }
#endif //DISABLE_SMALL_PREVIEW
  camWidget->setRotation(imageRotCombo->currentText().toFloat());
}

// } Camera Group
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Capture Group {

void TurntableScanningPanel::on_blankFrameCheck_stateChanged() {
  int nImagesPerFrame = 0;
  if(blankFrameCheck->isChecked()) nImagesPerFrame++;
  if(laser1FrameCheck->isChecked()) nImagesPerFrame++;
  if(laser2FrameCheck->isChecked()) nImagesPerFrame++;
  if(nImagesPerFrame==0) {
    blankFrameCheck->blockSignals(true);
    blankFrameCheck->setChecked(true);
    blankFrameCheck->blockSignals(false);
    nImagesPerFrame = 1;
  }
  imagesPerFrameLine->setText(QString("%1").arg(nImagesPerFrame));
}

void TurntableScanningPanel::on_laser1FrameCheck_stateChanged() {
  on_blankFrameCheck_stateChanged();
}

void TurntableScanningPanel::on_laser2FrameCheck_stateChanged() {
  on_blankFrameCheck_stateChanged();
}

int TurntableScanningPanel::getLaserMode() {
  std::cerr << "TurntableScanningPanel::getLaserMode() {\n";
  int laserMode = 0;
  if(blankFrameCheck->isChecked() ) laserMode |= 0x1;
  if(laser1FrameCheck->isChecked()) laserMode |= 0x2;
  if(laser2FrameCheck->isChecked()) laserMode |= 0x4;
  std::cerr << "  laserMode = " << laserMode << "\n";
  std::cerr << "}\n";
  return laserMode;
}

void TurntableScanningPanel::on_frameNumberLine_returnPressed() {
  int frameIndex = frameNumberLine->text().toInt();
  if(frameIndex>=0) {
    _frameIndex = frameIndex;
  } else {
    _frameIndex = 0;
    frameNumberLine->setText("0");
  }
}

void TurntableScanningPanel::on_oneImageLine_returnPressed() {
  int oneImageIndex = oneImageLine->text().toInt();
  if(oneImageIndex>=0) {
    _oneImageIndex = oneImageIndex;
  } else {
    _oneImageIndex = 0;
    oneImageLine->setText("0");
  }
}

void TurntableScanningPanel::on_captureButton_clicked(bool checked) {

  if (previewButton->isChecked()) {

    // preview is running, just capture one image
    _captureOneImage.store(1);

  } else if(checked) {

    // start the capture thread

    // create worker object
    CameraRecorder * recorder = new CameraRecorder(this);
    _recorder = recorder;

    recorder->setTurntableScanningPanel(this);
    recorder->setCameraName(cameraComboBox->currentText());
    
    int laserMode = getLaserMode();
    recorder->setLaserMode(laserMode);
    
    // int imagesPerFrame = 0;
    // if(blankFrameCheck->isChecked()) imagesPerFrame++;
    // if(laser1FrameCheck->isChecked()) imagesPerFrame++;
    // if(laser2FrameCheck->isChecked()) imagesPerFrame++;
    int imagesPerFrame = imagesPerFrameLine->text().toInt();
    int count      = imagesPerFrame*countSpin->value();
    recorder->setImageCount(count);
    
    int frameDelay = frameDelaySpin->value();
    recorder->setCaptureInterval(frameDelay);
    
    int rotation   = imageRotCombo->currentText().toInt();
    recorder->setImageRotation(rotation);
    
    auto mainWin = getApp()->getMainWindow();
    recorder->setImagePath(mainWin->getWorkDirectory());

    recorder->setIndexFirstImage(imagesPerFrame*_frameIndex+_laserIndex);
    
    connect(recorder, &CameraRecorder::started,
            this, &TurntableScanningPanel::_on_captureStarted);
    connect(recorder, &CameraRecorder::progress,
            this, &TurntableScanningPanel::_on_captureProgress);
    connect(recorder, &CameraRecorder::finished,
            this, &TurntableScanningPanel::_on_captureFinished);

    recorder->start();

  } else /* if(!checked) */ {

    // stop the running capture thread

    if(_recorder!=(CameraRecorder*)0) {
      _recorder->stop();
      int indx = _recorder->getIndex();
      std::cerr << "  stopped @ index = " << indx << " \n";
      _recorder = (CameraRecorder*)0;
    }

  }
}

// connect(recorder, &CameraRecorder::started,
//         this,     &TurntableScanningPanel::_on_captureStarted);

void TurntableScanningPanel::_on_captureStarted(void) {

  std::cerr << "TurntableScanningPanel::_on_captureStarted(void) {\n";

  // executed in the CameraRecorder thread :
  // sendTurntableCommand("S ",100);

  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (!recorder) { return; }

  int index = recorder->getIndex();
  std::cerr << "  started @ index = " << index << " \n";
  int imagesPerFrame = imagesPerFrameLine->text().toInt();
  int frame = index/imagesPerFrame;
  int laser   = index%imagesPerFrame;
  frameNumberLine->setText(QString("%1").arg(frame));
  frameImageLine->setText(QString("%1").arg(laser));

  // sendTurntableCommand("GF 0 0 ",1000);

  auto mainWin = getApp()->getMainWindow();

  // update the GUI
  // captureButton->setEnabled(false);
  captureButton->setText("STOP");

  cameraComboBox->setEnabled(false);
  imageRotCombo->setEnabled(false);
  previewButton->setEnabled(false);
  // turntableCommand->setEnabled(false);
  mainWin->setChangesEnabled(false);

  int total = recorder->getImageCount();

  captureProgress->setMaximum(total);
  captureProgress->setVisible(true);
  _model.clear();

  std::cerr << "}\n";
}

// connect(recorder, &CameraRecorder::progress,
//         this,     &TurntableScanningPanel::_on_captureProgress);

void TurntableScanningPanel::_on_captureProgress(void) {

  std::cerr << "TurntableScanningPanel::_on_captureProgress(void) {\n";

  // executed in the CameraRecorder thread :

  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (!recorder) { return; }

  //////////////////////////////
  // update the gui

  // 0={L1:off,L2:off} 0={L1:on,L2:off} 2={L1:off,L2:on} 3={L1:on,L2:on}
  int  laserMode      = recorder->getLaserMode();
  bool blankImg       = ((laserMode&0x1)!=0); // 0
  bool laser1Img      = ((laserMode&0x2)!=0); // 1
  bool laser2Img      = ((laserMode&0x3)!=0); // 2
  int  imageIndex     = recorder->getIndex();
  int  imagesPerFrame = imagesPerFrameLine->text().toInt();
  int  frame          = imageIndex/imagesPerFrame;
  int  indx           = imageIndex%imagesPerFrame; // 0<=indx<imagesPerFrame

  int laser = 0;
  switch(imagesPerFrame) {
  case 1: // indx==0
    laser = (laser2Img)?2:(laser1Img)?1:0;
    break;
  case 2: // indx==0 || indx==1
    // blank+laser1, blank+laser2, laser1+laser2 
    if(indx==0) { 
      laser = (blankImg)?0:1;
    } else /* if(indx==1) */ {
      laser = (laser2Img)?2:1;
    }
    break;
  case 3: // indx==0 || indx==1 || indx==2
    laser = indx;
    break;
  }

  std::cerr << " index = " << imageIndex
            << " frame = " << frame
            << " laser = " << laser << " \n";

  frameNumberLine->setText(QString("%1").arg(frame));
  frameImageLine->setText(QString("%1").arg(laser));

  //////////////////////////////
  // add the filename to the image list

  auto list = recorder->getImageNameList();
  int curr = list.length();

  captureProgress->setValue(curr);
  
  QDir dir(recorder->getImagePath());

  QStandardItem *item = new QStandardItem(list.last());
  item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
  item->setData(dir.absoluteFilePath(item->text()));
  _model.invisibleRootItem()->appendRow(item);

  //////////////////////////////
  // display the last image 

  if (item) { //display
    QString imageName = item->text();
    QString imagePath = item->data().toString();

    std::cerr << "  imageName = \""<< imageName.toUtf8().constData() <<"\" \n";
    std::cerr << "  imagePath =\n";
    std::cerr << "  \""<< imagePath.toUtf8().constData() <<"\" \n";

    // QDir dir(imagePath);
    // QString filename = dir.absoluteFilePath(imageName);
    // std::cerr << "  filename =\n";
    // std::cerr << "  \""<< filename.toUtf8().constData() <<"\" \n";
    QString& filename = imagePath;

    QImage qImg;
    if (qImg.load(filename)) {
      std::cerr <<"  loaded\n";
      updateImage(qImg);
    } else {
      std::cerr <<"  NOT loaded\n";
    }
  }

  std::cerr << "}\n";
}

// connect(recorder, &CameraRecorder::finished,
//         this,     &TurntableScanningPanel::_on_captureFinished);

void TurntableScanningPanel::_on_captureFinished(void) {

  std::cerr << "TurntableScanningPanel::_on_captureFinished(void) {\n";

  auto mainWin = getApp()->getMainWindow();

  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (recorder) {
    auto error = recorder->getError();
    auto message = recorder->getErrorMessage();
    qDebug
      ("[TurntableScanningPanel::_on_captureFinished] error %s, message '%s'",
       (error?"TRUE":"FALSE"), qPrintable(message));

    if (error) {
      QMessageBox::critical
        (this, "Error", "Capture failed with message '" + message + "'");
    }
  }

  int imageIndex = recorder->getIndex();
  std::cerr << "  finished @ index = " << imageIndex << " \n";
  
  // update the GUI
  cameraComboBox->setEnabled(true);
  imageRotCombo->setEnabled(true);
  previewButton->setEnabled(true);

  // TODO Thu Mar 22 2018
  // bool portIsOpen = !serialPortButton->isChecked();
  // captureButton->setEnabled(portIsOpen);
  captureButton->setText("START");
  captureButton->setChecked(false);

  mainWin->setChangesEnabled(true);
  // captureProgress->setVisible(false);
  captureProgress->setVisible(true);
  captureProgress->setValue(0);

  updateImageFiles();

  std::cerr << "}\n";
}

// } Capture Group
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Images Group {

void TurntableScanningPanel::_on_workDirectoryChanged() {
  updateImageFiles();
}

void TurntableScanningPanel::_on_currentImageChanged
(const QModelIndex & current, const QModelIndex & previous) {
  Q_UNUSED(current);
  Q_UNUSED(previous);
  QImage qImg = gui_util::loadCurrentImage(imageList);
  updateImage(qImg,0);
}

// updateImage() is called by
// _on_captureProgress
// _on_currentImageChanged

void TurntableScanningPanel::updateImage(QImage image, int rotation) {

  auto mainWin = getApp()->getMainWindow();
  if (!mainWin) {
    return;
  }
  auto glWidget = mainWin->getImgGLWidget();
  
  if (image.isNull()) {
    glWidget->clearImage();
    return;
  }

  // save last (QImage,rotation) pair
  _currentImage = image;
  _currentRotation = rotation;

  //update image
  glWidget->setRotation(rotation);
  glWidget->setQImage(image);
  QTimer::singleShot(10, glWidget, SLOT(update()));
}

void TurntableScanningPanel::updateImageFiles() {
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

void TurntableScanningPanel::on_imagesUpdateButton_clicked() {
  std::cerr << "TurntableScanningPanel::on_imagesUpdateButton_clicked() {\n";
  updateImageFiles();
  std::cerr << "}\n";
}

void TurntableScanningPanel::deleteAllImageFiles() {

  auto mainWin = getApp()->getMainWindow();
  
  //get files
  QStringList fileList;
  QDir dir(mainWin->getWorkDirectory());
  auto filters(QStringList() << "*.jpg" << "*.bmp" << "*.png");
  auto list =
    dir.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);
  foreach(auto file, list) {
    dir.remove(file.absoluteFilePath());
  }
  
  //update the model
  gui_util::updateFilesItems(&_model, fileList);
}

void TurntableScanningPanel::on_imagesDeleteAllButton_clicked() {
  std::cerr << "TurntableScanningPanel::on_imagesDeleteAllButton_clicked() {\n";
  deleteAllImageFiles();
  std::cerr << "}\n";
}

void TurntableScanningPanel::on_imagesNextButton_clicked() {
  QAbstractItemModel *model = imageList->model();
  int nRows = model->rowCount();
  if(nRows>0) {
    QModelIndex indx = imageList->currentIndex();
    int row = 0;
    if(indx.isValid()) {
      row = indx.row()+1;
      if(row==nRows) row = 0;
    }
    QModelIndex nextIndex = model->index(row,0);
    imageList->setCurrentIndex(nextIndex);
  }
}

void TurntableScanningPanel::on_imagesPreviousButton_clicked() {
  QAbstractItemModel *model = imageList->model();
  int nRows = model->rowCount();
  if(nRows>0) {
    QModelIndex indx = imageList->currentIndex();
    int row = 0;
    if(indx.isValid()) {
      row = indx.row()-1;
      if(row<0) row = nRows-1;
    }
    QModelIndex nextIndex = model->index(row,0);
    imageList->setCurrentIndex(nextIndex);
  }
}

// } Images Group
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Turntable Group {

void TurntableScanningPanel::serialPortWriteData(const QByteArray &data) {
  // std::cerr << "TurntableScanningPanel::serialPortWriteData() {\n";
  qint64 bytesWritten =_serial->write(data);
  if (bytesWritten == -1) {
    std::cerr << "TurntableScanningPanel::serialPortWriteData() : "
              << "Failed to write the data" << std::endl;
  } else if (bytesWritten != data.size()) {
    std::cerr << "TurntableScanningPanel::serialPortWriteData() : "
              << "Failed to write all the data" << std::endl;
  } else if (!_serial->waitForBytesWritten(5000)) {
    std::cerr << "TurntableScanningPanel::serialPortWriteData() : "
      "Operation timed out or an error occurred" << std::endl;
  } else {
    // std::cerr << "  OK\n";
  }
  // std::cerr << "}\n";
}

QByteArray TurntableScanningPanel::serialPortReadData(int msec) {

  QByteArray readData;

  std::cerr << "TurntableScanningPanel::serialPortReadData() {\n";
  std::cerr << "  msec = " << msec << "\n";

  // bool QSerialPort::waitForReadyRead(int msecs = 30000)
  //
  // This function blocks until new data is available for reading and
  // the readyRead() signal has been emitted. The function will
  // timeout after msecs milliseconds; the default timeout is 30000
  // milliseconds. If msecs is -1, this function will not time out.
  //
  // The function returns true if the readyRead() signal is emitted
  // and there is new data available for reading; otherwise it returns
  // false (if an error occurred or the operation timed out).

  QElapsedTimer timer;
  timer.start();

  if(_serial->waitForReadyRead(msec)) {
    readData += _serial->readAll();
    while(_serial->waitForReadyRead(10))
      readData += _serial->readAll();
  } else {
    std::cerr << "  timed out" << "\n";
  }

  std::cerr << "  elapsed time = " << timer.elapsed() << "\n";
  std::cerr << "}\n";

  return readData;
}

void TurntableScanningPanel::serialPortUpdateName() {
  SerialPortSettings::Settings p = _serialPortSettings->settings();
  serialPortName->setText(tr("%1").arg(p.name));
}

void TurntableScanningPanel::on_serialPortSettingsButton_clicked() {
  if(_serialPortSettings->isVisible())
    _serialPortSettings->setVisible(false);
  else
    _serialPortSettings->setVisible(true);
}
void TurntableScanningPanel::on_serialPortCommandButton_clicked() {
  if(_serialPortCommandLine->isVisible())
    _serialPortCommandLine->setVisible(false);
  else
    _serialPortCommandLine->setVisible(true);
}

void TurntableScanningPanel::on_serialPortButton_clicked(bool checked) {
  if(!checked) { // port is open : close it

    if (_serial->isOpen()) _serial->close();

    serialPortSettingsButton->setEnabled(true);
    serialPortCommandButton->setEnabled(false);
    _serialPortCommandLine->setVisible(false);

    // TODO Thu Mar 22 2018
    // bool portIsOpen = !serialPortButton->isChecked();
    // captureButton->setEnabled(portIsOpen);

    serialPortStatus->setText(tr("Disconnected"));
    serialPortButton->setText("Open");

  } else { // port is closed : open it

    SerialPortSettings::Settings p = _serialPortSettings->settings();
    _serial->setPortName(p.name);
    _serial->setBaudRate(p.baudRate);
    _serial->setDataBits(p.dataBits);
    _serial->setParity(p.parity);
    _serial->setStopBits(p.stopBits);
    _serial->setFlowControl(p.flowControl);
    if (_serial->open(QIODevice::ReadWrite)) {
      
      serialPortSettingsButton->setEnabled(false);
      serialPortCommandButton->setEnabled(true);

      // TODO Thu Mar 22 2018
      // bool portIsOpen = !serialPortButton->isChecked();
      // captureButton->setEnabled(portIsOpen);

      // serialPortStatus->setText
      //   (tr("Connected @ B=%1 D=%2 P=%3 S=%4 F=%5")
      //    .arg(p.stringBaudRate).arg(p.stringDataBits)
      //    .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

      serialPortStatus->setText(tr("Connected @ %1").arg(p.stringBaudRate));
      serialPortButton->setText("Close");

    } else {
      QMessageBox::critical(this, tr("Error"), _serial->errorString());
      serialPortStatus->setText(tr("Open error"));
    }
  }
}

QByteArray TurntableScanningPanel::sendTurntableCommand(QString cmd, int msec) {
  std::cerr << "TurntableScanningPanel::sendTurntableCommand() {\n";
  QByteArray response;
  QByteArray command = cmd.toUtf8();
  int commandLength = command.length();
  if(commandLength>=1) {
    char lastChar = command[commandLength-1];
    if(lastChar!=' ') command.append(' ');
    std::cerr << "  command = \"" << command.constData() << "\"\n";
    serialPortWriteData(command);
    if(msec>0) {
      response = serialPortReadData(msec);
      std::cerr << "  response = \"" << response.constData() << "\"\n";
    }
  }
  std::cerr << "}\n";
  return response;
}

// } Turntable Group
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Calibrate Group {

TurntableScanningPanel::CalibrationMode
TurntableScanningPanel::getCalibrationMode() {
  if(calibrateLaser2Check->isChecked()) {
    return CalibrationMode::CalibrateLaser2;
  } else if(calibrateLaser1Check->isChecked()) {
    return CalibrationMode::CalibrateLaser1;
  } else if(calibrateTurntableCheck->isChecked()) {
    return CalibrationMode::CalibrateTurntable;
  } else /*if(calibrateCameraCheck->isChecked()) */ {
    return CalibrateCamera;
  }
}

void TurntableScanningPanel::setCalibrationMode(const CalibrationMode mode) {
  calibrateCameraCheck->blockSignals(true);
  calibrateTurntableCheck->blockSignals(true);
  calibrateLaser1Check->blockSignals(true);
  calibrateLaser2Check->blockSignals(true);

  calibrateCameraCheck->setChecked(false);
  calibrateTurntableCheck->setChecked(false);
  calibrateLaser1Check->setChecked(false);
  calibrateLaser2Check->setChecked(false);
  switch(mode) {
  case CalibrateCamera:
    calibrateCameraCheck->setChecked(true);
    break;
  case CalibrateTurntable:
    calibrateTurntableCheck->setChecked(true);
    break;
  case CalibrateLaser1:
    calibrateLaser1Check->setChecked(true);
    break;
  case CalibrateLaser2:
    calibrateLaser2Check->setChecked(true);
    break;
  }

  calibrateCameraCheck->blockSignals(false);
  calibrateTurntableCheck->blockSignals(false);
  calibrateLaser1Check->blockSignals(false);
  calibrateLaser2Check->blockSignals(false);
}

void TurntableScanningPanel::on_calibrateCameraCheck_stateChanged() {
  setCalibrationMode(CalibrateCamera);
}

void TurntableScanningPanel::on_calibrateTurntableCheck_stateChanged() {
  setCalibrationMode(CalibrateTurntable);
}

void TurntableScanningPanel::on_calibrateLaser1Check_stateChanged() {
  setCalibrationMode(CalibrateLaser1);
}

void TurntableScanningPanel::on_calibrateLaser2Check_stateChanged() {
  setCalibrationMode(CalibrateLaser2);
}

//////////////////////////////////////////////////////////////////////
CalibrationData & TurntableScanningPanel::calibrationData() {
  if(calibrateLaser2Check->isChecked()) {
    return _laser2Calib;
  } else if(calibrateLaser1Check->isChecked()) {
    return _laser1Calib;
  } else if(calibrateTurntableCheck->isChecked()) {
    return _turntableCalib;
  } else /* if(calibrateCameraCheck->isChecked()) */ {
    return _cameraCalib;
  }
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::chessboardDetectCurrent
(bool interactive, bool displayCorners) {

  CalibrationData & calibData = calibrationData();

  if(interactive)
    std::cerr << "TurntableScanningPanel::chessboardDetectCurrent() {\n";

  // get the number of images in the image list
  QAbstractItemModel *model = imageList->model();
  int nImages = model->rowCount();
  if(interactive)
    std::cerr << "  nImages = " << nImages << "\n";
  if(nImages<=0) {
    if(interactive) {
      QMessageBox::critical(this, "Error", "Empty list of images.");
      std::cerr << "  empty list of images\n";
      std::cerr << "}\n";
    }
    return;
  }

  // get the index of the currently selected image
  QModelIndex modelIndex;
  auto item = gui_util::getCurrentItem(imageList, &modelIndex);
  if (!item) { //no item selected
    if(interactive) {
      QMessageBox::critical(this, "Error", "No item is selected.");
      std::cerr << "  no item selected\n";
      std::cerr << "}\n";
    }
    return;
  }
  int iImage  = (modelIndex.isValid())?modelIndex.row():0;
  if(interactive)
    std::cerr << "  iImage  = " << iImage << "\n";

  QFileInfo info(modelIndex.data().toString());
  auto imageName = info.fileName();
  if(interactive)
    std::cerr << "  imageName = " << qPrintable(imageName) << "\n";

  // auto executablePath = info.absolutePath();
  // std::cerr << "  executablePath = " << qPrintable(executablePath) << "\n";

  // QImage _currentImage;
  int width  = _currentImage.width();
  int height = _currentImage.height();
  cv::Size  imageSize(width,height);
  cv::Mat1b intensityImage(imageSize);

  // convert _currentImage to intensityImage
  for(int y=0;y<height;y++) {
    for(int x=0;x<width;x++) {
      intensityImage.at<uchar>(y,x) = qGray(_currentImage.pixel(x,y));
    }
  }

  int chessboardRows = chessboardRowsSpin->value();
  int chessboardCols = chessboardColsSpin->value();
  cv::Size cornerCount(chessboardCols,chessboardRows);

  double cellWidth  = chessboardCellWidthValue->text().toDouble();
  double cellHeight = chessboardCellHeightValue->text().toDouble();
  cv::Size2f cornerSize(cellWidth,cellHeight);

  std::vector<cv::Point3f> cornersWorld;
  std::vector<cv::Point2f> cornersCamera;

  int nCornersExpected = chessboardRows*chessboardCols;
  int nCornersDetected =
    hw4::detectChessboardCorners
    (intensityImage,cornerCount,cornerSize,cornersWorld, cornersCamera);

  if(nCornersDetected!=nCornersExpected) {
    if(interactive) {
      QMessageBox::critical
        (this, "Error",
         QString("Detected only %1 Corners. Expected %2.")
         .arg(nCornersDetected)
         .arg(nCornersExpected));
      std::cerr << "  detected "<<nCornersDetected
                <<" corners out of "<<nCornersExpected<<" expected\n";
      std::cerr << "}\n";
    }
    return;
  }
  
  if (displayCorners) {
    QPainter painter(&_currentImage);
    QPen pen;
    // pen.setWidth(8);
    // pen.setColor(Qt::red);
    // painter.setPen(pen);
    qreal red,green,blue;
    QColor penColor;
    int radius   = 4;
    int diameter = 2*radius;

    //   0 - tCol - chessboardCols-1
    //   |           |
    //  red  ------ yellow - 0
    //   |           |     - tRow
    // green ------ blue --- chessboardRows-1

    // foreach(cv::Point2f const& p, cornersCamera) {
    //   painter.drawEllipse(p.x-radius, p.y-radius, diameter, diameter);

    for(int row=0;row<chessboardRows;row++) {
      float tRow = ((float)row)/((float)(chessboardRows-1));
      for(int col=0;col<chessboardCols;col++) {
        float tCol = ((float)col)/((float)(chessboardCols-1));

        // color = (1-tRow)*(1-tCol)*red   + (1-tRow)*(  tCol)*yellow
        //       + (  tRow)*(1-tCol)*green + (  tRow)*(1-tCol)*blue

        // (r,g,b) = (1-tRow)*(1-tCol)*(1,0,0) + (1-tRow)*(  tCol)*(1,1,0)
        //         + (  tRow)*(1-tCol)*(0,1,0) + (  tRow)*(  tCol)*(0,0,1)

        red   = 1.0f-tRow;
        green = (1.0f-tRow)*tCol+tRow*(1.0f-tCol);
        blue  = tRow*tCol;
        penColor.setRgbF(red,green,blue);
        pen.setColor(penColor);
        pen.setWidth(8);
        painter.setPen(pen);

        cv::Point2f const& p = cornersCamera[col+row*chessboardCols];
        painter.drawEllipse(p.x-radius, p.y-radius, diameter, diameter);

      }
    }
  }

  // save image with detected corners ???
  // QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
  // _currentImage.save("detected.png");

  // update calib 
  calibData.imageData.resize(static_cast<int>(nImages));
  calibData.imageCount = nImages;
  calibData.imageSize.setWidth(width);
  calibData.imageSize.setHeight(height);

  CalibrationDataCamera::ImageData & data = calibData.imageData[iImage];
  data.imageName = imageName;

  // copy cornersWorld onto data.worldPoints
  data.worldPoints.clear();
  for(int i=0;i<cornersWorld.size();i++) {
    cv::Point3f & q = cornersWorld[i];
    Vector3d p(q.x,q.y,q.z); 
    data.worldPoints.push_back(p);
  }

  // copy cornersCamera onto data.imagePoints
  data.imagePoints.clear();
  for(int i=0;i<cornersCamera.size();i++) {
    cv::Point2f & q = cornersCamera[i];
    Vector2d p(q.x,q.y); 
    data.imagePoints.push_back(p);
  }

  // }

  // update image
  auto mainWin = getApp()->getMainWindow();
  auto glWidget = mainWin->getImgGLWidget();
  glWidget->setQImage(_currentImage);
  QTimer::singleShot(10, glWidget, SLOT(update()));

  if(interactive)
    std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::on_chessboardDetectAllButton_clicked(bool checked) {
  std::cerr << "TurntableScanningPanel::on_chessboardDetectAllButton_clicked() {\n";

  static bool stop = false;
  if(!checked) {
    stop = true;
  } else {
    const bool interactive = false;
    QAbstractItemModel *model = imageList->model();
    int nRows = model->rowCount();
    calibrationProgress->setMaximum(nRows);
    calibrationProgress->setValue(0);


    CalibrationMode calMode = getCalibrationMode();
  
    switch(calMode) {
    case CalibrateCamera:
      std::cerr << "  detecting all camera chessboards\n";
      break;
    case CalibrateTurntable:
      std::cerr << "  detecting all turntable chessboards\n";
      break;
    case CalibrateLaser1:
      std::cerr << "  detecting all laser1 chessboards\n";
      break;
    case CalibrateLaser2:
      std::cerr << "  detecting all laser2 chessboards\n";
      break;
    }


    for(int row = 0;stop==false && row<nRows;row++) {
      QModelIndex indx = model->index(row,0);
      imageList->setCurrentIndex(indx);
      chessboardDetectCurrent(interactive,true);
      calibrationProgress->setValue(row+1);
      QApplication::processEvents();
    }
    stop = false;
  }
  chessboardDetectAllButton->setChecked(false);
  QApplication::processEvents();
  calibrationProgress->setValue(0);
    
  std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::on_calibrateButton_clicked() {
  std::cerr << "TurntableScanningPanel::on_calibrateButton_clicked() {\n";

  CalibrationMode calMode = getCalibrationMode();
  
  switch(calMode) {
  case CalibrateCamera:
    std::cerr << "  calibrating camera\n";
    break;
  case CalibrateTurntable:
    std::cerr << "  calibrating turntable\n";
    break;
  case CalibrateLaser1:
    std::cerr << "  calibrating laser1\n";
    break;
  case CalibrateLaser2:
    std::cerr << "  calibrating laser2\n";
    break;
  }

  CalibrationData & calibData = calibrationData();

  int nImages = calibData.imageCount;
  if(nImages<3) {
    QMessageBox::critical(this, "Error", "Minimum Number of Calibration Images = 3");
    std::cerr << "  nImages = "<<nImages<<" < 3\n";
    std::cerr << "}\n";
    calibrateButton->setChecked(false);
    return;
  }

  int width  = calibData.imageSize.width();
  int height = calibData.imageSize.height();
  if(width<=0 || height<=0) {
    QMessageBox::critical(this, "Error", "imageWidth & imageHeight should be > 0");
    std::cerr << "  imageSize = ("<<width<<","<<height<<")\n";
    std::cerr << "}\n";
    calibrateButton->setChecked(false);
    return;
  }
  cv::Size2i imageSize(width,height);

  int chessboardRows = chessboardRowsSpin->value();
  int chessboardCols = chessboardColsSpin->value();
  int nCorners       = chessboardRows*chessboardCols;
  
  std::cerr << "  nCorners = " << nCorners << "\n";

  std::vector<int> iImageUsed;
  std::vector<std::vector<cv::Point3f> > worldPoints;
  std::vector<std::vector<cv::Point2f> > imagePoints;
  QVector<CalibrationDataCamera::ImageData> & dataVec = calibData.imageData;
  QVector<QString> imageFileName;
  for(int iImage=0; iImage<nImages;iImage++) {
    CalibrationDataCamera::ImageData & data = dataVec[iImage];
    imageFileName.push_back(data.imageName);
    if(data.imagePoints.size()!=nCorners || data.worldPoints.size()!=nCorners) {
      std::cerr << "  skipping image " << iImage << "\n";
      std::cerr << "    nImageCorners = " << data.imagePoints.size() << "\n";
      std::cerr << "    nWorldCorners = " << data.worldPoints.size() << "\n";
    } else {
      iImageUsed.push_back(iImage);
      QVector<Vector3d> & qWorldPoints = data.worldPoints;
      QVector<Vector2d> & qImagePoints = data.imagePoints;

      std::vector<cv::Point3f> worldCorners;
      std::vector<cv::Point2f> imageCorners;
      for(int k=0;k<nCorners;k++) {
        Vector3d & wp = qWorldPoints[k];
        Vector2d & ip = qImagePoints[k];
        cv::Point3f wc(wp(0),wp(1),wp(2));
        cv::Point2f ic(ip(0),ip(1));
        worldCorners.push_back(wc);
        imageCorners.push_back(ic);
      }

      worldPoints.push_back(worldCorners);
      imagePoints.push_back(imageCorners);
    }
  }

  int nImagesUsed = iImageUsed.size();
  std::cerr << "  nImagesUsed = " << nImagesUsed << "\n";

  if(calMode==CalibrateLaser2) {
	  
	  std::cout << "Only Use Plane 1!!!!!!!!!!!!!!!!" << std::endl;

 //   if(_cameraCalib.isCalibrated == false) {
 //     QMessageBox::critical(this, "Error", "Camera is not calibrated");
 //     std::cerr << "  camera is not calibrated\n";
 //     std::cerr << "}\n";
 //     return;
 //   }

 //   // Matrix3d _cameraCalib.K;
 //   // Vector5d _cameraCalib.kc;

 //   cv::Mat K(3,3,CV_64F);
 //   K.at<double>(0,0) = _cameraCalib.K(0,0);
 //   K.at<double>(0,1) = _cameraCalib.K(0,1);
 //   K.at<double>(0,2) = _cameraCalib.K(0,2);
 //   K.at<double>(1,0) = _cameraCalib.K(1,0);
 //   K.at<double>(1,1) = _cameraCalib.K(1,1);
 //   K.at<double>(1,2) = _cameraCalib.K(1,2);
 //   K.at<double>(2,0) = _cameraCalib.K(2,0);
 //   K.at<double>(2,1) = _cameraCalib.K(2,1);
 //   K.at<double>(2,2) = _cameraCalib.K(2,2);

 //   cv::Mat kc(1,5,CV_64F);
 //   kc.at<double>(0,0) = _cameraCalib.kc(0);
 //   kc.at<double>(0,1) = _cameraCalib.kc(1);
 //   kc.at<double>(0,2) = _cameraCalib.kc(2);
 //   //kc.at<double>(0,4) = _cameraCalib.kc(3);
 //   //kc.at<double>(0,5) = _cameraCalib.kc(4);
	//kc.at<double>(0,3) = _cameraCalib.kc(3);
	//kc.at<double>(0,4) = _cameraCalib.kc(4);

 //   int chessboardRows = chessboardRowsSpin->value();
 //   int chessboardCols = chessboardColsSpin->value();
 //   cv::Size cornerCount(chessboardCols,chessboardRows);

 //   double cellWidth  = chessboardCellWidthValue->text().toDouble();
 //   double cellHeight = chessboardCellHeightValue->text().toDouble();
 //   cv::Size2f cornerSize(cellWidth,cellHeight);

 //   Vector4d& laserPlane = _laser2Calib.laserPlane;

 //   // void hw4::calibrateLaserPlane
 //   //   (// inputs
 //   //    cv::Mat & K,
 //   //    cv::Mat & kc,
 //   //    QVector<QString> const& laserImageFileName,
 //   //    cv::Size2i cornerCount,
 //   //    cv::Size2f cornerSize,
 //   //    // outputs
 //   //    Vector4d& laserPlane);
	//QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
	//QString workPath = workDir.absolutePath();
	//workPath += "/";
	//for (int v = 0; v < imageFileName.size(); v++)
	//{
	//	imageFileName[v] = workPath + imageFileName[v];
	//	//std::cout << imageFileName[v].toUtf8().constData() << std::endl;
	//}
 //   hw4::calibrateLaserPlane(K,kc,imageFileName,cornerCount,cornerSize,laserPlane);

  } else if(calMode==CalibrateLaser1) {

    if(_cameraCalib.isCalibrated == false) {
      QMessageBox::critical(this, "Error", "Camera is not calibrated");
      std::cerr << "  camera is not calibrated\n";
      std::cerr << "}\n";
      return;
    }

    // Matrix3d _cameraCalib.K;
    // Vector5d _cameraCalib.kc;

    cv::Mat K(3,3,CV_64F);
    K.at<double>(0,0) = _cameraCalib.K(0,0);
    K.at<double>(0,1) = _cameraCalib.K(0,1);
    K.at<double>(0,2) = _cameraCalib.K(0,2);
    K.at<double>(1,0) = _cameraCalib.K(1,0);
    K.at<double>(1,1) = _cameraCalib.K(1,1);
    K.at<double>(1,2) = _cameraCalib.K(1,2);
    K.at<double>(2,0) = _cameraCalib.K(2,0);
    K.at<double>(2,1) = _cameraCalib.K(2,1);
    K.at<double>(2,2) = _cameraCalib.K(2,2);

    cv::Mat kc(1,5,CV_64F);
    kc.at<double>(0,0) = _cameraCalib.kc(0);
    kc.at<double>(0,1) = _cameraCalib.kc(1);
    kc.at<double>(0,2) = _cameraCalib.kc(2);
    //kc.at<double>(0,4) = _cameraCalib.kc(3);
    //kc.at<double>(0,5) = _cameraCalib.kc(4);
	kc.at<double>(0, 3) = _cameraCalib.kc(3);
	kc.at<double>(0, 4) = _cameraCalib.kc(4);

    int chessboardRows = chessboardRowsSpin->value();
    int chessboardCols = chessboardColsSpin->value();
    cv::Size cornerCount(chessboardCols,chessboardRows);

    double cellWidth  = chessboardCellWidthValue->text().toDouble();
    double cellHeight = chessboardCellHeightValue->text().toDouble();
    cv::Size2f cornerSize(cellWidth,cellHeight);

    Vector4d& laserPlane = _laser1Calib.laserPlane;
	Vector4d& laserPlane2 = _laser2Calib.laserPlane;

    // void hw4::calibrateLaserPlane
    //   (// inputs
    //    cv::Mat & K,
    //    cv::Mat & kc,
    //    QVector<QString> const& laserImageFileName,
    //    cv::Size2i cornerCount,
    //    cv::Size2f cornerSize,
    //    // outputs
    //    Vector4d& laserPlane);
	QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
	QString workPath = workDir.absolutePath();
	//workPath += "/";
	//for (int v = 0; v < imageFileName.size(); v++)
	//{
	//	imageFileName[v] = workPath + imageFileName[v];
	//	std::cout << imageFileName[v].toUtf8().constData() << std::endl;
	//}
	workPath += "/Actual";
	QDir actual_Dir(workPath);
	QFileInfoList actual_Img_list = actual_Dir.entryInfoList();
	QString Img_add = actual_Dir.absolutePath() + "/";
	QVector<QString> actual_imageFileName;
	for (int i = 0; i < actual_Img_list.size(); ++i) 
	{
		QFileInfo fileInfo = actual_Img_list.at(i);
		actual_imageFileName.push_back(Img_add + fileInfo.fileName());
	}

    hw4::calibrateLaserPlane(K,kc, actual_imageFileName,cornerCount,cornerSize,laserPlane, laserPlane2);


  } else if(calMode==CalibrateTurntable) {

    if(_cameraCalib.isCalibrated == false) {
      QMessageBox::critical(this, "Error", "Camera is not calibrated");
      std::cerr << "  camera is not calibrated\n";
      std::cerr << "}\n";
      return;
    }

    // Matrix3d _cameraCalib.K;
    // Vector5d _cameraCalib.kc;

    cv::Mat K(3,3,CV_64F);
	K.at<double>(0,0) = _cameraCalib.K(0,0);
    K.at<double>(0,1) = _cameraCalib.K(0,1);
    K.at<double>(0,2) = _cameraCalib.K(0,2);
    K.at<double>(1,0) = _cameraCalib.K(1,0);
    K.at<double>(1,1) = _cameraCalib.K(1,1);
    K.at<double>(1,2) = _cameraCalib.K(1,2);
    K.at<double>(2,0) = _cameraCalib.K(2,0);
    K.at<double>(2,1) = _cameraCalib.K(2,1);
    K.at<double>(2,2) = _cameraCalib.K(2,2);

    cv::Mat kc(1,5,CV_64F);
    kc.at<double>(0,0) = _cameraCalib.kc(0);
    kc.at<double>(0,1) = _cameraCalib.kc(1);
    kc.at<double>(0,2) = _cameraCalib.kc(2);
    //kc.at<double>(0,4) = _cameraCalib.kc(3);
    //kc.at<double>(0,5) = _cameraCalib.kc(4);
	kc.at<double>(0, 3) = _cameraCalib.kc(3);
	kc.at<double>(0, 4) = _cameraCalib.kc(4);

    // center of rotation in chessboard coordinates
    Vector2d & C       = _turntableCalib.C;
    // turntable coordinate system rotation
    Matrix3d & R_world = _turntableCalib.R;
    // turntable coordinate system rotation
    Vector3d & T_world = _turntableCalib.T;
    
    hw4::calibrateTurntable(K,kc,worldPoints,imagePoints,C,R_world,T_world);

	if (true) {
		std::cerr << "  C    = " << C[0] << endl;
		std::cerr << "  C    = " << C[1] << endl;
		std::cerr << "  R_world00   = " << _turntableCalib.R(0,0) << endl;
		std::cerr << "  R_world01   = " << _turntableCalib.R(0, 1) << endl;
		std::cerr << "  R_world02   = " << _turntableCalib.R(0, 2) << endl;
		std::cerr << "  R_world10   = " << _turntableCalib.R(1, 0) << endl;
		std::cerr << "  R_world11   = " << _turntableCalib.R(1, 1) << endl;
		std::cerr << "  R_world12   = " << _turntableCalib.R(1, 2) << endl;
		std::cerr << "  R_world20   = " << _turntableCalib.R(2, 0) << endl;
		std::cerr << "  R_world21   = " << _turntableCalib.R(2, 1) << endl;
		std::cerr << "  R_world22   = " << _turntableCalib.R(2, 2) << endl;
		std::cerr << "  T_world0   = " << _turntableCalib.T[0] << endl;
		std::cerr << "  T_world1   = " << _turntableCalib.T[1] << endl;
		std::cerr << "  T_world2   = " << _turntableCalib.T[2] << endl;
	}
	QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
	QString workPath = workDir.absolutePath();
	workPath += "/";
	cv::Mat turntable_coordinate;
	turntable_coordinate = cv::imread((workPath + imageFileName[0]).toStdString());
	Vector3d c = _cameraCalib.K*T_world / (_cameraCalib.K*T_world)[2];
	Vector3d x = _cameraCalib.K*(100 * R_world.col(0) + T_world) / (_cameraCalib.K*T_world)[2];
	Vector3d y = _cameraCalib.K*(100 * R_world.col(1) + T_world) / (_cameraCalib.K*T_world)[2];
	Vector3d z = _cameraCalib.K*(-100 * R_world.col(2) + T_world) / (_cameraCalib.K*T_world)[2];
	cv::arrowedLine(turntable_coordinate, cv::Point(c[0], c[1]), cv::Point(x[0], x[1]), cv::Scalar(0, 255, 0), 4);
	cv::arrowedLine(turntable_coordinate, cv::Point(c[0], c[1]), cv::Point(y[0], y[1]), cv::Scalar(0, 0, 255), 4);
	cv::arrowedLine(turntable_coordinate, cv::Point(c[0], c[1]), cv::Point(z[0], z[1]), cv::Scalar(255, 0, 0), 4);
	cv::imwrite("turntable_coordinate.png", turntable_coordinate);
    _turntableCalib.isCalibrated = true;

    // TODO Wed Apr  4 21:50:34 2018
    // draw turntable coordinate system in the 3D view

  } else /* if(calMode==CalibrateCamera) */ {

    std::vector<cv::Mat> calRvecs;
    std::vector<cv::Mat> calTvecs;
    cv::Mat K;
	cv::Mat kc;

    double camError =
      hw4::calibrateCamera
      (worldPoints,imagePoints,imageSize,K,kc,calRvecs,calTvecs);

    std::cerr << "  camError = " << camError << "\n";

    // save the intrinsic parameters
    _cameraCalib.K(0,0) = K.at<double>(0,0);
    _cameraCalib.K(0,1) = K.at<double>(0,1);
    _cameraCalib.K(0,2) = K.at<double>(0,2);
    _cameraCalib.K(1,0) = K.at<double>(1,0);
    _cameraCalib.K(1,1) = K.at<double>(1,1);
    _cameraCalib.K(1,2) = K.at<double>(1,2);
    _cameraCalib.K(2,0) = K.at<double>(2,0);
    _cameraCalib.K(2,1) = K.at<double>(2,1);
    _cameraCalib.K(2,2) = K.at<double>(2,2);
    // save the lens distortion parameters
    _cameraCalib.kc(0) = kc.at<double>(0,0);
    _cameraCalib.kc(1) = kc.at<double>(0,1);
    _cameraCalib.kc(2) = kc.at<double>(0,2);
    _cameraCalib.kc(3) = kc.at<double>(0,3);
    _cameraCalib.kc(4) = kc.at<double>(0,4);
    // save the camera poses
    for(int j=0;j<nImagesUsed;j++) {
      int iImage = iImageUsed[j];
      CalibrationDataCamera::ImageData & data = dataVec[iImage];
      //
      cv::Mat  & Rvec  = calRvecs[j];
      Vector3d & cRvec = data.rotVec;
      cRvec(0) = Rvec.at<double>(0,0);
      cRvec(1) = Rvec.at<double>(1,0);
      cRvec(2) = Rvec.at<double>(2,0);
      //
      cv::Mat  & Tvec  = calTvecs[j];
      Vector3d & cTvec = data.T;
      cTvec(0) = Tvec.at<double>(0,0);
      cTvec(1) = Tvec.at<double>(1,0);
      cTvec(2) = Tvec.at<double>(2,0);
    }

    _cameraCalib.calError     = camError;
    _cameraCalib.isCalibrated = true;

    if(true) {
      std::cerr << "  K00    = " << _cameraCalib.K(0, 0) << endl;
      std::cerr << "  K01    = " << _cameraCalib.K(0, 1) << endl;
      std::cerr << "  K02    = " << _cameraCalib.K(0, 2) << endl;
      std::cerr << "  K10    = " << _cameraCalib.K(1, 0) << endl;
      std::cerr << "  K11    = " << _cameraCalib.K(1, 1) << endl;
      std::cerr << "  K12    = " << _cameraCalib.K(1, 2) << endl;
      std::cerr << "  K20    = " << _cameraCalib.K(2, 0) << endl;
      std::cerr << "  K21    = " << _cameraCalib.K(2, 1) << endl;
      std::cerr << "  K22    = " << _cameraCalib.K(2, 2) << endl;
      std::cerr << "  kc1    = " << _cameraCalib.kc[0] << endl;
      std::cerr << "  kc2    = " << _cameraCalib.kc[1] << endl;
      std::cerr << "  kc3    = " << _cameraCalib.kc[2] << endl;
      std::cerr << "  kc4    = " << _cameraCalib.kc[3] << endl;
      std::cerr << "  kc5    = " << _cameraCalib.kc[4] << endl;
      std::cerr << "  calErr = " << _cameraCalib.calError << endl;
    }
  }


  // TODO Wed Apr  4 21:50:34 2018
  // draw turntable coordinate system in the 3D view


  std::cerr << "}\n";
  calibrateButton->setChecked(false);
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::on_loadScannerCalibrationButton_clicked() {

  // TODO Tue Apr 10 2018

  // if SCANNER_CALIBRATION_FILENAME does not exist
  //   - pop ERROR dialog and quit

  // open SCANNER_CALIBRATION_FILENAME for reading

  // read camera calibration parameters and set _cameraCalib
  // read turntable calibration parameters and set _turntableCalib
  // read laser1 calibration parameters and set _laser1Calib
  // read laser2 calibration parameters and set _laser2Calib

  // close SCANNER_CALIBRATION_FILENAME
}

//////////////////////////////////////////////////////////////////////
void TurntableScanningPanel::on_saveScannerCalibrationButton_clicked() {

  // TODO Tue Apr 10 2018

  // if camera is not calibrated
  //   - pop ERROR dialog and quit
  // if turntable is not calibrated
  //   - pop ERROR dialog and quit
  // if laser1 is not calibrated
  //   - pop ERROR dialog and quit
  // if laser2 is not calibrated
  //   - pop ERROR dialog and quit

  // open SCANNER_CALIBRATION_FILENAME for writing

  // save camera calibration parameters
  // save turntable calibration parameters
  // save laser1 calibration parameters
  // save laser2 calibration parameters

  // close SCANNER_CALIBRATION_FILENAME
}

//////////////////////////////////////////////////////////////////////
// Scanning Group {

void TurntableScanningPanel::_on_laserTriangulatorStarted() {

  LaserTriangulator* laserTriangulator =
    dynamic_cast<LaserTriangulator*>(sender());
  if (!laserTriangulator) { return; }

  auto mainWin = getApp()->getMainWindow();
  mainWin->setChangesEnabled(false);

  scanningScanButton->setText("STOP");

  int count = laserTriangulator->count();
  scanningProgress->setMaximum(count);
  scanningProgress->setValue(0);

  SceneGraph* pWrl = mainWin->getSceneGraph();
  //Shape* shape = dynamic_cast<Shape*>(pWrl->find("POINTS"));
  Shape* shape = dynamic_cast<Shape*>(pWrl->find("QtLogo"));
  IndexedFaceSet* ifs = dynamic_cast<IndexedFaceSet*>(shape->getGeometry());
  std::vector<float>& coord = ifs->getCoord();
  coord.clear();

  // TODO Tue May  3 15:45:41 2016
  // clear scene graph point cloud 

  // calibrateTurntableButton->setEnabled(false);
}

void TurntableScanningPanel::_on_laserTriangulatorProgress() {

  LaserTriangulator* laserTriangulator =
    dynamic_cast<LaserTriangulator*>(sender());
  if (!laserTriangulator) { return; }

  int index = laserTriangulator->index();
  scanningProgress->setValue(index);

  QVector<Vector3d>& worldCoord = laserTriangulator->worldCoord(); 
  int nPoints = worldCoord.size();
  if(nPoints>0) {
    auto mainWin = getApp()->getMainWindow();
    SceneGraph* pWrl = mainWin->getSceneGraph();
    //Shape* shape = dynamic_cast<Shape*>(pWrl->find("POINTS"));
	Shape* shape = dynamic_cast<Shape*>(pWrl->find("QtLogo"));
    IndexedFaceSet* ifs = dynamic_cast<IndexedFaceSet*>(shape->getGeometry());
    if(ifs!=(IndexedFaceSet*)0) {
      // remove all the properties for now
      ifs->getColor().clear();
      ifs->getNormal().clear();
      ifs->getColorIndex().clear();
      ifs->getNormalIndex().clear();
      ifs->getCoordIndex().clear();
      
      std::vector<float>& coord  = ifs->getCoord();
	  //coord.clear();
      for(int i=0;i<nPoints;i++) {
        coord.push_back(worldCoord[i](0));
        coord.push_back(worldCoord[i](1));
        coord.push_back(worldCoord[i](2));
      }
    }
    mainWin->refresh();
  }
}

void TurntableScanningPanel::_on_laserTriangulatorFinished() {

  LaserTriangulator* laserTriangulator =
    dynamic_cast<LaserTriangulator*>(sender());
  if (!laserTriangulator) { return; }

  scanningScanButton->setText("All");

  auto mainWin = getApp()->getMainWindow();
  mainWin->setChangesEnabled(true);

  scanningProgress->setValue(0);
  scanningScanButton->setChecked(false);
}

void TurntableScanningPanel::on_scanningScanButton_clicked(bool checked) {

  // TODO Tue Apr 10 2018

  // if camera is not calibrated
  //   - pop ERROR dialog and quit
  // if turntable is not calibrated
  //   - pop ERROR dialog and quit
  // if laser1 is not calibrated
  //   - pop ERROR dialog and quit
  // if laser2 is not calibrated
  //   - pop ERROR dialog and quit

  // if number of images <= 2
  //   - pop ERROR dialog and quit

  auto mainWin = getApp()->getMainWindow();
  
  if(checked) {
  
    _laserTriangulator = new LaserTriangulator(this);
  
    _laserTriangulator->setWorkDirectory(mainWin->getWorkDirectory());

    Matrix3d& K  = _cameraCalib.K;
    Vector5d& kc = _cameraCalib.kc;

    _laserTriangulator->setIntrinsicCameraParameters(K,kc);
  
    // center of rotation in chessboard coordinate system
    Vector2d & C = _turntableCalib.C;
    // turntable coordinate system : rotation matrix
    Matrix3d & R = _turntableCalib.R;
    // turntable coordinate system : translateion vector
    Vector3d & T = _turntableCalib.T;

    _laserTriangulator->setWorldRotation(R);
    _laserTriangulator->setWorldTranslation(T);


    Vector4d & laserPlane1 = _laser1Calib.laserPlane;
    Vector4d & laserPlane2 = _laser2Calib.laserPlane;

    _laserTriangulator->setLaserPlane1(laserPlane1);
    _laserTriangulator->setLaserPlane2(laserPlane2);

    // TODO Tue Apr 10 2018
    //float degreesPerFrame = turntableDegreesPerFrameValue->text().toFloat();

    float degreesPerFrame = 5.0f; 
    _laserTriangulator->setDegreesPerFrame(degreesPerFrame);
     
    connect(_laserTriangulator, &LaserTriangulator::started,
            this, &TurntableScanningPanel::_on_laserTriangulatorStarted);
    connect(_laserTriangulator, &LaserTriangulator::progress,
            this, &TurntableScanningPanel::_on_laserTriangulatorProgress);
    connect(_laserTriangulator, &LaserTriangulator::finished,
            this, &TurntableScanningPanel::_on_laserTriangulatorFinished);
  
    _laserTriangulator->start();
  
  } else /* if(!checked) */ {
    if(_laserTriangulator!=(LaserTriangulator*)0) {
      _laserTriangulator->stop();
      _laserTriangulator = (LaserTriangulator*)0;
    }
  }

}

// } Scanning Group
//////////////////////////////////////////////////////////////////////
