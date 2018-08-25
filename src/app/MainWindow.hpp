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

#ifndef _MainWindow_hpp_
#define _MainWindow_hpp_

#include <QMainWindow>
#include <QTimer>

#include <functional>

#include <io/AppLoader.h>
#include <io/AppSaver.h>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow, protected Ui_MainWindow
{
  Q_OBJECT;

  static QMap<QString,
              std::function<QWidget*(QWidget *parent)>> & getRegisteredPanels(void);

  void setPanel(QString name);
  void workDirUpdated(void);

  void showEvent(QShowEvent *event);
  void resizeEvent(QResizeEvent *event);

public:

  MainWindow(QWidget * parent = 0, Qt::WindowFlags flags = 0);

  void setChangesEnabled(bool enabled);

  QString getWorkDirectory(void) const;

  void closeEvent(QCloseEvent * e);

  static bool registerPanel(QString panelName,
                            std::function<QWidget*(QWidget *parent)> panelFactory);

  void timerStop()  { _timer->stop(); }
  void timerStart() { _timer->start(_timerInterval); }
  void showStatusBarMessage(const QString & message);
  void updateState();
  void refresh();
  void resizeSplitter();

  ImgGLWidget * getImgGLWidget(void) { return imgGLWidget; }
  WrlGLWidget * getWrlGLWidget(void) { return wrlGLWidget; }

  int  getImgGLWidgetWidth();
  int  getImgGLWidgetHeight();
  int  getWrlGLWidgetWidth();
  int  getWrlGLWidgetHeight();
  int  getPanelsWidgetWidth();
  int  getPanelsWidgetHeight();

  void resizeImgGLWidget(int w, int h);
  void resizeWrlGLWidget(int w, int h);
  
  void showWrlGLWidget(bool value);
  void showImgGLWidget(bool value);

  WrlViewerData& getData() const;

  SceneGraph*    getSceneGraph();
  void           setSceneGraph(SceneGraph* pWrl, bool resetHomeView);
  SceneGraph*    loadSceneGraph(const char* fname);

signals:
  void workDirectoryChanged();

protected slots:

  void on_splitter_splitterMoved(int pos, int index);
  void on_actionAbout_triggered();
  void on_panelCombo_currentIndexChanged(int index);
  void on_workDirButton_clicked();
  void on_workDirLine_editingFinished();

public slots:

  void on_wrlFileLoadButton_clicked();
  void on_wrlFileSaveButton_clicked();
  // void on_turntableFileLoadButton_clicked();

private:

  AppLoader       _loader;
  AppSaver        _saver;
  QTimer         *_timer;

  // WrlGLWidget    *_glWidget;
  // WrlToolsWidget *_toolsWidget;
  // QWidget*        _centralWidget; 
  // QStatusBar     *_statusBar;

  int             _bdryTop;
  int             _bdryBottom;;
  int             _bdryLeft;
  int             _bdryCenter;
  int             _bdryRight;
  int             _toolsWidth;

  static int      _timerInterval;

};

#endif //_MainWindow_hpp_
