/*
Copyright (c) 2014, Daniel Moreno and Gabriel Taubin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Brown University nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO AND GABRIEL TAUBIN BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __V4L2Camera_hpp__
#define __V4L2Camera_hpp__

#include "CameraInterface.hpp"

#include <QMap>
#include <QThread>
#include <QString>
#include <QStringList>

#include "Log.hpp"
#include "ImageBuffer.hpp"

struct v4l2_queryctrl;

class V4L2Camera : public CameraInterface
{
  Q_OBJECT; ADD_LOG; static const bool silent = false; //for log functions

  static QMap<QString,QString> _globalCameraList;

  int             _cam;     // camera file descriptor
  QString         _name;
  QThread         _thread;
  QStringList     _formatList;
  QMap<QString,size_t>      _controlMap;
  QMap<QString,QStringList> _controlMenu;

  QStringList listCameraFormats(void);
  void findControls(void);
  void disableAuto(void);
  bool stopVideo(void);
  V4L2Camera(int cam, QString name = "V4L2Cam");

  static int openDevice(QString const& device);
  static const char * getFormatName(size_t pixelformat);
  static size_t getPixelFormat(QString format);
  static QStringList getControlMenu(int cam, v4l2_queryctrl const& queryctrl);
  static bool getPropertyValue(int cam, size_t property, QVariant & value,
                               int *minVal=NULL, int *maxVal=NULL, int *defaultVal=NULL);
  static bool setPropertyValue(int cam, size_t property, QVariant value);

  static QString name2var(unsigned char * name);

signals:
  void stopWorker();

public:
  
  //-- camera interface BEGIN ---------------------------------------------
  bool start();
  inline void stop() { stopVideo(); }

  QStringList getPropertyList(void);
  QVariant getCameraProperty(QString propertyName);
  void setCameraProperty(QString propertyName, QVariant value);

  inline QStringList const& getFormatList(void) const { return _formatList; }
  //-- camera interface END -----------------------------------------------

  ~V4L2Camera();

  double getExposure(void) const;
  void setExposure(double value);
  double getContrast(void) const;
  void setContrast(double value);
  double getBrightness(void) const;
  void setBrightness(double value);
  QString getFormat(void) const;
  void setFormat(QString const& value);

  static QStringList getCameraList(void);
  static QSharedPointer<CameraInterface> getCamera(QString cameraName);
};

//------------------------------------------------------------------------------------------
// V4L2CameraWorker
//------------------------------------------------------------------------------------------

class V4L2CameraWorker : public QObject
{
  Q_OBJECT; ADD_LOG; static const bool silent = false; //for log functions
  enum {StateStop=0, StateInit, StateStreaming};

  int                       _cam;
  std::vector<ImageBuffer>  _buffer;
  volatile int              _state;

  bool allocateBuffers(size_t n = 4);
  bool startStreaming(void);
  bool stopStreaming(void);
  void deallocateBuffers(void);
  bool readFrame(void);
public:
  V4L2CameraWorker(int cam);
  ~V4L2CameraWorker();
  inline bool isStreaming(void) const { return (_state==StateStreaming); }

public slots:
  void start();
  inline void stop() { _state = StateStop; }

signals:
  void newImage(ImageBuffer const& image);
};

#endif //__V4L2Camera_hpp__
