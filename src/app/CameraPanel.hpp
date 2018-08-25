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

#ifndef _CameraPanel_hpp_
#define _CameraPanel_hpp_

#include <QWidget>
#include <QImage>
#include <QElapsedTimer>
#include <QStandardItemModel>
#include <QAtomicInteger>

#include "ui_CameraPanel.h"

class CameraInterface;

class CameraPanel : public QWidget, protected Ui_CameraPanel
{
  Q_OBJECT;

  static bool _registered;
  static bool registerPanel();

  QSharedPointer<CameraInterface> _camera;
  size_t _imageCount;
  QElapsedTimer _fpsTimer;
  QStandardItemModel _model;

  QAtomicInteger<int> _captureOneImage;

  void loadSettings(void);
  void saveSettings(void);

  void getAndSetupCamera(void);
  void updateCameraResolutions(void);
  void changeCameraResolution(void);

  void updateImage(QImage image, int rotation = 0);

  void showEvent(QShowEvent * event);

public:
  CameraPanel(QWidget * parent = 0, Qt::WindowFlags f = 0);
  ~CameraPanel();

signals:
  void setInfoLabel(QString text);

public slots:
  void _on_captureStarted();
  void _on_captureProgress();
  void _on_captureFinished();

protected slots:
  void on_cameraCombo_currentIndexChanged(int index);
  void on_imageRotCombo_currentIndexChanged(int index);
  void on_previewButton_clicked(bool checked = false);
  void on_captureButton_clicked();
  void _on_currentImageChanged(const QModelIndex & current, const QModelIndex & previous);

#ifdef HAVE_IMG
  void updateInfo(ImageBuffer const& image);
#endif //HAVE_IMG
};

#endif //_CameraPanel_hpp_
