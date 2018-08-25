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

#define TURNTABLE_WRL "../../assets/turntable_160x32mm.wrl"

#include <iostream>
#include "MainWindow.hpp"

#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>

#include "Application.hpp"
#include "AboutDialog.hpp"

#include <io/LoaderWrl.h>
#include <io/SaverWrl.h>

int MainWindow::_timerInterval = 20;

QMap<QString,
     std::function<QWidget*(QWidget *parent)>> &
     MainWindow::getRegisteredPanels(void) {
  typedef QMap<QString,std::function<QWidget*(QWidget *parent)>> MapType;
  static MapType * _registeredPanels = new MapType;
  return *_registeredPanels;
}

bool MainWindow::registerPanel
(QString panelName,
 std::function<QWidget*(QWidget *parent)> panelFactory) {
  getRegisteredPanels().insert(panelName, panelFactory);
  return true;
}

MainWindow::MainWindow(QWidget * parent, Qt::WindowFlags flags) :
  QMainWindow(parent, flags),
  _bdryTop(5),
  _bdryBottom(5),
  _bdryLeft(5),
  _bdryCenter(5),
  _bdryRight(5),
  _toolsWidth(300) {

  _timer = new QTimer(this);
  _timer->setInterval(_timerInterval);

  setupUi(this);
  
  setWindowTitle(APP_NAME);

  connect(_timer, SIGNAL(timeout()), wrlGLWidget, SLOT(update()));

  wrlGLWidget->setBackgroundColor(qRgb(200, 200, 200));
  wrlGLWidget->setMaterialColor(qRgb(225, 150, 75));

  splitter->setOpaqueResize(false);

  // connect QSplitter::splitterMoved(int pos, int index) to

  LoaderWrl* wrlLoader = new LoaderWrl();
  _loader.registerLoader(wrlLoader);

  SaverWrl* wrlSaver = new SaverWrl();
  _saver.registerSaver(wrlSaver);
  
  //fill the panel combo
  auto panelNames = getRegisteredPanels().keys();
  foreach (auto item, panelNames) {
    panelCombo->insertItem(999, item);
  }

  //restore settings
  QSettings config;
  restoreGeometry(config.value("MainWindow/geometry").toByteArray());
  restoreState(config.value("MainWindow/windowState").toByteArray());
  auto panel = config.value("MainWindow/panel").toString().trimmed();
  if (!panel.isEmpty() && panelNames.contains(panel)) {
    panelCombo->setCurrentText(panel);
  }
  workDirLine->setText(config.value("MainWindow/workDirectory",
                                    QDir::currentPath()).toString());
  workDirUpdated();

  showStatusBarMessage("");
}

void MainWindow::showEvent(QShowEvent *event) {
  std::cerr << "MainWindow::showEvent() {\n";

  QSize splitterSize = splitter->size();
  QList<int> sizes = splitter->sizes();
   
  int height      = splitterSize.height();
  int imgWidth    = sizes[0];
  int wrlWidth    = sizes[1];
  int panelsWidth = sizes[2];
  
  std::cerr << "  height      = " << height << "\n";
  std::cerr << "  imgWidth    = " << imgWidth << "\n";
  std::cerr << "  wrlWidth    = " << wrlWidth << "\n";
  std::cerr << "  panelsWidth = " << panelsWidth << "\n";

  // if(height>0 && imgWidth>0 && wrlWidth>0 && panelsWidth>0) {
  //
  //   int dx = height*9/16-imgWidth;
  //   imgWidth += dx;
  //   wrlWidth -= dx;
  //
  //   sizes.clear();
  //   sizes.append(imgWidth);
  //   sizes.append(wrlWidth);
  //   sizes.append(panelsWidth);
  //
  //   splitter->setSizes(sizes);
  // }

  std::cerr << "}\n";
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  std::cerr << "MainWindow::resizeEvent() {\n";

  QSize splitterSize = splitter->size();
  QList<int> sizes = splitter->sizes();
  
  int height      = splitterSize.height();
  int imgWidth    = sizes[0];
  int wrlWidth    = sizes[1];
  int panelsWidth = sizes[2];

  
  std::cerr << "  height      = " << height << "\n";
  std::cerr << "  imgWidth    = " << imgWidth << "\n";
  std::cerr << "  wrlWidth    = " << wrlWidth << "\n";
  std::cerr << "  panelsWidth = " << panelsWidth << "\n";

  // if(height>0 && imgWidth>0 && wrlWidth>0 && panelsWidth>0) {
  //  
  //   int dx = height*9/16-imgWidth;
  //   imgWidth += dx;
  //   wrlWidth -= dx;
  //
  //   sizes.clear();
  //   sizes.append(imgWidth);
  //   sizes.append(wrlWidth);
  //   sizes.append(panelsWidth);
  //
  //   splitter->setSizes(sizes);
  //
  // }

  std::cerr << "}\n";
}

void MainWindow::resizeSplitter() {
  std::cerr << "MainWindow::resizeSplitter() {\n";

  QSize splitterSize = splitter->size();
  QList<int> sizes = splitter->sizes();

  if(sizes.size()==3) {
  
    int height      = splitterSize.height();
    int imgWidth    = sizes[0];
    int wrlWidth    = sizes[1];
    int panelsWidth = sizes[2];
    
    std::cerr << "  height      = " << height << "\n";
    std::cerr << "  imgWidth    = " << imgWidth << "\n";
    std::cerr << "  wrlWidth    = " << wrlWidth << "\n";
    std::cerr << "  panelsWidth = " << panelsWidth << "\n";
    
    if(height>0 && imgWidth>0 && wrlWidth>0 && panelsWidth>0) {
      
      int dx = height*9/16-imgWidth;
      imgWidth += dx;
      wrlWidth -= dx;
      
      sizes.clear();
      sizes.append(imgWidth);
      sizes.append(wrlWidth);
      sizes.append(panelsWidth);
      
      splitter->setSizes(sizes);
      
    }
  }

  std::cerr << "}\n";
}

void MainWindow::closeEvent(QCloseEvent *e) {
  //save settings
  QSettings config;
  config.setValue("MainWindow/geometry", saveGeometry());
  config.setValue("MainWindow/windowState", saveState());
  config.setValue("MainWindow/panel", panelCombo->currentText());
  config.setValue("MainWindow/workDirectory", workDirLine->text());
  QMainWindow::closeEvent(e);
}

void MainWindow::on_splitter_splitterMoved(int pos, int index) {
  std::cerr << "MainWindow::on_splitter_splitterMoved() {\n";
  std::cerr << "  pos   = " << pos   << "\n";
  std::cerr << "  index = " << index << "\n";
  std::cerr << "}\n";
}

void MainWindow::on_actionAbout_triggered() {
  AboutDialog d;
  d.exec();
}

void MainWindow::on_panelCombo_currentIndexChanged(int index) {
  qDebug("MainWindow::on_panelCombo_currentIndexChanged: index %d", index);
  setPanel(panelCombo->currentText());
}

void MainWindow::setPanel(QString name) {
  //remove current widgets from panel
  auto children = currPanelWidget->children();
  foreach(auto child, children) {
    auto widget = dynamic_cast<QWidget*>(child);
    if (widget) {
      widget->hide();
      widget->deleteLater();
    }
  }
  
  //delete current layout
  auto oldLayout = currPanelWidget->layout();
  if (oldLayout) {
    delete oldLayout;
  }

  //create a new layout
  QVBoxLayout * newLayout = new QVBoxLayout(currPanelWidget);
  newLayout->setContentsMargins(0, 0, 0, 0);

  //add new panel
  QWidget * panel = nullptr;
  
  auto & registeredPanels = getRegisteredPanels();
  auto panelFactory = registeredPanels.find(name);
  if (panelFactory!= registeredPanels.end()) { //panel found
    panel = (*panelFactory)(currPanelWidget);
  }

  if (!panel) { //panel not found
    imgGLWidget->clearImage();
    qDebug("MainWindow::on_panelCombo_currentIndexChanged: panel '%s' not found",
           qPrintable(name));
    return;
  }

  //setup new panel
  newLayout->addWidget(panel);
  panel->show();
}

void MainWindow::on_workDirButton_clicked() {
  QString dir =
    QFileDialog::getExistingDirectory
    (this, "Select Work Directory",
     workDirLine->text(),
     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!dir.isEmpty()) {
    workDirLine->setText(dir);
    workDirUpdated();
  }
}

void MainWindow::on_workDirLine_editingFinished() {
  workDirUpdated();
}

void MainWindow::workDirUpdated(void) {
  static QMutex mutex;
  if (!mutex.tryLock(0)) return;

  QString dirName = workDirLine->text();;
  QString prevDirName = workDirLine->property("prevValue").toString();

  if (prevDirName.isEmpty()) prevDirName = QDir::currentPath();
  if (dirName.isEmpty()) dirName = prevDirName;

  QDir workDir(dirName);
  if (!workDir.exists()) { //doesn't exist: ask to create
    auto button =
      QMessageBox::question(nullptr,
                            "Warning", "Directory '" + dirName
                            + "' does not exist.\nDo you want to create it?");
    if (button == QMessageBox::Yes) {
      if (workDir.mkpath(dirName)) {
        qInfo("Directory created: %s", qPrintable(dirName));
      } else { //couldn't create
        qCritical("Directory create failed: %s", qPrintable(dirName));
        dirName = prevDirName;
      }
    } else { //user cliked 'No'
      dirName = prevDirName;
    }
  }//workDir exists

  //update the value
  if (workDirLine->text() != dirName) {
    workDirLine->setText(dirName);
  }
  workDirLine->setProperty("prevValue", dirName);
  emit workDirectoryChanged();
  mutex.unlock();
}

QString MainWindow::getWorkDirectory(void) const {
  return workDirLine->text();
}

void MainWindow::setChangesEnabled(bool enabled) {
  panelCombo->setEnabled(enabled);
  workDirLabel->setEnabled(enabled);
  workDirLine->setEnabled(enabled);
  workDirButton->setEnabled(enabled);
}

// inherited from WrlMainWindows

void MainWindow::updateState() {
  // _toolsWidget->updateState();
}

void MainWindow::showStatusBarMessage(const QString & message) {
  statusbar->showMessage(message);
}

void MainWindow::refresh() {
  wrlGLWidget->update();
}

WrlViewerData& MainWindow::getData() const {
  return wrlGLWidget->getData();
}

SceneGraph* MainWindow::getSceneGraph() {
  return wrlGLWidget->getSceneGraph();
}

void MainWindow::setSceneGraph(SceneGraph* pWrl, bool resetHomeView) {
  wrlGLWidget->setSceneGraph(pWrl,resetHomeView);
}

SceneGraph* MainWindow::loadSceneGraph(const char* fname) {
  static char str[1024];
  sprintf(str,"Trying to load \"%s\" ...",fname);
  showStatusBarMessage(QString(str));
  SceneGraph* pWrl = new SceneGraph();
  if(_loader.load(fname,*pWrl)) { // if success
    sprintf(str,"Loaded \"%s\"",fname);
    pWrl->updateBBox();
    wrlGLWidget->setSceneGraph(pWrl,true);
    updateState();
  } else {
    sprintf(str,"Unable to load \"%s\"",fname);
    delete pWrl;
    pWrl = (SceneGraph*)0;
  }
  showStatusBarMessage(QString(str));
  return pWrl;
}

int MainWindow::getImgGLWidgetWidth() {
  return imgGLWidget->size().width();
}

int MainWindow::getImgGLWidgetHeight() {
  return imgGLWidget->size().height();
}

int MainWindow::getWrlGLWidgetWidth() {
  return wrlGLWidget->size().width();
}

int MainWindow::getWrlGLWidgetHeight() {
  return wrlGLWidget->size().height();
}

int MainWindow::getPanelsWidgetWidth() {
  return panelsWidget->size().width();
}

int MainWindow::getPanelsWidgetHeight() {
  return panelsWidget->size().height();
}

void MainWindow::showImgGLWidget(bool value) {
  imgGLWidget->setVisible(value);
}

void MainWindow::showWrlGLWidget(bool value) {
  wrlGLWidget->setVisible(value);
}

void MainWindow::resizeImgGLWidget(int w, int h) {
  imgGLWidget->resize(w,h);
}

void MainWindow::resizeWrlGLWidget(int w, int h) {
  wrlGLWidget->resize(w,h);
}

//////////////////////////////////////////////////////////////////////
void MainWindow::on_wrlFileLoadButton_clicked() {

  std::string filename;

  // stop animation
  _timer->stop();

  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::ExistingFile); // allowed to select only one 
  fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
  fileDialog.setNameFilter(tr("VRML Files (*.wrl)"));
  QStringList fileNames;
  if(fileDialog.exec()) {
    fileNames = fileDialog.selectedFiles();
    if(fileNames.size()>0)
      filename = fileNames.at(0).toStdString();
  }

  if (filename.empty()) {
    showStatusBarMessage("load filename is empty");
  } else {
    loadSceneGraph(filename.c_str());
  } 

  // restart animation
  _timer->start(_timerInterval);
}

// //////////////////////////////////////////////////////////////////////
// void MainWindow::on_turntableFileLoadButton_clicked() {
//
//   std::string filename(TURNTABLE_WRL);
//
//   // stop animation
//   _timer->stop();
//
//   if (filename.empty()) {
//     showStatusBarMessage("load filename is empty");
//   } else {
//     loadSceneGraph(filename.c_str());
//   } 
//
//   // restart animation
//   _timer->start(_timerInterval);
// }

//////////////////////////////////////////////////////////////////////
void MainWindow::on_wrlFileSaveButton_clicked() {

  std::string filename;

  // stop animation
  _timer->stop();

  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::AnyFile); // allowed to select only one 
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setNameFilter(tr("VRML Files (*.wrl)"));
  QStringList fileNames;
  if(fileDialog.exec()) {
    fileNames = fileDialog.selectedFiles();
    if(fileNames.size()>0)
      filename = fileNames.at(0).toStdString();
  }

  // restart animation
  _timer->start(_timerInterval);

  if (filename.empty()) {
    showStatusBarMessage("save filename is empty");
  } else {

    static char str[1024];

    sprintf(str,"Saving \"%s\" ...",filename.c_str());
    showStatusBarMessage(QString(str));

    SceneGraph* pWrl = wrlGLWidget->getSceneGraph();

    if(_saver.save(filename.c_str(),*pWrl)) {
      sprintf(str,"Saved \"%s\"", filename.c_str());
    } else {
      sprintf(str,"Unable to save \"%s\"",filename.c_str());
    }

    showStatusBarMessage(QString(str));
  }
}
