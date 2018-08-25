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

#include "CameraPanel.hpp"

#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QTimer>
#include <QDir>

#include "Application.hpp"
#include "MainWindow.hpp"
#include "CameraRecorder.hpp"

#include <cam/cam.hpp>
#include <cam/CameraInterface.hpp>

#include <img/util.hpp>
#include <img/yuv.hpp>

#include "gui_util.hpp"

#define DISABLE_SMALL_PREVIEW

bool CameraPanel::_registered = CameraPanel::registerPanel();
bool CameraPanel::registerPanel() {
   qDebug("CameraPanel: register");
   return MainWindow::registerPanel
     ("Camera Capture",
      [](QWidget *parent) -> QWidget*{ return new CameraPanel(parent); });
   return true;
}

CameraPanel::CameraPanel(QWidget * parent, Qt::WindowFlags f) :
  QWidget(parent, f),
  _camera(),
  _imageCount(0),
  _fpsTimer(),
  _model(),
  _captureOneImage(0) {
  setupUi(this);

#ifdef DISABLE_SMALL_PREVIEW
  camWidget->setVisible(false);
#endif //DISABLE_SMALL_PREVIEW

  QSettings config;

  //fill camera combo
  gui_util::fillCombo(cameraCombo, cam::getCameraList(),
                      config.value("CameraPanel/camera").toString());

  //fill rotation combo
  QStringList rotationList; rotationList << "0" << "90" << "180" << "270";
  gui_util::fillCombo(imageRotCombo, rotationList,
                      config.value("CameraPanel/rotation").toString());
  camWidget->setRotation(imageRotCombo->currentText().toFloat());

  //signal to update camera info label from threads
  connect(this, &CameraPanel::setInfoLabel, imageInfoLabel, &QLabel::setText);

  //capture group
  countSpin->setRange(1, 1000);
  intervalSpin->setRange(0, 100000);
  intervalSpin->setSingleStep(100);
  captureProgress->setVisible(false);

  //load other settings
  loadSettings();

  //list view model
  imageList->setModel(&_model);
  auto selectionModel = imageList->selectionModel();
  connect(selectionModel, &QItemSelectionModel::currentChanged,
          this, &CameraPanel::_on_currentImageChanged);

  qDebug(" -- CameraPanel created -- ");
}

CameraPanel::~CameraPanel() {
  if (!_camera.isNull()) { //stop
#ifdef DISABLE_SMALL_PREVIEW
    auto camWidget =  getApp()->getMainWindow()->getImgGLWidget();
#endif //DISABLE_SMALL_PREVIEW
#ifdef HAVE_IMG
    disconnect(_camera.data(), &CameraInterface::newImage,
               camWidget, &ImgGLWidget::setImage);
    disconnect(_camera.data(), &CameraInterface::newImage,
               this, &CameraPanel::updateInfo);
#endif //HAVE_IMG
    gui_util::wait(100); //milliseconds
    _camera.clear();
  }

  saveSettings();
  qDebug(" -- CameraPanel destroyed -- ");
}

void CameraPanel::loadSettings(void) {
  QSettings config;

  //current camera
  if (cameraCombo->currentText() == config.value("CameraPanel/camera").toString()) {
    cameraFormatLabel->setText(config.value("CameraPanel/format").toString());
  }

  //capture group
  countSpin->setValue(config.value("CameraPanel/count", 1).toUInt());
  intervalSpin->setValue(config.value("CameraPanel/interval", 500).toUInt());
}

void CameraPanel::saveSettings(void) {
  QSettings config;

  //current camera
  config.setValue("CameraPanel/camera", cameraCombo->currentText());
  config.setValue("CameraPanel/format", cameraFormatLabel->text());
  config.setValue("CameraPanel/rotation", imageRotCombo->currentText());

  //capture group
  config.setValue("CameraPanel/count", countSpin->value());
  config.setValue("CameraPanel/interval", intervalSpin->value()); 
}

void CameraPanel::on_previewButton_clicked(bool checked) {
  auto mainWin = getApp()->getMainWindow();

  //update the GUI
  cameraCombo->setEnabled(!checked);
  countSpin->setEnabled(!checked);
  intervalSpin->setEnabled(!checked);
  captureButton->setText((checked?"Capture one":"Capture"));
  mainWin->setChangesEnabled(!checked);


#ifdef DISABLE_SMALL_PREVIEW
  auto camWidget = mainWin->getImgGLWidget();
#endif //DISABLE_SMALL_PREVIEW

  if (checked) { //start
    getAndSetupCamera();
    bool error = _camera.isNull();
    if (!error) {
#ifdef HAVE_IMG
      connect(_camera.data(), &CameraInterface::newImage,
              this, &CameraPanel::updateInfo, Qt::DirectConnection);
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
               this, &CameraPanel::updateInfo);
#endif //HAVE_IMG
    gui_util::wait(100); //milliseconds
    _camera.clear();
  }
}

void CameraPanel::getAndSetupCamera(void) {
  //get camera
  _camera = cam::getCamera(cameraCombo->currentText());

  //setup camera
  if (!_camera.isNull()) {
    //set camera format
    QString format = cameraFormatLabel->text().trimmed();
    if (!format.isEmpty()) {
      _camera->setCameraProperty(CAM_PROP_FORMAT, format);
    }

    //add formats to context menu
    updateCameraResolutions();
  }
}

void CameraPanel::updateCameraResolutions(void) {
  //clean previous actions
  QList<QAction *> actions = cameraFormatLabel->actions();
  foreach(auto action, actions) {
    cameraFormatLabel->removeAction(action);
    delete action;
  }

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
    {
      QString fmt = sp.at(0)+":"+sp.at(1);
      if (!used[fmt]) {
        used[fmt] = true;
        QAction * action = new QAction(fmt, this);
        connect(action, &QAction::triggered,
                this, &CameraPanel::changeCameraResolution);
        cameraFormatLabel->addAction(action);
      }
    }
  }

  cameraFormatLabel->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void CameraPanel::changeCameraResolution() {
  QAction * action = qobject_cast<QAction *>(sender());
  if (!action) { return; }
  cameraFormatLabel->setText(action->text());
}

#ifdef HAVE_IMG
void CameraPanel::updateInfo(ImageBuffer const& image) {
  QString infoText = "No image.";
  if (image.data) {
    double fps =
      1000.0*static_cast<double>(_imageCount)/
             static_cast<double>(_fpsTimer.elapsed());
    infoText =
      QString("Count: %1 FPS: %2 %3:%4x%5").arg(++_imageCount).arg(fps)
      .arg(image.formatName(image.format)).arg(image.cols).arg(image.rows);
    if (_fpsTimer.elapsed()>30000) {
      _imageCount = 0;
      _fpsTimer.start();
    }

    if (_captureOneImage.testAndSetRelaxed(1,0)) { //save image
      QString defaultImageName = "image_000.png";
      QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
      QString fileName =
        QFileDialog::getSaveFileName
        (this, "Save Image", workDir.absoluteFilePath(defaultImageName),
         "Images (*.png *.jpg)");
      if (!fileName.isEmpty()) {
        int rotation = imageRotCombo->currentText().toInt();
        ImageBuffer flippedImage;
        if (image.format==ImageBuffer::NV12) {
          ImageBuffer rgbImage;
          if (img::rgbFromNV12(image, rgbImage)) {
            img::flipVertical(rgbImage, flippedImage);
          }
        } else { //I hope is RGB ...
          img::flipVertical(image, flippedImage);
        }
        ImageBuffer rotatedImage;
        if (rotation!=0) {
          img::rotateImage(flippedImage, rotation, rotatedImage);
        }
        img::save(fileName, (rotatedImage.data?rotatedImage:flippedImage));
        qDebug("Saved '%s'", qPrintable(fileName));
      }
    }
  }

  //use a signal so that it gets updated in the main thread
  emit setInfoLabel(infoText);

  if (cameraFormatLabel->text().isEmpty() && !_camera.isNull())  {
    cameraFormatLabel->setText
      (_camera->getCameraProperty(CAM_PROP_FORMAT).toString());
  }

  //update image rotation
  camWidget->setRotation(imageRotCombo->currentText().toFloat());
}
#endif //HAVE_IMG

void CameraPanel::on_cameraCombo_currentIndexChanged(int index) {
  cameraFormatLabel->clear();
  updateCameraResolutions();
}

void CameraPanel::on_imageRotCombo_currentIndexChanged(int index) {
#ifdef DISABLE_SMALL_PREVIEW
  auto mainWin = getApp()->getMainWindow();
  auto camWidget = this->camWidget;
  if (mainWin) { camWidget = mainWin->getImgGLWidget(); }
#endif //DISABLE_SMALL_PREVIEW
  camWidget->setRotation(imageRotCombo->currentText().toFloat());
}

void CameraPanel::on_captureButton_clicked() {
  //preview is running, just capture one image
  if (previewButton->isChecked()) {
    _captureOneImage.store(1);
    return;
  }

  auto mainWin = getApp()->getMainWindow();

  //create worker object
  CameraRecorder * recorder = new CameraRecorder(this);

  recorder->setCameraName(cameraCombo->currentText());
  recorder->setCameraFormat(cameraFormatLabel->text().trimmed());
  recorder->setImageCount(countSpin->value());
  recorder->setCaptureInterval(intervalSpin->value());
  recorder->setImageRotation(imageRotCombo->currentText().toInt());

  recorder->setImagePath(mainWin->getWorkDirectory());

  connect(recorder, &CameraRecorder::started,
          this, &CameraPanel::_on_captureStarted);
  connect(recorder, &CameraRecorder::progress,
          this, &CameraPanel::_on_captureProgress);
  connect(recorder, &CameraRecorder::finished,
          this, &CameraPanel::_on_captureFinished);

  recorder->start();
}

void CameraPanel::_on_captureStarted(void) {
  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (!recorder) { return; }

  auto mainWin = getApp()->getMainWindow();

  //update the GUI
  cameraCombo->setEnabled(false);
  imageRotCombo->setEnabled(false);
  previewButton->setEnabled(false);
  captureButton->setEnabled(false);
  mainWin->setChangesEnabled(false);

  int total = recorder->getImageCount();
  captureProgress->setMaximum(total);
  captureProgress->setVisible(true);
  _model.clear();
}

void CameraPanel::_on_captureProgress(void) {
  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (!recorder) { return; }
  
  auto list = recorder->getImageNameList();
  int curr = list.length();

  captureProgress->setValue(curr);
  
  QDir dir(recorder->getImagePath());
  QStandardItem *item = new QStandardItem(list.last());
  item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemNeverHasChildren);
  item->setData(dir.absoluteFilePath(item->text()));
  _model.invisibleRootItem()->appendRow(item);

  if (item) { //display
    auto imageName = item->text();
    auto imagePath = item->data().toString();

    QDir dir(imagePath);
    QString filename = dir.absoluteFilePath(imageName);
    QImage qImg;
    if (qImg.load(filename))
    {
      updateImage(qImg);
    }
  }
}

void CameraPanel::_on_captureFinished(void) {
  auto mainWin = getApp()->getMainWindow();

  CameraRecorder * recorder = dynamic_cast<CameraRecorder*>(sender());
  if (recorder) {
    auto error = recorder->getError();
    auto message = recorder->getErrorMessage();
    qDebug("[CameraPanel::_on_captureFinished] error %s, message '%s'",
           (error?"TRUE":"FALSE"), qPrintable(message));

    if (error) {
      QMessageBox::critical(this, "Error",
                            "Capture failed with message '" + message + "'");
    }
  }

  //update the GUI
  cameraCombo->setEnabled(true);
  imageRotCombo->setEnabled(true);
  previewButton->setEnabled(true);
  captureButton->setEnabled(true);
  mainWin->setChangesEnabled(true);
  captureProgress->setVisible(false);
}

void CameraPanel::_on_currentImageChanged
(const QModelIndex & current, const QModelIndex & previous) {
  Q_UNUSED(current);
  Q_UNUSED(previous);
  updateImage(gui_util::loadCurrentImage(imageList));
}

void CameraPanel::updateImage(QImage image, int rotation) {
  auto mainWin = getApp()->getMainWindow();
  if (!mainWin) {
    return;
  }
  auto glWidget = mainWin->getImgGLWidget();

  if (image.isNull()) {
    glWidget->clearImage();
    return;
  }

  //update image
  glWidget->setRotation(rotation);
  glWidget->setQImage(image);
  QTimer::singleShot(10, glWidget, SLOT(update()));
}

void CameraPanel::showEvent(QShowEvent * event) {
  std::cerr << "void CameraPanel::showEvent(QShowEvent * event) {\n";
  auto mainWin = getApp()->getMainWindow();

  mainWin->showImgGLWidget(true);
  mainWin->showWrlGLWidget(false);

  std::cerr << "}\n";
}
