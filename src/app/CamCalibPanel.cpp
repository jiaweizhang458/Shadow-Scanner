// Software developed for the Spring 2016 Brown University course
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

#include <iostream>

#include "CamCalibPanel.hpp"

#include <QShowEvent>
#include <QHideEvent>
#include <QMessageBox>
#include <QSettings>
#include <QPainter>
#include <QTimer>
#include <QDir>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "Application.hpp"

#include <img/ImgArgb.hpp>

#include "gui_util.hpp"
// #include "matrix_util.hpp"

#ifdef HAVE_HOMEWORK
# include <homework/homework2.hpp>
# include <homework/homework4.hpp>
#endif //HAVE_HOMEWORK

//////////////////////////////////////////////////////////////////////
bool CamCalibPanel::_registered = CamCalibPanel::registerPanel();
bool CamCalibPanel::registerPanel() {
  qDebug("CamCalibPanel: register");
  return MainWindow::registerPanel
    ("Camera Calibration",
     [](QWidget *parent) -> QWidget*{ return new CamCalibPanel(parent); });
  return true;
}

//////////////////////////////////////////////////////////////////////
CamCalibPanel::CamCalibPanel(QWidget * parent, Qt::WindowFlags f) :
  QWidget(parent, f),
  _calib(),
  _model(),
  _currentImage(),
  _reprojection() {
  setupUi(this);

  QSettings config;

  // load other settings
  loadSettings();

  // list view model
  imageList->setModel(&_model);
  auto selectionModel = imageList->selectionModel();
  connect(selectionModel, &QItemSelectionModel::currentChanged,
          this,           &CamCalibPanel::_on_currentImageChanged);

  // progress bar
  reprojectProgress->setVisible(false);
}

//////////////////////////////////////////////////////////////////////
CamCalibPanel::~CamCalibPanel() {
  saveSettings();
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::loadSettings(void) {
  QSettings config;

  displayCornersCheck->setChecked
    (config.value("CamCalibPanel/displayCorners", false).toBool());
  displayReprojectionCheck->setChecked
    (config.value("CamCalibPanel/displayReprojection", false).toBool());
  chessboardColsSpin->setValue
    (config.value("CamCalibPanel/chessboard/cols",7).toInt());
  chessboardRowsSpin->setValue
    (config.value("CamCalibPanel/chessboard/rows",9).toInt());
  double cellWidth = 
    config.value("CamCalibPanel/chessboard/width",20.0).toDouble();
  double cellHeight = 
    config.value("CamCalibPanel/chessboard/height",20.0).toDouble();
  chessboardCellWidthValue->setText(QString("%1").arg(cellWidth));
  chessboardCellHeightValue->setText(QString("%1").arg(cellHeight));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::saveSettings(void) {
  QSettings config;

  config.setValue("CamCalibPanel/displayCorners",
                  displayCornersCheck->isChecked());
  config.setValue("CamCalibPanel/displayReprojection",
                  displayReprojectionCheck->isChecked());
  config.setValue("CamCalibPanel/chessboard/cols",
                  chessboardColsSpin->value());
  config.setValue("CamCalibPanel/chessboard/rows",
                  chessboardRowsSpin->value());
  config.setValue("CamCalibPanel/chessboard/width",
                  chessboardCellWidthValue->text().toDouble());
  config.setValue("CamCalibPanel/chessboard/height",
                  chessboardCellHeightValue->text().toDouble());
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::showEvent(QShowEvent * e) {
  if (e && !e->spontaneous()) {
    auto mainWin = getApp()->getMainWindow();
    auto glWidget = mainWin->getImgGLWidget();
    glWidget->clearImage();
    glWidget->setRotation(0);
    connect(mainWin, &MainWindow::workDirectoryChanged,
            this, &CamCalibPanel::_on_workDirectoryChanged);

    loadMatlabCalib();

    mainWin->showImgGLWidget(true);
    mainWin->showWrlGLWidget(false);
  }
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::hideEvent(QHideEvent * e) {
  if (e && !e->spontaneous()) {
    auto mainWin = getApp()->getMainWindow();
    disconnect(mainWin, &MainWindow::workDirectoryChanged,
               this,    &CamCalibPanel::_on_workDirectoryChanged);

    if(_calib.isCalibrated) saveMatlabCalib();
  }
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::_on_workDirectoryChanged() {
  loadMatlabCalib();
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::updateImageFiles() {

  // //get image files
  // QStringList fileList;
  // auto const& imageData = _calib.imageData;
  // foreach(auto & data, imageData) {
  //   fileList.append(data.imageName);
  // }

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

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::_on_currentImageChanged
(const QModelIndex & current, const QModelIndex & previous) {
  Q_UNUSED(current);
  Q_UNUSED(previous);
  QImage qImg = gui_util::loadCurrentImage(imageList);
  updateImage(qImg);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_imagesUpdateButton_clicked() {
  std::cerr << "CamCalibPanel::on_imagesUpdateButton_clicked() {\n";
  updateImageFiles();
  std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::deleteAllImageFiles() {

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

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_imagesDeleteAllButton_clicked() {
  std::cerr << "CamCalibPanel::on_imagesDeleteAllButton_clicked() {\n";
  deleteAllImageFiles();
  std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_imagesNextButton_clicked() {
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

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_imagesPreviousButton_clicked() {
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

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::updateImage(QImage image) {
  auto mainWin = getApp()->getMainWindow();
  if (!mainWin) {
    return;
  }
  auto glWidget = mainWin->getImgGLWidget();

  if (image.isNull()) {
    glWidget->clearImage();
    return;
  }

  _currentImage = image;

  QPainter painter(&image);
  int index = image.text("index").toInt();
  QPen pen;
  pen.setWidth(8);
  int radius   = 4;
  int diameter = 2*radius;

  //draw corners
  pen.setColor(Qt::green);
  painter.setPen(pen);
  if (displayCornersCheck->isChecked() && index<_calib.imageData.size()) {
    auto const& imagePoints = _calib.imageData[index].imagePoints;
    foreach(auto const& p, imagePoints) {
      painter.drawEllipse(p.x()-radius, p.y()-radius, diameter, diameter);
      painter.drawPoint(p.x(), p.y());
    }
  }

  //draw reprojected points
  pen.setColor(Qt::blue);
  painter.setPen(pen);
  if (displayReprojectionCheck->isChecked() && index<_reprojection.size()) {
    auto const& imagePoints = _reprojection[index].imagePoints;
    foreach(auto const& p, imagePoints) {
      painter.drawLine(p.x()-radius, p.y(), p.x()+radius, p.y());
      painter.drawLine(p.x(), p.y()-radius, p.x(), p.y()+radius);
    }
  }

  //update rms error lineEdit
  double rmsError = 0.0;
  if (index<_reprojection.size()) {
    rmsError = _reprojection[index].rmsError;
  }
  rmsErrorLine->setText(QString::number(rmsError));

  //update image
  glWidget->setQImage(image);
  QTimer::singleShot(10, glWidget, SLOT(update()));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::calibGroupUpdate() {
  imageCountLine->setText(QString::number(_calib.imageCount));
  imageWidthLine->setText(QString::number(_calib.imageSize.width()));
  imageHeightLine->setText(QString::number(_calib.imageSize.height()));
  K00_Line->setText(QString::number(_calib.K(0, 0)));
  K01_Line->setText(QString::number(_calib.K(0, 1)));
  K02_Line->setText(QString::number(_calib.K(0, 2)));
  K10_Line->setText(QString::number(_calib.K(1, 0)));
  K11_Line->setText(QString::number(_calib.K(1, 1)));
  K12_Line->setText(QString::number(_calib.K(1, 2)));
  K20_Line->setText(QString::number(_calib.K(2, 0)));
  K21_Line->setText(QString::number(_calib.K(2, 1)));
  K22_Line->setText(QString::number(_calib.K(2, 2)));
  kc1Line->setText(QString::number(_calib.kc[0]));
  kc2Line->setText(QString::number(_calib.kc[1]));
  kc3Line->setText(QString::number(_calib.kc[2]));
  kc4Line->setText(QString::number(_calib.kc[3]));
  kc5Line->setText(QString::number(_calib.kc[4]));
  calErrorLine->setText(QString::number(_calib.calError));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::resetPanel(void) {
  auto mainWin = getApp()->getMainWindow();
  if (mainWin) { mainWin->getImgGLWidget()->clearImage(); }
  _model.clear();
  _calib.clear();
  _reprojection.clear();
  calibGroupUpdate();
}

//////////////////////////////////////////////////////////////////////
bool CamCalibPanel::loadMatlabCalib(void) {
  //reset GUI
  resetPanel();

  //calib file
  auto workDir = getApp()->getMainWindow()->getWorkDirectory();
  QDir dir(workDir);
  auto fileName = dir.absoluteFilePath(MATLAB_CALIB_FILENAME);

  //load
  bool retVal = _calib.loadMatlabCalib(fileName);

  //update GUI
  calibGroupUpdate();  
  updateImageFiles();

  return retVal;
}

//////////////////////////////////////////////////////////////////////
bool CamCalibPanel::saveMatlabCalib(void) {
  auto workDir = getApp()->getMainWindow()->getWorkDirectory();
  QDir dir(workDir);
  auto fileName = dir.absoluteFilePath(MATLAB_CALIB_FILENAME);
  return _calib.saveMatlabCalib(fileName);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::chessboardDetectCurrent(bool interactive) {
  if(interactive)
    std::cerr << "CamCalibPanel::chessboardDetectCurrent() {\n";

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
      // return; ???
      std::cerr << "  detected only "<< nCornersDetected <<" corners of "<< nCornersExpected <<" expected\n";
      std::cerr << "}\n";
    }
    return;
  }
  
  // draw corners
  QPainter painter(&_currentImage);
  QPen pen;
  // pen.setWidth(8);
  // pen.setColor(Qt::red);
  // painter.setPen(pen);
  qreal red,green,blue;
  QColor penColor;
  int radius   = 4;
  int diameter = 2*radius;
  if (displayCornersCheck->isChecked()) {

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

  // update _calib 
  _calib.imageData.resize(static_cast<int>(nImages));
  _calib.imageCount = nImages;
  _calib.imageSize.setWidth(width);
  _calib.imageSize.setHeight(height);

  CalibrationDataCamera::ImageData & data = _calib.imageData[iImage];
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

  // _calib.print();

  // update image
  auto mainWin = getApp()->getMainWindow();
  auto glWidget = mainWin->getImgGLWidget();
  glWidget->setQImage(_currentImage);
  QTimer::singleShot(10, glWidget, SLOT(update()));

  if(interactive)
    std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_chessboardDetectButton_clicked() {
  bool interactive = true;
  chessboardDetectCurrent(interactive);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_chessboardDetectPreviousButton_clicked() {
  on_imagesPreviousButton_clicked();    
  bool interactive = true;
  chessboardDetectCurrent(interactive);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_chessboardDetectNextButton_clicked() {
  on_imagesNextButton_clicked();    
  bool interactive = true;
  chessboardDetectCurrent(interactive);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_chessboardDetectAllButton_clicked(bool checked) {
  static bool stop = false;
  if(!checked) {
    stop = true;
  } else {
    const bool interactive = false;
    QAbstractItemModel *model = imageList->model();
    int nRows = model->rowCount();
    chessboardProgress->setMaximum(nRows);
    chessboardProgress->setValue(0);
    for(int row = 0;stop==false && row<nRows;row++) {
      QModelIndex indx = model->index(row,0);
      imageList->setCurrentIndex(indx);
      chessboardDetectCurrent(interactive);
      chessboardProgress->setValue(row+1);
      QApplication::processEvents();
    }
    stop = false;
  }
  chessboardDetectAllButton->setChecked(false);
  QApplication::processEvents();
  chessboardProgress->setValue(0);
    
  _calib.print();
  calibGroupUpdate();
}

//////////////////////////////////////////////////////////////////////
// void CamCalibPanel::on_chessboardColsSpin_valueChanged(int i) {
// }

//////////////////////////////////////////////////////////////////////
// void CamCalibPanel::on_chessboardRowsSpin_valueChanged(int i) {
// }

//////////////////////////////////////////////////////////////////////
// void CamCalibPanel::on_chessboardCellWidthValue_returnPressed() {
// }

//////////////////////////////////////////////////////////////////////
// void CamCalibPanel::on_chessboardCellHeightValue_returnPressed() {
// }

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_displayReprojectionCheck_stateChanged() {
  updateImage(gui_util::loadCurrentImage(imageList, false));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_displayCornersCheck_stateChanged() {
  updateImage(gui_util::loadCurrentImage(imageList, false));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_reprojectCurrentButton_clicked() {
  //get selected item
  QModelIndex modelIndex;
  auto item = gui_util::getCurrentItem(imageList, &modelIndex);
  if (!item) { //no item selected
    QMessageBox::critical(this, "Error", "No item is selected.");
    return;
  }

  auto mainWin = getApp()->getMainWindow();

  //disable gui
  mainWin->setChangesEnabled(false);
  imageList->setEnabled(false);
  displayCornersCheck->setEnabled(false);
  displayReprojectionCheck->setEnabled(false);
  reprojectCurrentButton->setEnabled(false);
  reprojectAllButton->setEnabled(false);
  calibGroup->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  //do reprojection
  reprojectImage(modelIndex.row());

  //update gui
  updateImage(gui_util::loadCurrentImage(imageList, false));
  updateMinMaxError();

  //enable gui
  mainWin->setChangesEnabled(true);
  imageList->setEnabled(true);
  displayCornersCheck->setEnabled(true);
  displayReprojectionCheck->setEnabled(true);
  reprojectCurrentButton->setEnabled(true);
  reprojectAllButton->setEnabled(true);
  calibGroup->setEnabled(true);

  //restore cursor
  QApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_reprojectAllButton_clicked() {
  auto mainWin = getApp()->getMainWindow();

  //disable gui
  mainWin->setChangesEnabled(false);
  imageList->setEnabled(false);
  displayReprojectionCheck->setEnabled(false);
  reprojectCurrentButton->setEnabled(false);
  reprojectAllButton->setEnabled(false);
  calibGroup->setEnabled(false);

  //busy cursor
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();

  int rows = _model.rowCount();
  reprojectProgress->setMaximum(rows);
  reprojectProgress->setVisible(true);
  for (int i = 0; i < rows; ++i) {
    auto item = _model.item(i, 0);
    if (!item) { //invalid item
      QMessageBox::critical(this, "Error", "Invalid item.");
      break;
    }

    //do reprojection
    reprojectImage(i);

    //update progress bar
    reprojectProgress->setValue(i);
    QApplication::processEvents();
  }
  reprojectProgress->setVisible(false);

  //update gui
  updateImage(gui_util::loadCurrentImage(imageList, false));
  updateMinMaxError();

  //enable gui
  mainWin->setChangesEnabled(true);
  imageList->setEnabled(true);
  displayReprojectionCheck->setEnabled(true);
  reprojectCurrentButton->setEnabled(true);
  reprojectAllButton->setEnabled(true);
  calibGroup->setEnabled(true);

  //restore cursor
  QApplication::restoreOverrideCursor();
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::reprojectImage(int imageIndex) {
  if (imageIndex<0 || imageIndex>=_calib.imageData.size()) {
    return;
  }

  auto const& K = _calib.K;
  auto const& kc = _calib.kc;
  auto const& imageData = _calib.imageData[imageIndex];
  auto R = rodrigues(imageData.rotVec);
  auto const& T = imageData.T;
  auto const& worldPoints = imageData.worldPoints;
  auto const& imagePoints = imageData.imagePoints;

  int imageCount = static_cast<int>(_calib.imageCount);
  if (_reprojection.size()!=imageCount) {
    _reprojection.resize(imageCount);
  }

  auto & reproj = _reprojection[imageIndex];
  reproj.imagePoints.clear();
  reproj.rmsError =
    hw2::reprojectPoints(K, kc, R, T, worldPoints, imagePoints, reproj.imagePoints);
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::updateMinMaxError(void) {
  if (!_reprojection.size()) { //no data
    minErrorLine->clear();
    maxErrorLine->clear();
    return;
  }

  double minErr = _reprojection.first().rmsError;
  double maxErr = _reprojection.first().rmsError;
  foreach (auto const& item, _reprojection) {
    if (item.rmsError<minErr) { minErr = item.rmsError; }
    if (item.rmsError>maxErr) { maxErr = item.rmsError; }
  }

  minErrorLine->setText(QString::number(minErr));
  maxErrorLine->setText(QString::number(maxErr));
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_calibrateCameraButton_clicked() {
  std::cerr << "CamCalibPanel::on_calibrateCameraButton_clicked() {\n";

  int nImages = _calib.imageCount;
  if(nImages<3) {
    QMessageBox::critical(this, "Error", "Minimum Number of Calibration Images = 3");
    return;
  }

  int width  = _calib.imageSize.width();
  int height = _calib.imageSize.height();
  if(width<=0 || height<=0) {
    QMessageBox::critical(this, "Error", "imageWidth & imageHeight should be > 0");
    return;
  }
  cv::Size2i imageSize(width,height);

  int chessboardRows = chessboardRowsSpin->value();
  int chessboardCols = chessboardColsSpin->value();
  int nCorners       = chessboardRows*chessboardCols;
  
  std::cerr << "  nCorners = " << nCorners << "\n";

  std::vector<int> iImageUsed;
  std::vector<std::vector<cv::Point3f> > worldCornersActive;
  std::vector<std::vector<cv::Point2f> > imageCornersActive;
  QVector<CalibrationDataCamera::ImageData> & dataVec = _calib.imageData;
  for(int iImage=0; iImage<nImages;iImage++) {
    CalibrationDataCamera::ImageData & data = dataVec[iImage];
    if(data.imagePoints.size()!=nCorners || data.worldPoints.size()!=nCorners) {
      std::cerr << "  skipping image " << iImage << "\n";
      std::cerr << "    nImageCorners = " << data.imagePoints.size() << "\n";
      std::cerr << "    nWorldCorners = " << data.worldPoints.size() << "\n";
    } else {
      iImageUsed.push_back(iImage);
      QVector<Vector3d> & worldPoints = data.worldPoints;
      QVector<Vector2d> & imagePoints = data.imagePoints;

      std::vector<cv::Point3f> worldCorners;
      std::vector<cv::Point2f> imageCorners;
      for(int k=0;k<nCorners;k++) {
        Vector3d & wp = worldPoints[k];
        Vector2d & ip = imagePoints[k];
        cv::Point3f wc(wp(0),wp(1),wp(2));
        cv::Point2f ic(ip(0),ip(1));
        worldCorners.push_back(wc);
        imageCorners.push_back(ic);
      }

      worldCornersActive.push_back(worldCorners);
      imageCornersActive.push_back(imageCorners);
    }
  }

  int nImagesUsed = iImageUsed.size();
  std::cerr << "  nImagesUsed = " << nImagesUsed << "\n";

  // int calFlags = 0
  //   //+ cv::CALIB_FIX_K1
  //   //+ cv::CALIB_FIX_K2
  //   //+ cv::CALIB_ZERO_TANGENT_DIST
  //   + cv::CALIB_FIX_K3
  //   ;

  std::vector<cv::Mat> calRvecs;
  std::vector<cv::Mat> calTvecs;
  cv::Mat K;
  cv::Mat kc;

  // auto camError =
  //   cv::calibrateCamera(worldCornersActive,imageCornersActive,
  //                       imageSize,K,kc,calRvecs,calTvecs,calFlags,
  //                       cv::TermCriteria(cv::TermCriteria::COUNT +
  //                                        cv::TermCriteria::EPS, 50, DBL_EPSILON));

  double camError = hw4::calibrateCamera
    (worldCornersActive,imageCornersActive,imageSize,K,kc,calRvecs,calTvecs);

  // save the intrinsic parameters
  _calib.K(0,0) = K.at<double>(0,0);
  _calib.K(0,1) = K.at<double>(0,1);
  _calib.K(0,2) = K.at<double>(0,2);
  _calib.K(1,0) = K.at<double>(1,0);
  _calib.K(1,1) = K.at<double>(1,1);
  _calib.K(1,2) = K.at<double>(1,2);
  _calib.K(2,0) = K.at<double>(2,0);
  _calib.K(2,1) = K.at<double>(2,1);
  _calib.K(2,2) = K.at<double>(2,2);
  // save the lens distortion parameters
  _calib.kc(0) = kc.at<double>(0,0);
  _calib.kc(1) = kc.at<double>(0,1);
  _calib.kc(2) = kc.at<double>(0,2);
  _calib.kc(3) = kc.at<double>(0,3);
  _calib.kc(4) = kc.at<double>(0,4);
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

  std::cerr << "  camError = " << camError << "\n";

  _calib.calError     = camError;
  _calib.isCalibrated = true;

  _calib.print();
  calibGroupUpdate();

  std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_saveCalibrationButton_clicked() {
  if (!saveMatlabCalib()) {
    QMessageBox::critical
      (this, "Error", 
       QString("Matlab calibration file '%1' save failed.")
       .arg(MATLAB_CALIB_FILENAME));
  }
}

//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_loadCalibrationButton_clicked() {
  if (!loadMatlabCalib()) {
    QMessageBox::critical
      (this, "Error", 
       QString("Matlab calibration file '%1' load failed."
               " Check that the file exists in the current work directory.")
       .arg(MATLAB_CALIB_FILENAME));
  }
}


//////////////////////////////////////////////////////////////////////
void CamCalibPanel::on_clearCalibrationButton_clicked() {
  std::cerr << "CamCalibPanel::on_clearCalibrationButton_clicked() {\n";
  _calib.clear();
  calibGroupUpdate();
  std::cerr << "}\n";
}
