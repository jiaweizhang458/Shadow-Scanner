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

#ifndef _DShowCamera_hpp_
#define _DShowCamera_hpp_

#include "CameraInterface.hpp"

#include <QMap>
#include <QString>
#include <QStringList>

#ifdef HAVE_IMG
# include <img/ImageBuffer.hpp>
#endif //HAVE_IMG

#include "Log.hpp"

#include "rpc.h"
#include "strmif.h"

class DShowCamera : public CameraInterface
{
  ADD_LOG; static const bool silent = false; //for log functions

  static QMap<QString,QString> _globalCameraList;

  QString         _monikerName;
  QString         _displayName;

  IBaseFilter   * _source;
  IFilterGraph2 * _graph;
  QString         _format;
  QStringList     _formatList;

#ifdef HAVE_IMG
  typedef ImageBuffer::FormatType FormatType;
#endif //HAVE_IMG

  static IBaseFilter * buildFilterFromMoniker(QString const& monikerName);
  static QStringList getCameraFormatList(IBaseFilter *pFilter);
  static bool getConnectedPinMediaType(IBaseFilter *pFilter, AM_MEDIA_TYPE &mt);
  static bool getPreferredSource(IBaseFilter *pFilter, QString requestedFormat, 
                  /*out*/ _AMMediaType &preferredMediaType, /*out*/ IPin **preferredPin = NULL);
  
  bool checkSourceFilter(void) const; //not true const: it will create a filter if required
  bool buildGraph(IBaseFilter * sourceFilter, QString requestedFormat = QString());
  bool stopVideo(void);

  DShowCamera(QString monikerName, QString displayName = "DShowCam");

public:

  inline bool isValid(void) const { return !_formatList.isEmpty(); }

  //-- camera interface BEGIN ---------------------------------------------
  bool start();
  inline void stop() { stopVideo(); }

  QStringList getPropertyList(void);
  QVariant getCameraProperty(QString propertyName);
  void setCameraProperty(QString propertyName, QVariant value);

  inline QStringList const& getFormatList(void) const { return _formatList; }
  //-- camera interface END -----------------------------------------------

  ~DShowCamera();

  double getExposure(void) const;
  void setExposure(double value);
  double getContrast(void) const;
  void setContrast(double value);
  double getBrightness(void) const;
  void setBrightness(double value);
  QString getFormat(void) const;
  void setFormat(QString format);

  static QStringList getCameraList(void);
  static QSharedPointer<CameraInterface> getCamera(QString cameraName);

  static QString getFormatType(AM_MEDIA_TYPE * mediaType);

#ifdef HAVE_IMG
  static FormatType getImageBufferFormatType(AM_MEDIA_TYPE * mediaType, QSize * size = nullptr);
#endif //HAVE_IMG
};

//------------------------------------------------------------------------------------------
// DShowCameraCallback
//------------------------------------------------------------------------------------------

// {0579154A-2B53-4994-B0D0-E773148EFF85}
DEFINE_GUID(IID_ISampleGrabberCB,
  0x0579154A, 0x2B53, 0x4994, 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85);

MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SampleCB(double SampleTime, IMediaSample *pSample) = 0;
  virtual HRESULT STDMETHODCALLTYPE BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;
};

// {6B652FFF-11FE-4fce-92AD-0266B5D7C78F}
DEFINE_GUID(IID_ISampleGrabber,
  0x6B652FFF, 0x11FE, 0x4fce, 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F);

MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown
{
public:
  virtual HRESULT STDMETHODCALLTYPE SetOneShot(BOOL OneShot) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetBufferSamples(BOOL BufferThem) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer(/* [out][in] */ long *pBufferSize, /* [out] */ long *pBuffer) = 0;
  virtual HRESULT STDMETHODCALLTYPE GetCurrentSample(/* [retval][out] */ IMediaSample **ppSample) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetCallback(ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;
};

class DShowCameraCallback : public QObject, public ISampleGrabberCB
{
  Q_OBJECT;
  ADD_LOG; static const bool silent = false; //for log functions

#ifdef HAVE_IMG
  ImageBuffer _buffer;
#endif //HAVE_IMG

  LONG _refCount;

public:
  DShowCameraCallback();
  ~DShowCameraCallback();

  void SetMediaType(AM_MEDIA_TYPE &mt);

  virtual HRESULT __stdcall SampleCB(double SampleTime, IMediaSample *pSample);
  virtual HRESULT __stdcall BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen);
  virtual HRESULT __stdcall QueryInterface(REFIID riid, LPVOID *ppvObj);
  virtual ULONG   __stdcall AddRef();
  virtual ULONG   __stdcall Release();

signals:
#ifdef HAVE_IMG
  void newImage(ImageBuffer const& image);
#endif //HAVE_IMG
};

#endif //_DShowCamera_hpp_
