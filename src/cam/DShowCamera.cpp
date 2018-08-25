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

#include "DShowCamera.hpp"

#include <QSize>
#include <QVariant>
#include <QMetaType>
#include <QStringList>
#include <QSharedPointer>

#include <Dshow.h>
#include "LVUVCPublic.h" //logitech public properties

#include "assert.h"

// {C1F400A0-3F08-11D3-9F0B-006008039E37}
DEFINE_GUID(CLSID_SampleGrabber,
  0xC1F400A0, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37); //qedit.dll

DEFINE_GUID(CLSID_NullRenderer,
  0xC1F400A4, 0x3F08, 0x11D3, 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37); //qedit.dll

inline void FreeMediaType(AM_MEDIA_TYPE& mt)
{
  if (mt.cbFormat != 0)
  {
    CoTaskMemFree((PVOID)mt.pbFormat);
    mt.cbFormat = 0;
    mt.pbFormat = NULL;
  }
  if (mt.pUnk != NULL)
  {
    // pUnk should not be used.
    mt.pUnk->Release();
    mt.pUnk = NULL;
  }
}

inline HRESULT CopyMediaType(AM_MEDIA_TYPE *pmtTarget, const AM_MEDIA_TYPE *pmtSource)
{
  //  We'll leak if we copy onto one that already exists - there's one
  //  case we can check like that - copying to itself.
  assert(pmtSource != pmtTarget);
  *pmtTarget = *pmtSource;
  if (pmtSource->cbFormat != 0)
  {
    assert(pmtSource->pbFormat != NULL);
    pmtTarget->pbFormat = (PBYTE)CoTaskMemAlloc(pmtSource->cbFormat);
    if (pmtTarget->pbFormat == NULL)
    {
      pmtTarget->cbFormat = 0;
      return E_OUTOFMEMORY;
    }
    else
    {
      CopyMemory((PVOID)pmtTarget->pbFormat, (PVOID)pmtSource->pbFormat, pmtTarget->cbFormat);
    }
  }
  if (pmtTarget->pUnk != NULL)
  {
    pmtTarget->pUnk->AddRef();
  }

  return S_OK;
}

#define CHECK_SUCCESS(rv) {if (FAILED(hr)) {/* error */ return rv;}}


template <typename T> class AutoRelease
{
  T * _p;
public:
  AutoRelease(T * p = NULL) : _p(p) {}
  ~AutoRelease() { if (_p) { _p->Release(); } }
  T ** operator& (void) { return &_p; }
  T *  operator->(void) { return _p; }
  T * get(void) { return _p; }
  T * reset(T * p = NULL) { return (_p=p); }
};

QMap<QString,QString> DShowCamera::_globalCameraList;

QStringList DShowCamera::getCameraList(void)
{
  QMap<QString,QString> globalCameraList;

  ICreateDevEnum *pDevEnum = NULL;
  IEnumMoniker *pEnum = NULL;
  int deviceCounter = 0;
    
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void**>(&pDevEnum));
  if (SUCCEEDED(hr))
  {
    // Create an enumerator for the video capture category.
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (hr==S_OK)
    {
      log("Enumerating capture devices\n");
      IMoniker *pMoniker = NULL;
      while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
      {
        IPropertyBag *pPropBag;
        hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
        if (FAILED(hr))
        {
          pMoniker->Release();
          continue;  // Skip this one, maybe the next one will work.
        }

        QString friendlyName, devicePath, displayName;

        { //get the friendly name
          VARIANT varName;
          VariantInit(&varName);
          hr = pPropBag->Read(L"FriendlyName", &varName, 0);
          if (SUCCEEDED(hr))
          {
            friendlyName = QString::fromWCharArray(varName.bstrVal);
          }
        }

        { //get the device path
          VARIANT varName;
          VariantInit(&varName);
          hr = pPropBag->Read(L"DevicePath", &varName, 0);
          if (SUCCEEDED(hr))
          {
            devicePath = QString::fromWCharArray(varName.bstrVal);
          }
        }

        { //use displayName instead of path
          LPOLESTR ppszDisplayName;
          hr = pMoniker->GetDisplayName(0, 0, &ppszDisplayName);
          if (SUCCEEDED(hr))
          {
            displayName = QString::fromWCharArray(ppszDisplayName);
          }
        }

        if (!friendlyName.isNull() && !displayName.isNull())
        {
            globalCameraList.insert(friendlyName, displayName);
            //TODO: add "[2]" to repeated camera names
            log("Camera found: %s\n", qPrintable(friendlyName));
        }

        pPropBag->Release();
        pPropBag = NULL;

        pMoniker->Release();
        pMoniker = NULL;

        deviceCounter++;
      }

      pDevEnum->Release();
      pDevEnum = NULL;

      pEnum->Release();
      pEnum = NULL;
    }

    log("Device(s) found: %d\n", deviceCounter);
  }

  //update global list
  _globalCameraList.swap(globalCameraList);

  return _globalCameraList.keys();
}

QSharedPointer<CameraInterface> DShowCamera::getCamera(QString cameraName)
{
  auto it = _globalCameraList.find(cameraName);
  if (it!=_globalCameraList.end())
  { //found
    std::wstring monikerDisplayName = it.value().toStdWString();
    DShowCamera * camera = new DShowCamera(it.value(), it.key());
    if (camera && camera->isValid())
    { //ok
      return QSharedPointer<CameraInterface>(camera, &QObject::deleteLater); //TMP: REVIEW
    }

    //error
    if (camera)
    {
      delete camera;
      camera = NULL;
    }
  }

  //not found
  log("[ERROR] Camera not found: %s\n", qPrintable(cameraName));
  return QSharedPointer<CameraInterface>();
}

DShowCamera::DShowCamera(QString monikerName, QString displayName) :
  _monikerName(monikerName),
  _displayName(displayName),
  _source(NULL),
  _graph(NULL),
  _format(),
  _formatList()
{
  CoInitialize(NULL);

#ifdef HAVE_IMG
  qRegisterMetaType<ImageBuffer>("ImageBuffer");
#endif //HAVE_IMG

  //get the source filter
  _source = buildFilterFromMoniker(_monikerName);

  //build the supported format list
  if (_source)
  {
    _formatList.swap(getCameraFormatList(_source));
  }

  //info
  log("[INFO] Camera created: '%s' (%d formats)\n", qPrintable(_displayName), _formatList.size());
}

IBaseFilter * DShowCamera::buildFilterFromMoniker(QString const& monikerName)
{
  HRESULT hr;

  AutoRelease<IBindCtx> pBindCtx;
  hr = CreateBindCtx(0, &pBindCtx);
  CHECK_SUCCESS(NULL);

  ULONG ulEaten;
  AutoRelease<IMoniker> pMoniker;
  hr = MkParseDisplayName(pBindCtx.get(), monikerName.toStdWString().c_str(), &ulEaten, &pMoniker);
  CHECK_SUCCESS(NULL);

  IBaseFilter *pSrcFilter = NULL;
  hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pSrcFilter);
  CHECK_SUCCESS(NULL);

  return pSrcFilter;
}

bool DShowCamera::checkSourceFilter(void) const
{
  return (_source ? true : (const_cast<DShowCamera *>(this)->_source=buildFilterFromMoniker(_monikerName))!=NULL);
}

DShowCamera::~DShowCamera()
{
  stop();

  if (_graph)
  {
    ULONG refs = _graph->Release();
    if (refs) { assert(refs==0); }
  }

  if (_source)
  {
    ULONG refs = _source->Release();
    if (refs) { assert(refs==0); }
  }

  log("[INFO] Camera destroyed: %s\n", qPrintable(_displayName));
}

bool DShowCamera::start()
{
  if (!checkSourceFilter())
  { //error 
    log("[ERROR] DShow graph source filter (%s)\n", qPrintable(_displayName));
    return false;
  }

  if (!_graph && !buildGraph(_source, _format))
  { //error
    log("[ERROR] DShow create graph failure (%s)\n", qPrintable(_displayName));
    return false;
  }

  HRESULT hr;
    
  //get the controller for the graph
  AutoRelease<IMediaControl> control;
  hr = _graph->QueryInterface(IID_IMediaControl, (void**) &control);
  CHECK_SUCCESS(false);

  hr = control->Run(); //run even when S_FALSE is returned (meaning it needs more time to finish)
  CHECK_SUCCESS(false);

  return true;
}

bool DShowCamera::stopVideo(void)
{
  HRESULT hr;

  //make sure they are deleted 
  AutoRelease<IBaseFilter> pSource(_source);
  AutoRelease<IFilterGraph2> pGraph(_graph);
  _source = NULL; _graph = NULL;
  
  if (pGraph.get())
  { 
    { //get the controller for the graph
      AutoRelease<IMediaControl> control;
      hr = pGraph->QueryInterface(IID_IMediaControl, (void**) &control);
      CHECK_SUCCESS(false);

      //stop streaming and wait
      hr = control->StopWhenReady();
      CHECK_SUCCESS(false);
    }

    //delete the graph here to make sure we have the reference counting correct
    ULONG refs = pGraph->Release(); pGraph.reset();
    if (refs) { assert(refs==0); return false;}
  }

  if (pSource.get())
  { //delete the _source here to make sure we have the reference counting correct
    ULONG refs = pSource->Release(); pSource.reset();
    if (refs) { assert(refs==0); return false;}
  }

  return true;
}

bool DShowCamera::buildGraph(IBaseFilter * sourceFilter, QString requestedFormat)
{
  HRESULT hr;
  AutoRelease<IBaseFilter> grabberFilter, nullRenderer;

  //create the FilterGraph
  hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,IID_IFilterGraph2,(void**) &_graph);
  CHECK_SUCCESS(false);

  //create filters
  if (_graph)
  {
    //create sample grabber
    hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&grabberFilter);
    CHECK_SUCCESS(false);

    //create a null renderer
    hr = CoCreateInstance(CLSID_NullRenderer,NULL,CLSCTX_INPROC_SERVER,IID_IBaseFilter,(void**)&nullRenderer);
    CHECK_SUCCESS(false);
  }

  //add filters to graph
  if (_graph && sourceFilter && grabberFilter.get() && nullRenderer.get())
  {
    //add source filter
    hr = _graph->AddFilter(sourceFilter,  L"Source Filter");
    CHECK_SUCCESS(false);

    //add sample grabber filter
    hr = _graph->AddFilter(grabberFilter.get(), L"Sample Grabber");
    CHECK_SUCCESS(false);

    //add null render to graph
    hr = _graph->AddFilter(nullRenderer.get(), L"Null Renderer");
    CHECK_SUCCESS(false);
  }

  //connect filters
  if (_graph)
  {
    //create a CaptureGraphBuilder
    AutoRelease<ICaptureGraphBuilder2> builder;
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,IID_ICaptureGraphBuilder2,(void**) &builder);
    CHECK_SUCCESS(false);

    //set current graph
    hr = builder->SetFiltergraph(_graph);
    CHECK_SUCCESS(false);

    //search a preferred resolution
    AutoRelease<IPin> pPin;
    AM_MEDIA_TYPE preferredMediaType;
    if (getPreferredSource(sourceFilter, requestedFormat, preferredMediaType, &pPin))
    { //use preferred pin
    
      //get pin stream config interface
      AutoRelease<IAMStreamConfig> stream;
      hr = pPin->QueryInterface(IID_PPV_ARGS(&stream));
      CHECK_SUCCESS(false);

      //configure pin media type
      hr = stream->SetFormat(&preferredMediaType);
      CHECK_SUCCESS(false);

      //release media type
      FreeMediaType(preferredMediaType);

      //connect everything
      hr = builder->RenderStream(&PIN_CATEGORY_CAPTURE, NULL, pPin.get(), grabberFilter.get(), nullRenderer.get());
      CHECK_SUCCESS(false);
    }
    else
    { //try default
      hr = builder->RenderStream(&PIN_CATEGORY_CAPTURE, NULL, sourceFilter, grabberFilter.get(), nullRenderer.get());
      CHECK_SUCCESS(false);
    }
  }

  //setup sample grabber
  if (grabberFilter.get())
  {
    //get the sample grabber interface
    AutoRelease<ISampleGrabber> sampleGrabber;
    hr = grabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&sampleGrabber);
    CHECK_SUCCESS(false);
 
    //set the media type
    AM_MEDIA_TYPE mt;
    memset(&mt, 0, sizeof(AM_MEDIA_TYPE));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype   = MEDIASUBTYPE_RGB24;
    hr = sampleGrabber->SetMediaType(&mt);
    CHECK_SUCCESS(false);

    //set the callback
    DShowCameraCallback * callback = new DShowCameraCallback();
    hr = sampleGrabber->SetCallback(callback, 0);
    CHECK_SUCCESS(false);

    //get the current camera media type
    AM_MEDIA_TYPE sourceMediaType;
    if (getConnectedPinMediaType(sourceFilter, sourceMediaType))
    {
      callback->SetMediaType(sourceMediaType);
      FreeMediaType(sourceMediaType);
    }

    //connect callback signal
#ifdef HAVE_IMG
    connect(callback, &DShowCameraCallback::newImage, this, &DShowCamera::newImage, Qt::DirectConnection);
#endif //HAVE_IMG
  }

  return true;
}

QStringList DShowCamera::getCameraFormatList(IBaseFilter *pFilter)
{
  HRESULT hr;
  bool debug = true;

  //create pin enumerator
  AutoRelease<IEnumPins> pPinEnum;
  hr = pFilter->EnumPins(&pPinEnum);
  if (!SUCCEEDED(hr))
  { //error
    return QStringList();
  }

  //format map
  QMap<QString,bool> fmtMap;

  //enumerate pins
  IPin *pPin = NULL;
  while (pPinEnum->Next(1, &pPin, 0)==S_OK)
  {
    AutoRelease<IPin> currPin(pPin);

    //query pin direction
    PIN_DIRECTION PinDirThis;
    hr = pPin->QueryDirection(&PinDirThis);
    if (!SUCCEEDED(hr) || PinDirThis!=PINDIR_OUTPUT)
    {   //skip
      continue;
    }

    if (debug)
    { //print pin name
      PIN_INFO pi;
      hr = pPin->QueryPinInfo(&pi);
      AutoRelease<IBaseFilter> piFilter(pi.pFilter);
      if (SUCCEEDED(hr))
      {
        //TCHAR str[MAX_PIN_NAME];
        //StringCchCopy(str, NUMELMS(str), pi.achName);
        log("Pin name %S\n", pi.achName);
      }
    }

    //prepare for querying pin category
    AutoRelease<IKsPropertySet> pKs;
    hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
    if (FAILED(hr)) 
    { //error
      continue; 
    }

    //query pin category.
    GUID pinCategory;
    DWORD cbReturned = 0;
    hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pinCategory, sizeof(GUID), &cbReturned);
    if (!SUCCEEDED(hr))
    { //error
      continue; 
    }
    if (debug)
    {
      log(" Pin category %s\n", (pinCategory==PIN_CATEGORY_STILL ? "STILL" : (pinCategory==PIN_CATEGORY_CAPTURE ? "CAPTURE" : "OTHER")));
    }

    if (pinCategory!=PIN_CATEGORY_CAPTURE)
    { //skip
      continue;
    }

    //output pin found: enumerate media types
    AutoRelease<IEnumMediaTypes> mediaTypesEnumerator;
    hr = pPin->EnumMediaTypes(&mediaTypesEnumerator);
    if (FAILED(hr)) 
    { //error
      continue; 
    }

    //enumerate media types
    AM_MEDIA_TYPE* mediaType = NULL;  
    while (mediaTypesEnumerator->Next(1, &mediaType, NULL)==S_OK)
    {
      if (mediaType)
      {
        auto format = getFormatType(mediaType);
        if (format!=getFormatType(NULL))
        {
          fmtMap[format] = true;
        }
        FreeMediaType(*mediaType);
      }
    }//while (mediaTypesEnumerator->Next)
  }//while (pPinEnum->Next)

  if (debug)
  {
    foreach (auto res, fmtMap.keys()) { log("res %s\n", qPrintable(res)); }
  }

  return QStringList(fmtMap.keys());
}

QString DShowCamera::getFormatType(AM_MEDIA_TYPE * mediaType)
{
#ifdef HAVE_IMG
  QSize size;
  auto imgFmt = getImageBufferFormatType(mediaType, &size);

  if (imgFmt!=ImageBuffer::UnknownFormat)
  { //add to the list
    return QString("%1:%2x%3").arg(ImageBuffer().formatName(imgFmt)).arg(size.width()).arg(size.height());
  }
#endif //HAVE_IMG  
  return QString("UNKN:0x0");
}

#ifdef HAVE_IMG
ImageBuffer::FormatType DShowCamera::getImageBufferFormatType(AM_MEDIA_TYPE * mediaType, QSize * size)
{
  const bool debug = true;
  auto imgFmt = ImageBuffer::UnknownFormat;
 
  if ((mediaType != NULL) &&
      (mediaType->formattype == FORMAT_VideoInfo) &&
      (mediaType->cbFormat >= sizeof(VIDEOINFOHEADER)) &&
      (mediaType->pbFormat != NULL))
  {
    VIDEOINFOHEADER* videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER *>(mediaType->pbFormat);

    int rows = videoInfoHeader->bmiHeader.biHeight;
    int cols = videoInfoHeader->bmiHeader.biWidth;

    if (size)
    {
      size->setWidth(cols);
      size->setHeight(rows);
    }

    { //identify the image format
      char code[5] = {0, 0, 0, 0, 0};
      *reinterpret_cast<LONG*>(code) = videoInfoHeader->bmiHeader.biCompression;
      QString fmtStr(videoInfoHeader->bmiHeader.biCompression==BI_RGB? "RGB" :
                        (videoInfoHeader->bmiHeader.biCompression==BI_BITFIELDS ? "BITFIELDS" : code));

      if (debug)
      {
        log(" format string '%s:%dx%d'\n", qPrintable(fmtStr), cols, rows);
      }
          
      if (fmtStr=="RGB")       { imgFmt = ImageBuffer::RGB24;  }
      else if (fmtStr=="IYUV") { imgFmt = ImageBuffer::YUV422; }
      else if (fmtStr=="I420") { imgFmt = ImageBuffer::YUV422; }
      else if (fmtStr=="YUY2") { imgFmt = ImageBuffer::YUV422; }
      else if (fmtStr=="MJPG") { imgFmt = ImageBuffer::JPEG; }
    }
  }

  return imgFmt;
}
#endif //HAVE_IMG  

bool DShowCamera::getConnectedPinMediaType(IBaseFilter *pFilter, _AMMediaType &mt)
{
  HRESULT hr;
  AutoRelease<IEnumPins> pEnum;
  hr = pFilter->EnumPins(&pEnum);
  CHECK_SUCCESS(false);

  IPin * pPin = NULL;
  while (pEnum->Next(1, &pPin, 0)==S_OK)
  {
    AutoRelease<IPin> currPin(pPin);
    hr = pPin->ConnectionMediaType(&mt);
    if (hr==S_OK) 
    { //found a connected pin
      return true;
    }
  } //while

  return false;
}

bool DShowCamera::getPreferredSource(IBaseFilter *pFilter, QString requestedFormat, 
                              /*out*/ _AMMediaType &preferredMediaType, /*out*/ IPin **preferredPin)
{
  HRESULT hr;
  bool debug = true;

  const QString UnkownFormat = getFormatType(NULL);
  if (requestedFormat==UnkownFormat) { requestedFormat.clear(); }

  //create pin enumerator
  AutoRelease<IEnumPins> pPinEnum;
  hr = pFilter->EnumPins(&pPinEnum);
  CHECK_SUCCESS(false);

  //selected resolution pixel count
  size_t pixelCount = 0;

  //enumerate pins
  IPin *outPin = NULL;
  IPin *pPin = NULL;
  while (pPinEnum->Next(1, &pPin, 0)==S_OK)
  {
    AutoRelease<IPin> currPin(pPin);

    //query pin direction
    PIN_DIRECTION PinDirThis;
    hr = pPin->QueryDirection(&PinDirThis);
    if (!SUCCEEDED(hr) || PinDirThis!=PINDIR_OUTPUT)
    {   //skip
      continue;
    }

    if (debug)
    { //print pin name
      PIN_INFO pi;
      hr = pPin->QueryPinInfo(&pi);
      AutoRelease<IBaseFilter> piFilter(pi.pFilter);
      if (SUCCEEDED(hr))
      {
        //TCHAR str[MAX_PIN_NAME];
        //(str, NUMELMS(str), pi.achName);
        log("Pin name %S\n", pi.achName);
      }
    }

    //prepare for querying pin category
    AutoRelease<IKsPropertySet> pKs;
    hr = pPin->QueryInterface(IID_PPV_ARGS(&pKs));
    if (FAILED(hr)) 
    { //error
      continue; 
    }

    //query pin category.
    GUID pinCategory;
    DWORD cbReturned = 0;
    hr = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0, &pinCategory, sizeof(GUID), &cbReturned);
    if (!SUCCEEDED(hr))
    { //error
      continue; 
    }
    if (debug)
    {
      log(" Pin category %s\n", (pinCategory==PIN_CATEGORY_STILL ? "STILL" : (pinCategory==PIN_CATEGORY_CAPTURE ? "CAPTURE" : "OTHER")));
    }

    if (pinCategory!=PIN_CATEGORY_CAPTURE)
    { //skip
      continue;
    }

    //output pin found: enumerate media types
    AutoRelease<IEnumMediaTypes> mediaTypesEnumerator;
    hr = pPin->EnumMediaTypes(&mediaTypesEnumerator);
    if (FAILED(hr)) 
    { //error
      continue; 
    }

    //enumerate media types
    AM_MEDIA_TYPE* mediaType = NULL;  
    while (mediaTypesEnumerator->Next(1, &mediaType, NULL)==S_OK)
    {
      size_t count;
      auto format = getFormatType(mediaType);
      if (format==UnkownFormat)
      { //skip
        FreeMediaType(*mediaType);
        continue;
      }
      if (requestedFormat.isEmpty())
      { //maximize the number of pixels
        VIDEOINFOHEADER * videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER *>(mediaType->pbFormat);
        count = videoInfoHeader->bmiHeader.biWidth*videoInfoHeader->bmiHeader.biHeight;
      }
      else
      { //compared to requested format
        count = (format==requestedFormat ? 1+pixelCount : 0);
      }

      if (count>pixelCount)
      { //set resolution
        CopyMediaType(&preferredMediaType, mediaType);
        outPin = pPin;
        pixelCount = count;
      }

      FreeMediaType(*mediaType);
    }//while (mediaTypesEnumerator->Next)
  }//while (pPinEnum->Next)

  if (pixelCount)
  { //found
    if (preferredPin && outPin)
    { //return the pin (caller must free!)
      *preferredPin = outPin;
      (*preferredPin)->AddRef();
    }
    return true;
  }

  //not found
  return false;
}

QStringList DShowCamera::getPropertyList(void)
{
  return QStringList() << CAM_PROP_FORMAT << CAM_PROP_EXPOSURE << CAM_PROP_CONTRAST << CAM_PROP_BRIGHTNESS;
}

QVariant DShowCamera::getCameraProperty(QString propertyName)
{
  if (propertyName==CAM_PROP_FORMAT)
  {
    return QVariant(getFormat());
  }
  if (propertyName==CAM_PROP_EXPOSURE)
  {
    return QVariant(getExposure());
  }
  if (propertyName==CAM_PROP_CONTRAST)
  {
    return QVariant(getContrast());
  }
  if (propertyName==CAM_PROP_BRIGHTNESS)
  {
    return QVariant(getBrightness());
  }

  log("[WARN] Unknow property: %s\n", qPrintable(propertyName));
  return QVariant();
}

void DShowCamera::setCameraProperty(QString propertyName, QVariant value)
{
  if (propertyName==CAM_PROP_FORMAT)
  {
    setFormat(value.toString());
  }
  else if (propertyName==CAM_PROP_EXPOSURE)
  {
    setExposure(value.toDouble());
  }
  else if (propertyName==CAM_PROP_CONTRAST)
  {
    setContrast(value.toDouble());
  }
  else if (propertyName==CAM_PROP_BRIGHTNESS)
  {
    setBrightness(value.toDouble());
  }
  else
  {
    log("[WARN] Unknow property: %s\n", qPrintable(propertyName));
  }
}

double DShowCamera::getExposure(void) const
{
  if (!checkSourceFilter()) { return 0.0; }

  HRESULT hr;

  { //try Logitech
    AutoRelease<IKsPropertySet> propertySet;
    hr = _source->QueryInterface(IID_IKsPropertySet, (void**)&propertySet);
    CHECK_SUCCESS(0.0);

    //query value
    DWORD dwReturned = 0;
    KSPROPERTY_LP1_EXPOSURE_TIME_S exposureTime = { {0U, 0U, 0U} , 0U};
    hr = propertySet->Get(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, NULL, 0, &exposureTime, sizeof(exposureTime), &dwReturned);
    if (hr!=E_PROP_SET_UNSUPPORTED)
    { //property set supported
      CHECK_SUCCESS(0.0);

      //convert to seconds
      log("logitech getExposure: %lf\n", 0.0001*static_cast<double>(exposureTime.ulExposureTime));
      return 0.0001*static_cast<double>(exposureTime.ulExposureTime);
    }
  } //logitech

  //dshow---------------------------

  //query the capture filter for the cameraControl interface
  AutoRelease<IAMCameraControl> control;
  hr = _source->QueryInterface(IID_IAMCameraControl, (void**)&control);
  CHECK_SUCCESS(0.0);

  //get value
  long value, flags;
  hr = control->Get(CameraControl_Exposure, &value, &flags);
  CHECK_SUCCESS(0.0);

  //convert to seconds
  log("dshow getExposure: %lf\n", std::exp(static_cast<double>(value)*std::log(2.0)));
  return std::exp(static_cast<double>(value)*std::log(2.0));
}

void DShowCamera::setExposure(double value)
{
  if (!checkSourceFilter()) { return; }

  HRESULT hr;

  { //try Logitech
    AutoRelease<IKsPropertySet> propertySet;
    hr = _source->QueryInterface(IID_IKsPropertySet, (void**)&propertySet);
    if (hr!=S_OK) { return; }

    //query value
    KSPROPERTY_LP1_EXPOSURE_TIME_S exposureTime = { {CameraControl_Flags_Manual, 0U, 0U} , static_cast<ULONG>(10000.0*value)};
    hr = propertySet->Set(PROPSETID_LOGITECH_PUBLIC1, KSPROPERTY_LP1_EXPOSURE_TIME, NULL, 0, &exposureTime, sizeof(exposureTime));
    if (hr!=E_PROP_SET_UNSUPPORTED)
    { //property set supported
      if (hr!=S_OK) { return; }
      
      //succeeded
      log("logitech setExposure: %lf\n", value);
      return; 
    }
  } //logitech

  //dshow---------------------------

  //query the capture filter for the cameraControl interface
  AutoRelease<IAMCameraControl> control;
  hr = _source->QueryInterface(IID_IAMCameraControl, (void**)&control);
  if (hr!=S_OK) { return; }

  // value = ln(exposure)/ln(2) => exposure = exp(value*ln(2))

  //set value
  long valueLong = static_cast<long>(std::log(value)/std::log(2.0));
  long flags = CameraControl_Flags_Manual;
  hr = control->Set(CameraControl_Exposure, valueLong, flags);
  if (hr!=S_OK) { return; }
  log("dshow setExposure: %lf\n", value);
}

double DShowCamera::getContrast(void) const
{
  if (!checkSourceFilter()) { return 0.0; }

  HRESULT hr;

  //query the capture filter for the cameraControl interface
  AutoRelease<IAMVideoProcAmp> control;
  hr = _source->QueryInterface(IID_IAMVideoProcAmp, (void**)&control);
  CHECK_SUCCESS(0.0);

  //get the range and default value
  long Min, Max, Step, Default, Flags;
  hr = control->GetRange(VideoProcAmp_Contrast, &Min, &Max, &Step, &Default, &Flags);
  CHECK_SUCCESS(0.0);

  if (Min==0 && Max==255) { Max = 256; } //use 256 so 128 <--> 0.5

  //get value
  long value, flags;
  hr = control->Get(VideoProcAmp_Contrast, &value, &flags);
  CHECK_SUCCESS(0.0);

  if (flags==VideoProcAmp_Flags_Manual) { log("Contrast MANUAL\n"); }
  log("Contrast %d\n", value);

  //scale to 0.0 - 1.0
  return static_cast<double>(value+Min)/static_cast<double>(Max);
}

void DShowCamera::setContrast(double value)
{
  if (!checkSourceFilter()) { return; }
  
  HRESULT hr;

  //query the capture filter for the control interface
  AutoRelease<IAMVideoProcAmp> control;
  hr = _source->QueryInterface(IID_IAMVideoProcAmp, (void**)&control);
  if (hr!=S_OK) { return; }

  //get the range and default value
  long Min, Max, Step, Default, Flags;
  hr = control->GetRange(VideoProcAmp_Contrast, &Min, &Max, &Step, &Default, &Flags);
  if (hr!=S_OK) { return; }

  long adjustedMax = Max;
  if (Min==0 && adjustedMax==255) { adjustedMax = 256; } //use 256 so 128 <--> 0.5

  //set value
  long valueLong = std::min(Max, Min + static_cast<long>(value*static_cast<double>(adjustedMax-Min)));
  long flags = VideoProcAmp_Flags_Manual;
  hr = control->Set(VideoProcAmp_Contrast, valueLong, flags);
  if (hr!=S_OK) { return; }
}

double DShowCamera::getBrightness(void) const
{
  if (!checkSourceFilter()) { return 0.0; }
 
  HRESULT hr;

  //query the capture filter for the control interface
  AutoRelease<IAMVideoProcAmp> control;
  hr = _source->QueryInterface(IID_IAMVideoProcAmp, (void**)&control);
  CHECK_SUCCESS(0.0);

  //get the range and default value
  long Min, Max, Step, Default, Flags;
  hr = control->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step, &Default, &Flags);
  CHECK_SUCCESS(0.0);

  if (Min==0 && Max==255) { Max = 256; } //use 256 so 128 <--> 0.5

  //get value
  long value, flags;
  hr = control->Get(VideoProcAmp_Brightness, &value, &flags);
  CHECK_SUCCESS(0.0);

  if (flags==VideoProcAmp_Flags_Manual) { log("Brightness MANUAL\n"); }
  log("Brightness %d\n", value);

  //scale to 0.0 - 1.0
  return static_cast<double>(value+Min)/static_cast<double>(Max);
}

void DShowCamera::setBrightness(double value)
{ //accepts values between 0.0 and 1.0
  if (!checkSourceFilter()) { return; }

  HRESULT hr;

  //query the capture filter for the control interface
  AutoRelease<IAMVideoProcAmp> control;
  hr = _source->QueryInterface(IID_IAMVideoProcAmp, (void**)&control);
  if (hr!=S_OK) { return; }

  //get the range and default value
  long Min, Max, Step, Default, Flags;
  hr = control->GetRange(VideoProcAmp_Brightness, &Min, &Max, &Step, &Default, &Flags);
  if (hr!=S_OK) { return; }

  long adjustedMax = Max;
  if (Min==0 && adjustedMax==255) { adjustedMax = 256; } //use 256 so 128 <--> 0.5

  //set value
  long valueLong = std::min(Max, Min + static_cast<long>(value*static_cast<double>(adjustedMax-Min)));
  long flags = VideoProcAmp_Flags_Manual;
  hr = control->Set(VideoProcAmp_Brightness, valueLong, flags);
  if (hr!=S_OK) { return; }
}

QString DShowCamera::getFormat(void) const
{
  AM_MEDIA_TYPE preferredMediaType;
  if (_source && _graph && getConnectedPinMediaType(_source, preferredMediaType))
  { //there is graph: return the currently set format
    return getFormatType(&preferredMediaType);
  }
  if (checkSourceFilter() && getPreferredSource(_source, _format, preferredMediaType))
  {
    return getFormatType(&preferredMediaType);
  }
  return getFormatType(NULL);
}

void DShowCamera::setFormat(QString format)
{
  _format = format;
}

#undef CHECK_SUCCESS


//------------------------------------------------------------------------------------------
// DShowCameraCallback
//------------------------------------------------------------------------------------------

#include <QTime>

DShowCameraCallback::DShowCameraCallback() :
  QObject(NULL),
  ISampleGrabberCB(),
#ifdef HAVE_IMG
  _buffer(),
#endif //HAVE_IMG
  _refCount(0)
{
  log("Created: ref=%d\n", _refCount);
}

DShowCameraCallback::~DShowCameraCallback()
{
  log("Deleted: ref=%d\n", _refCount);
}
 
void DShowCameraCallback::SetMediaType(AM_MEDIA_TYPE &mt)
{
  //reset
#ifdef HAVE_IMG
  _buffer.init();
#endif //HAVE_IMG

  //update
#ifdef _OLD
  if (mt.formattype==FORMAT_VideoInfo )
  {
    const VIDEOINFOHEADER * videoInfoHeader = reinterpret_cast<VIDEOINFOHEADER*>(mt.pbFormat);
    const BITMAPINFOHEADER & bmiHeader = videoInfoHeader->bmiHeader;

    {
      char code[5] = {0, 0, 0, 0, 0};
      *reinterpret_cast<LONG*>(code) = videoInfoHeader->bmiHeader.biCompression;
      if (!silent)
      {
          log("SetMediaType: w %d h %d type %s\n", 
                  videoInfoHeader->bmiHeader.biWidth,  // Supported width
                  videoInfoHeader->bmiHeader.biHeight, // Supported height
                  (videoInfoHeader->bmiHeader.biCompression==BI_RGB? "RGB" :
                      (videoInfoHeader->bmiHeader.biCompression==BI_BITFIELDS ? "BITFIELDS" : code)
                  )
                );
      }//if !silent
    }
    
    //assert(videoInfoHeader->bmiHeader.biCompression==BI_RGB);

    //save media type
    _buffer.init(bmiHeader.biHeight, bmiHeader.biWidth, ImageBuffer::BGR24);
    _rowStride = ((((bmiHeader.biWidth * bmiHeader.biBitCount) + 31) & ~31) >> 3);
  }
#endif //_OLD

#ifdef HAVE_IMG
  QSize size;
  auto imgFmt = DShowCamera::getImageBufferFormatType(&mt, &size);
  _buffer.init(size.height(), size.width(), imgFmt);
#endif //HAVE_IMG

}
 
HRESULT DShowCameraCallback::SampleCB(double SampleTime, IMediaSample *pSample)
{
  //fprintf(stdout, "."); fflush(stdout);

  if (pSample)
  {
    //query current sample media type
    AM_MEDIA_TYPE *pMediaType = NULL;
    if (pSample->GetMediaType(&pMediaType)==S_OK)
    { //media type has changed
      SetMediaType(*pMediaType);
      FreeMediaType(*pMediaType);
      pMediaType = NULL;
    }

    //get sample buffer
    unsigned char * buffer = NULL;
    if (pSample->GetPointer(&buffer)==S_OK)
    {

/*
      if (_image.isValid())
      {
        //make an image header 
        ImageType image(_image.rows(), _image.cols(), buffer, _rowStride); 

        //mirror and convert to BGR
        for (int h1=0, h2=image.rows()-1; h1<image.rows(); ++h1, --h2)
        {
          ImageType::Vec3b * row1 = _image.rowPtr<ImageType::Vec3b>(h1);
          const ImageType::Vec3b * row2 = image.rowPtr<ImageType::Vec3b>(h2);
          for (int w=0; w<image.cols(); ++w)
          {
            auto const& px = row2[w];
            row1[w] = ImageType::Vec3b(px[2],px[1],px[0]);
          }
        }

        //notify
        emit newImage(_image);
      }
*/

      log("camera image %s bytes %d\n", qPrintable(QTime::currentTime().toString("ss.zzz")), pSample->GetActualDataLength());

#ifdef HAVE_IMG
      _buffer.data = buffer;
      _buffer.bytes = pSample->GetActualDataLength();

      emit newImage(_buffer);

      _buffer.data = NULL;
      _buffer.bytes = 0;
#endif //HAVE_IMG

    }
  }

  return S_OK;
}

HRESULT DShowCameraCallback::BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen)
{ //Not implemented: do nothing
  return S_OK;
}

HRESULT DShowCameraCallback::QueryInterface(REFIID riid, LPVOID *ppvObj)
{
  // Always set out parameter to NULL, validating it first.
  if (!ppvObj)
  {
        return E_INVALIDARG;
  }
  *ppvObj = NULL;
  if (riid==IID_IUnknown || riid==IID_ISampleGrabberCB)
  { // Increment the reference count and return the pointer.
    *ppvObj = (LPVOID)this;
    AddRef();
    return NOERROR;
  }
  return E_NOINTERFACE;
}

ULONG DShowCameraCallback::AddRef()
{
  InterlockedIncrement(&_refCount);
  return _refCount;
}

ULONG DShowCameraCallback::Release()
{
  // Decrement the object's internal counter.
  ULONG ulRefCount = InterlockedDecrement(&_refCount);
  if (_refCount==0)
  {
    delete this;
  }
  return ulRefCount;
}
