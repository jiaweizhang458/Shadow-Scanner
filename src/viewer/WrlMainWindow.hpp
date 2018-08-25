//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-14 15:45:08 taubin>
//------------------------------------------------------------------------
//
// WrlMainWindow.hpp
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

#ifndef _WRL_MAINWINDOW_HPP
#define _WRL_MAINWINDOW_HPP

#include <string>

#include <QMainWindow>
#include <QTimer>
#include <QGridLayout>

#include "io/AppLoader.h"
#include "io/AppSaver.h"
#include "WrlGLWidget.hpp"
#include "WrlToolsWidget.hpp"

QT_FORWARD_DECLARE_CLASS(QOpenGLWidget)

class WrlMainWindow : public QMainWindow {

  Q_OBJECT

public:

  WrlMainWindow();

  void timerStop()  { _timer->stop(); }
  void timerStart() { _timer->start(_timerInterval); }
  void showStatusBarMessage(const QString & message);

  int  getGLWidgetWidth();
  int  getGLWidgetHeight();
  void setGLWidgetSize(int width, int height);

  WrlViewerData& getData() const;
  SceneGraph*    getSceneGraph();
  void           setSceneGraph(SceneGraph* pWrl, bool resetHomeView);

  SceneGraph*    loadSceneGraph(const char* fname);

  void updateState();
  void refresh();

private slots:

  void onMenuFileExit();
  void onMenuFileLoad();
  void onMenuFileSave();
  void onMenuFileLoadBunny();
  void onMenuToolsShow();
  void onMenuToolsHide();
  void onMenuHelpAbout();

protected:

  virtual void resizeEvent(QResizeEvent * event) Q_DECL_OVERRIDE;
  void _resize(int width, int height);

private:

  AppLoader       _loader;
  AppSaver        _saver;
  WrlGLWidget    *_glWidget;
  WrlToolsWidget *_toolsWidget;
  QWidget*        _centralWidget; 
  QStatusBar     *_statusBar;
  QTimer         *_timer;

  int             _bdryTop;
  int             _bdryBottom;;
  int             _bdryLeft;
  int             _bdryCenter;
  int             _bdryRight;
  int             _toolsWidth;

  static int      _timerInterval;

};

#endif // _WRL_MAINWINDOW_HPP
