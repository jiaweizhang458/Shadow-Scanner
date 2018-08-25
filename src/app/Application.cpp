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

#include "Application.hpp"

#include <QFileInfo>
#include <QSettings>
#include <QSurfaceFormat>

Application::Application(int argc, char *argv[]) :
  QApplication(argc, argv),
  _mainWin() {
  //set default values for global settings
  QCoreApplication::setOrganizationName(APP_SHORT_NAME);
  QCoreApplication::setApplicationName(APP_SHORT_NAME);
  QSettings::setDefaultFormat(QSettings::IniFormat);

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  if (QCoreApplication::arguments().contains(QStringLiteral("--multisample")))
    format.setSamples(4);
  // format.setRenderableType(QSurfaceFormat::OpenGLES);
  QSurfaceFormat::setDefaultFormat(format);

#ifdef Q_OS_WIN
  QString a = QCoreApplication::applicationDirPath();
  QFileInfo appDir(QCoreApplication::applicationDirPath());
  if (appDir.isWritable()) {
    QSettings::setPath(QSettings::IniFormat,
                       QSettings::UserScope, appDir.filePath());
  } else { //cannot write in the app dir
    qDebug("Application::Application() current dir is not writable, settings will be created in the user directory");
  }
#endif //Q_OS_WIN

  //create main window
      MainWindow* mw = new MainWindow();
      // GuiMainWindow* mw = new MainWindow();
      // mw->setMinimumSize(500,500);
  // mw->updateGeometry();
  // mw->setMinimumSize(500,500);
  // mw->resize(600,600);
  // mw->show();

  _mainWin.reset(mw);
  _mainWin->show();
}

MainWindow * Application::getMainWindow(void) {
  return _mainWin.data();
}
