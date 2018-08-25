//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-12 12:58:03 taubin>
//------------------------------------------------------------------------
//
// WrlMainWindow.cpp
//
// Software developed for the Fall 2015 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2015, Gabriel Taubin
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
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#define BUNNY_WRL \
  "/Users/taubin/Teaching/C++/engn2912b/2015/projects/IfsViewer2015-data/bunny-opts.wrl"

#include <iostream>

#include "WrlMainWindow.hpp"
#include "WrlGLWidget.hpp"
#include "WrlAboutDialog.hpp"

#include <QApplication>
#include <QMenuBar>
#include <QGroupBox>
#include <QStatusBar>
#include <QFileDialog>
#include <QRect>
#include <QMargins>

#include "io/LoaderWrl.h"
#include "io/SaverWrl.h"

int WrlMainWindow::_timerInterval = 20;

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::showStatusBarMessage(const QString & message) {
  _statusBar->showMessage(message);
}

//////////////////////////////////////////////////////////////////////
WrlMainWindow::WrlMainWindow():
  _bdryTop(5),
  _bdryBottom(5),
  _bdryLeft(5),
  _bdryCenter(5),
  _bdryRight(5),
  _toolsWidth(300) {

  setWindowIcon(QIcon("qt.icns"));
  setWindowTitle("WrlViewer3-2015 | ENGN2912B");

  LoaderWrl* wrlLoader = new LoaderWrl();
  _loader.registerLoader(wrlLoader);

  SaverWrl* wrlSaver = new SaverWrl();
  _saver.registerSaver(wrlSaver);

  // QColor clearColor    = qRgb(175, 200, 150);
  QColor clearColor    = qRgb(200, 200, 200);
  QColor materialColor = qRgb(225, 150, 75);
  _glWidget    =  new WrlGLWidget(this, clearColor, materialColor);
  _toolsWidget = new WrlToolsWidget(this);

  _centralWidget = new QWidget(this);
  setCentralWidget(_centralWidget);

  _glWidget->setParent(_centralWidget);
  _toolsWidget->setParent(_centralWidget);
  _toolsWidget->setMinimumWidth(300);

  _glWidget->show();
  // _toolsWidget->hide();
  _toolsWidget->show();

  _statusBar = new QStatusBar(this);
  QFont font;
  font.setPointSize(9);
  _statusBar->setFont(font);
  setStatusBar(_statusBar);
  // _statusBar->hide();

  //////////////////////////////////////////////////
  QMenu *fileMenu = menuBar()->addMenu("&File");

  QAction *exit = new QAction("E&xit" , fileMenu);
  fileMenu->addAction(exit);
  connect(exit, SIGNAL(triggered(bool)),
          this, SLOT(onMenuFileExit()));
  
  QAction *load = new QAction("Load" , fileMenu);
  fileMenu->addAction(load);
  connect(load, SIGNAL(triggered(bool)),
          this, SLOT(onMenuFileLoad()));
  
  QAction *save = new QAction("Save" , fileMenu);
  fileMenu->addAction(save);
  connect(save, SIGNAL(triggered(bool)),
          this, SLOT(onMenuFileSave()));
  
  QAction *qtLogo = new QAction("Qt Logo" , fileMenu);
  fileMenu->addAction(qtLogo);
  connect(qtLogo, SIGNAL(triggered(bool)),
          _glWidget, SLOT(setQtLogo()));
  
  QAction *bunny = new QAction("Bunny" , fileMenu);
  fileMenu->addAction(bunny);
  connect(bunny, SIGNAL(triggered(bool)),
          this, SLOT(onMenuFileLoadBunny()));

  //////////////////////////////////////////////////
  QMenu *toolsMenu = menuBar()->addMenu("&Tools");

  QAction *showTools = new QAction("Show" , toolsMenu);
  toolsMenu->addAction(showTools);
  connect(showTools, SIGNAL(triggered(bool)),
          this, SLOT(onMenuToolsShow()));

  QAction *hideTools = new QAction("Hide" , toolsMenu);
  toolsMenu->addAction(hideTools);
  connect(hideTools, SIGNAL(triggered(bool)),
          this, SLOT(onMenuToolsHide()));

  //////////////////////////////////////////////////
  QMenu *helpMenu = menuBar()->addMenu("&Help");

  QAction *about = new QAction("About QtOpenGL",helpMenu);
  helpMenu->addAction(about);
  connect(about, SIGNAL(triggered(bool)),
           this, SLOT(onMenuHelpAbout()));

  // for animation
  _timer = new QTimer(this);
  _timer->setInterval(_timerInterval);
  connect(_timer, SIGNAL(timeout()), _glWidget, SLOT(update()));

  showStatusBarMessage("");
  _glWidget->setFocus();
}

//////////////////////////////////////////////////////////////////////
SceneGraph* WrlMainWindow::loadSceneGraph(const char* fname) {
  static char str[1024];
  sprintf(str,"Trying to load \"%s\" ...",fname);
  showStatusBarMessage(QString(str));
  SceneGraph* pWrl = new SceneGraph();
  if(_loader.load(fname,*pWrl)) { // if success
    sprintf(str,"Loaded \"%s\"",fname);
    pWrl->updateBBox();
    _glWidget->setSceneGraph(pWrl,true);
    _toolsWidget->updateState();
  } else {
    sprintf(str,"Unable to load \"%s\"",fname);
    delete pWrl;
    pWrl = (SceneGraph*)0;
  }
  showStatusBarMessage(QString(str));
  return pWrl;
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuFileLoad() {

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

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuFileLoadBunny() {

  std::string filename(BUNNY_WRL);

  // stop animation
  _timer->stop();

  if (filename.empty()) {
    showStatusBarMessage("load filename is empty");
  } else {
    loadSceneGraph(filename.c_str());
  } 

  // restart animation
  _timer->start(_timerInterval);
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuFileSave() {

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

    SceneGraph* pWrl = _glWidget->getSceneGraph();

    if(_saver.save(filename.c_str(),*pWrl)) {
      sprintf(str,"Saved \"%s\"", filename.c_str());
    } else {
      sprintf(str,"Unable to save \"%s\"",filename.c_str());
    }

    showStatusBarMessage(QString(str));
  }
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuFileExit() {
  close();
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuToolsShow() {
  int c3dWidth  = getGLWidgetWidth();
  int c3dHeight = getGLWidgetHeight();
  _toolsWidget->show();
  setGLWidgetSize(c3dWidth,c3dHeight);
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuToolsHide() {
  int c3dWidth  = getGLWidgetWidth();
  int c3dHeight = getGLWidgetHeight();
  _toolsWidget->hide();
  setGLWidgetSize(c3dWidth,c3dHeight);
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::onMenuHelpAbout() {
  WrlAboutDialog dialog(this);
  dialog.exec();
}

//////////////////////////////////////////////////////////////////////
int WrlMainWindow::getGLWidgetWidth() {
  return _glWidget->size().width();
}

//////////////////////////////////////////////////////////////////////
int WrlMainWindow::getGLWidgetHeight() {
  return _glWidget->size().height();
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::_resize(int width, int height) {
  std::cout << "WrlMainWindow::_resize(int width, int height) {\n";

  QMenuBar   *menuBar   = this->menuBar();
  QStatusBar *statusBar = this->statusBar();

  int mbH = (menuBar  )?menuBar->height()  :0;
  int sbH = (statusBar)?statusBar->height():0;

  std::cout << "  menuBar.height   = "<< mbH <<"\n";
  std::cout << "  statusBar.height = "<< sbH<<"\n";

  int x0 = _bdryLeft;
  int w0 = width-_bdryLeft-_bdryRight;
  int w1 = _toolsWidth;
  int y  = _bdryTop;
  int h  = height-_bdryTop-_bdryBottom-sbH-mbH;

  if(_toolsWidget->isVisible()) {
    w0 -= (w1+_bdryCenter);
    int x1 = x0+w0+_bdryCenter;
    _toolsWidget->setGeometry(x1,y,w1,h);
  }
  _glWidget->setGeometry(x0,y,w0,h);
  _toolsWidget->updateState();

  std::cout << "}\n";
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::resizeEvent(QResizeEvent* event) {
  QSize size = event->size();
  _resize(size.width(),size.height());
}

//////////////////////////////////////////////////////////////////////
void WrlMainWindow::setGLWidgetSize(int c3dWidth, int c3dHeight) {
  int width  = _bdryLeft + c3dWidth  +  _bdryRight;
  if(_toolsWidget->isVisible()) width  += _bdryCenter + _toolsWidth;
  QMenuBar   *menuBar   = this->menuBar();
  QStatusBar *statusBar = this->statusBar();
  int mbH = (menuBar  )?menuBar->height()  :0;
  int sbH = (statusBar)?statusBar->height():0;
  int height = _bdryTop  + c3dHeight + _bdryBottom + sbH+mbH;
  this->resize(width,height);
  // _resize(width,height);
}

WrlViewerData& WrlMainWindow::getData() const {
  return _glWidget->getData();
}

SceneGraph* WrlMainWindow::getSceneGraph() {
  return _glWidget->getSceneGraph();
}

void WrlMainWindow::setSceneGraph(SceneGraph* pWrl, bool resetHomeView) {
  _glWidget->setSceneGraph(pWrl,resetHomeView);
}

void WrlMainWindow::updateState() {
  _toolsWidget->updateState();
}

void WrlMainWindow::refresh() {
  _glWidget->update();
}
