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

#include "V4L2Camera.hpp"

#include <QDir>
#include <QVariant>
#include <QMetaType>
#include <QStringList>
#include <QSharedPointer>
#include <QCoreApplication>

#include <linux/videodev2.h>

//#include "LVUVCPublic.h" //logitech public properties

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>  /* low-level i/o */
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "ImageBuffer.hpp"

static int xioctl(int fh, int request, void *arg)
{
  int r = -1;
  do { r = ioctl(fh, request, arg); } while (-1 == r && EINTR == errno);
  return r;
}

QMap<QString,QString> V4L2Camera::_globalCameraList;

QStringList V4L2Camera::getCameraList(void)
{
  QMap<QString,QString> globalCameraList;

  log("getCameraList: start\n");

  //enumerate video devices
  QDir devDir("/sys/class/video4linux");
  QStringList list = devDir.entryList(QDir::Dirs|QDir::NoDotAndDotDot);
  foreach (auto entry, list)
  {

    //read camera name
    QString name = entry;
    QString namePath = devDir.absoluteFilePath(entry) + "/name";
    FILE * fp = fopen(qPrintable(namePath), "r");
    if (fp)
    {
      char * line = NULL;
      size_t len = 0;
      if (getline(&line, &len, fp)>0)
      {
        name = QString(line).trimmed();
      }
      if (line) { free(line); }
      fclose(fp);
    }
    
    log("entry: %s %s\n", qPrintable(entry), qPrintable(name));
    
    globalCameraList.insert(name, entry);
  }
 
  //update global list
  _globalCameraList.swap(globalCameraList);

  return _globalCameraList.keys();
}

QSharedPointer<CameraInterface> V4L2Camera::getCamera(QString cameraName)
{
  auto it = _globalCameraList.find(cameraName);
  if (it!=_globalCameraList.end())
  { //found
    int cam = openDevice(it.value());
    if (cam>=0)
    { //ok
      V4L2Camera * camera = new V4L2Camera(cam, it.key());
      return QSharedPointer<CameraInterface>(camera);
    }
  }

  //not found
  log("[ERROR] Camera not found: %s\n", qPrintable(cameraName));
  return QSharedPointer<CameraInterface>();
}

V4L2Camera::V4L2Camera(int cam, QString name) :
  _cam(cam),
  _name(name),
  _thread(this),
  _formatList()
{
  qRegisterMetaType<ImageBuffer>("ImageBuffer");

  if (_cam>=0)
  {
    _formatList = listCameraFormats();
    findControls();
    disableAuto();
  }

  //info
  log("[INFO] Camera created: %s\n", qPrintable(_name));
}

V4L2Camera::~V4L2Camera()
{
  stop();

  if (_cam>=0)
  { //close descriptor
    ::close(_cam);
    _cam = -1;
  }
  
  log("[INFO] Camera destroyed: %s\n", qPrintable(_name));
}

bool V4L2Camera::start()
{
  if (_thread.isRunning())
  { //error
    log("[DEBUG] _thread.isRunning = true\n");
    return false;
  }
  
  /*if (!setStreamFormat())
  {
    return false;
  }*/

  //run worker on a thread
  auto worker = new V4L2CameraWorker(_cam);
  worker->moveToThread(&_thread);
  connect(&_thread, &QThread::started, worker, &V4L2CameraWorker::start);
  connect(&_thread, &QThread::finished, worker, &QObject::deleteLater);
  connect(worker, &V4L2CameraWorker::newImage, this, &V4L2Camera::newImage, Qt::DirectConnection);
  connect(this, &V4L2Camera::stopWorker, worker, &V4L2CameraWorker::stop, Qt::DirectConnection);
  _thread.start();

  //wait for worker initialization
  while (_thread.isRunning() && !worker->isStreaming())
  {
    QCoreApplication::processEvents();
  }
  if (_thread.isRunning() && worker->isStreaming())
  { //ok
    return true;
  }


  //failed
  log("[ERROR] Start %s\n", qPrintable(_name));
  return false;
}

bool V4L2Camera::stopVideo(void)
{
  log("stopVideo: start\n");
  if (_thread.isRunning())
  {
    emit stopWorker();
    if (QThread::currentThread()!=&_thread)
    {
      log("[INFO] waiting for thread to finish\n");
      while (_thread.isRunning()) { QCoreApplication::processEvents(); }
    }
  }
  log("stopVideo: end\n");
  return true;
}

int V4L2Camera::openDevice(QString const& device)
{
  QString devPath = "/dev/" + device;

  //check if is a character device
  struct stat st;
  if (stat(qPrintable(devPath), &st)<0) 
  {
    log("[ERROR] start: %s\n", strerror(errno));
    return -1;
  }
  if (!S_ISCHR(st.st_mode))
  { /* is character special */
    log("[ERROR] start: device is not character special\n");
    return -1;
  }

  //open device
  int cam = open(qPrintable(devPath), O_RDWR /* required */ | O_NONBLOCK, 0);
  if (cam<0)
  {
    log("[ERROR] cannot open stream descriptor (%s)\n", strerror(errno));
  }

  return cam;
}

QStringList V4L2Camera::getPropertyList(void)
{
  QStringList controlList;

  //add standard controls
  controlList << CAM_PROP_FORMAT 
              << CAM_PROP_EXPOSURE 
              << CAM_PROP_CONTRAST 
              << CAM_PROP_BRIGHTNESS;

  //add extra controls
  auto list = _controlMap.keys();
  foreach(auto ctrl, list)
  {
    if (!controlList.contains(ctrl))
    {
      auto it = _controlMenu.constFind(ctrl);
      if (it!=_controlMenu.constEnd())
      {
        controlList << ctrl + " " + it->join(',');
      }
      else
      {
        controlList << ctrl;
      }
    }
  }

  return controlList;
}

QVariant V4L2Camera::getCameraProperty(QString propertyName)
{
  //standard controls
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

  //extra controls
  auto it = _controlMap.constFind(propertyName);
  if (it!=_controlMap.constEnd())
  {
    //log("[DEBUG] control %s\n", qPrintable(it.key()));
    QVariant value;
    if (getPropertyValue(_cam, it.value(), value))
    {
      return value;
    }
  }

  log("[WARN] Unknown property (get): %s\n", qPrintable(propertyName));
  return QVariant();
}

void V4L2Camera::setCameraProperty(QString propertyName, QVariant value)
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
    auto it = _controlMap.constFind(propertyName);
    if (it!=_controlMap.constEnd())
    { //extra controls
      setPropertyValue(_cam, it.value(), value);
    }
    else
    { //property not found
      log("[WARN] Unknown property (set): %s\n", qPrintable(propertyName));
    }
  }
}

double V4L2Camera::getExposure(void) const
{
  bool rv = false;
  QVariant value;
  auto valueList = _controlMap.values();
  if (valueList.indexOf(V4L2_CID_EXPOSURE_ABSOLUTE)>=0)
  {
    rv = getPropertyValue(_cam, V4L2_CID_EXPOSURE_ABSOLUTE, value);
  }
  else if (valueList.indexOf(V4L2_CID_EXPOSURE)>=0)
  {
    rv = getPropertyValue(_cam, V4L2_CID_EXPOSURE, value);
  }
  else
  {
    log("[WARN] exposure control not found.\n");
  }

  if (rv)
  {
    return value.toDouble()/10000.0;  // V4L2_CID_EXPOSURE_ABSOLUTE uses 100us intervals
                                      // V4L2_CID_EXPOSURE has no defined units
  }

  /*
  auto it = _controlMap.constFind("exposure");
  if (it!=_controlMap.constEnd() 
      || (it=_controlMap.constFind("exposure_absolute"))!=_controlMap.constEnd()
      || (it=_controlMap.constFind("exposure_time_absolute"))!=_controlMap.constEnd())
  {
    log("[DEBUG] getExposure: %s %d [ABSOLUTE %d, EXPOSURE %d]\n", qPrintable(it.key()), it.value(), V4L2_CID_EXPOSURE_ABSOLUTE, V4L2_CID_EXPOSURE);
    QVariant value;
    if (getPropertyValue(_cam, it.value(), value))
    {
      return value.toDouble()/10000.0;  // V4L2_CID_EXPOSURE_ABSOLUTE uses 100us intervals
                                        // V4L2_CID_EXPOSURE has no defined units
    }
  }
  */

  return 0.0;
}

void V4L2Camera::setExposure(double value)
{
  size_t valueInt = static_cast<size_t>(value*10000.0);
                                        // V4L2_CID_EXPOSURE_ABSOLUTE uses 100us intervals
                                        // V4L2_CID_EXPOSURE has no defined units
  auto valueList = _controlMap.values();
  if (valueList.indexOf(V4L2_CID_EXPOSURE_ABSOLUTE)>=0)
  {
    setPropertyValue(_cam, V4L2_CID_EXPOSURE_ABSOLUTE, QVariant::fromValue(valueInt));
  }
  else if (valueList.indexOf(V4L2_CID_EXPOSURE)>=0)
  {
    setPropertyValue(_cam, V4L2_CID_EXPOSURE, QVariant::fromValue(valueInt));
  }
  /*
  auto it = _controlMap.constFind("exposure");
  if (it!=_controlMap.constEnd() 
      || (it=_controlMap.constFind("exposure_absolute"))!=_controlMap.constEnd())
  {
    size_t valueInt = static_cast<size_t>(value*10000.0);
                                        // V4L2_CID_EXPOSURE_ABSOLUTE uses 100us intervals
                                        // V4L2_CID_EXPOSURE has no defined units
    setPropertyValue(_cam, it.value(), QVariant::fromValue(valueInt));
  }
  */
}

double V4L2Camera::getContrast(void) const
{
  QVariant value;
  int Min, Max;
  if (getPropertyValue(_cam, V4L2_CID_CONTRAST, value, &Min, &Max))
  { //scale to 0.0 - 1.0
    double valueDouble = static_cast<double>(value.toInt()+Min)/static_cast<double>(Max);
    if (Min==0 && Max==255)
    { //hack fix: round to 2 digits
      valueDouble = std::round(100.0*valueDouble)/100.0;
    }
    return valueDouble;
  }
  return 0.0;
}

void V4L2Camera::setContrast(double value)
{
  QVariant curVal;
  int Min, Max;
  if (getPropertyValue(_cam, V4L2_CID_CONTRAST, curVal, &Min, &Max))
  { //descale from 0.0 - 1.0
    int valueInt = static_cast<int>(value*static_cast<double>(Max)-static_cast<double>(Min));
    setPropertyValue(_cam, V4L2_CID_CONTRAST, QVariant::fromValue(valueInt));
  }
}

double V4L2Camera::getBrightness(void) const
{
  QVariant value;
  int Min, Max;
  if (getPropertyValue(_cam, V4L2_CID_BRIGHTNESS, value, &Min, &Max))
  { //scale to 0.0 - 1.0
    double valueDouble = static_cast<double>(value.toInt()+Min)/static_cast<double>(Max);
    if (Min==0 && Max==255)
    { //hack fix: round to 2 digits
      valueDouble = std::round(100.0*valueDouble)/100.0;
    }
    return valueDouble;
  }
  return 0.0;
}

void V4L2Camera::setBrightness(double value)
{ //accepts values between 0.0 and 1.0
  QVariant curVal;
  int Min, Max;
  if (getPropertyValue(_cam, V4L2_CID_BRIGHTNESS, curVal, &Min, &Max))
  { //descale from 0.0 - 1.0
    int valueInt = static_cast<int>(value*static_cast<double>(Max)-static_cast<double>(Min));
    setPropertyValue(_cam, V4L2_CID_BRIGHTNESS, QVariant::fromValue(valueInt));
  }
}

QString V4L2Camera::getFormat(void) const
{
  struct v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (_cam>=0 && xioctl(_cam, VIDIOC_G_FMT, &fmt)>=0) 
  {
    return QString("%1:%2x%3").arg(getFormatName(fmt.fmt.pix.pixelformat))
                              .arg(fmt.fmt.pix.width).arg(fmt.fmt.pix.height);
  }
  return ImageBuffer().formatName(ImageBuffer::UnknownFormat);
}

void V4L2Camera::setFormat(QString const& value)
{
  if (_cam<0) { return; }

  struct v4l2_format fmt;

  //query current format
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(_cam, VIDIOC_G_FMT, &fmt)<0)
  { //error: abort
    log("[ERROR] setFormat VIDIOC_G_FMT: %d %s\n", errno, strerror(errno));
    return;
  }

  int n1 = value.indexOf(":");
  int n2 = value.indexOf("x", (n1>=0?n1:0));
  if (n1<0 || n2<0)
  {
    log("[ERROR] setFormat invalid format: '%s'\n", qPrintable(value));
    return;
  }
  bool ok1, ok2;
  int frame_width = value.mid(n1+1,n2-n1-1).toUInt(&ok1);
  int frame_height = value.mid(n2+1).toUInt(&ok2);

  //log("setFormat: frame_width %d, frame_height %d\n", frame_width, frame_height);
  
  if (!ok1 || !ok2 || frame_width<1 || frame_height <1)
  {
    log("[ERROR] setFormat invalid format: '%s'\n", qPrintable(value));
    return;
  }

  //set new format
  fmt.fmt.pix.pixelformat = getPixelFormat(value);
  fmt.fmt.pix.width       = frame_width;
  fmt.fmt.pix.height      = frame_height;
  if(xioctl(_cam, VIDIOC_S_FMT, &fmt)<0) 
  { //error
    log("[ERROR] setFormat VIDIOC_S_FMT: %d %s\n", errno, strerror(errno));
    return;
  }

  //ok
  return;
}


QStringList V4L2Camera::listCameraFormats(void)
{
  QStringList list;

  if (_cam<0) { /* error */ return list; }

  int fIndex = 0;
  int fRv = 1;
  while (fRv>=0)
  {
    v4l2_fmtdesc fReq;
    memset(&fReq, 0, sizeof(fReq));
    fReq.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fReq.index = fIndex;

    fRv = xioctl(_cam, VIDIOC_ENUM_FMT, &fReq);

    if (fRv==0)
    {
      QString fName = getFormatName(fReq.pixelformat);
      log(" fIndex %d: %s [%s]\n", fIndex, fReq.description, qPrintable(fName));

      int rIndex = 0;
      int rRv = 1;
      while (rRv>=0)
      {
        v4l2_frmsizeenum rReq;
        memset(&rReq, 0, sizeof(rReq));
        rReq.pixel_format = fReq.pixelformat;
        rReq.index = rIndex;

        rRv = xioctl(_cam, VIDIOC_ENUM_FRAMESIZES, &rReq);

        if (rRv==0)
        {
          int frame_width = 0, frame_height = 0;
          if (rReq.type==V4L2_FRMSIZE_TYPE_DISCRETE) 
          {
            frame_width = rReq.discrete.width;
            frame_height = rReq.discrete.height;
          } 
          else if (rReq.type==V4L2_FRMSIZE_TYPE_STEPWISE)
          {
            frame_width = rReq.stepwise.max_width;
            frame_height = rReq.stepwise.max_height;
            /*
            log("FRAMESIZE: %dx%d - %dx%d with step %d/%d\n", rReq.stepwise.min_width,                                                                                         rReq.stepwise.min_height,
                                                              rReq.stepwise.max_width,
                                                              rReq.stepwise.max_height,
                                                              rReq.stepwise.step_width,
                                                              rReq.stepwise.step_height);
`           */                                                              
          }
          //log(" -- rIndex %d: %ux%u\n", rIndex, frame_width, frame_height);

          if (fName!="UNKN")
          { //add only known formats
            list.append(QString("%1:%2x%3%4").arg(fName)
                                           .arg(frame_width)
                                           .arg(frame_height)
                                           .arg((rReq.type==V4L2_FRMSIZE_TYPE_STEPWISE?" stepwise":"")));
          }
        }
        ++rIndex;
      }

    }
    ++fIndex;
  }


  /*
  {
    int index = 0;
    int rv = 1;
    while (rv>=0)
    {
      v4l2_input req;
      memset(&req, 0, sizeof(req));
      req.index = index;

      rv = xioctl(_cam, VIDIOC_ENUMINPUT, &req);

      if (rv==0)
      {
        log(" ---- index %d: %s\n", index, req.name);
      }
      ++index;
    }
  }
  */

  return list;
}

QString V4L2Camera::name2var(unsigned char * name)
{
  QString s;
  bool add_underscore = 0;
  while (*name) 
  {
    if (isalnum(*name)) 
    {
      if (add_underscore) { s += '_'; add_underscore = false; }
      s += (*name);
    }
    else if (s.length())
    {
      add_underscore = true;
    }
    ++name;
  }
  return s.toLower();
}

QStringList V4L2Camera::getControlMenu(int cam, v4l2_queryctrl const& queryctrl)
{
  QStringList list;
  v4l2_querymenu querymenu;
  memset (&querymenu, 0, sizeof (querymenu));
  querymenu.id = queryctrl.id;
  for (querymenu.index=queryctrl.minimum; 
        static_cast<int>(querymenu.index)<=queryctrl.maximum; ++querymenu.index)
  {
    if (ioctl(cam, VIDIOC_QUERYMENU, &querymenu)==0)
    {
      list.append(name2var(querymenu.name));
    }
  }
  return list;
}

void V4L2Camera::findControls(void)
{
  QMap<QString,size_t> controlMap;

  if (_cam>=0)
  {
    v4l2_queryctrl qctrl;
    memset (&qctrl, 0, sizeof (qctrl));
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
    while (ioctl(_cam, VIDIOC_QUERYCTRL, &qctrl)==0) 
    { //standard controls
      if (qctrl.type!=V4L2_CTRL_TYPE_CTRL_CLASS && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)) 
      {
        auto name = name2var(qctrl.name);
        controlMap.insert(name, qctrl.id);
        if (qctrl.type==V4L2_CTRL_TYPE_MENU)
        { //menu
          _controlMenu.insert(name, getControlMenu(_cam, qctrl));
        }
      }
      qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
    if (qctrl.id != V4L2_CTRL_FLAG_NEXT_CTRL)
    {
      int id;
      for (id = V4L2_CID_USER_BASE; id < V4L2_CID_LASTP1; id++) 
      { //user controls
        qctrl.id = id;
        if (ioctl(_cam, VIDIOC_QUERYCTRL, &qctrl)==0 && !(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)) 
        {
          auto name = name2var(qctrl.name);
          controlMap.insert(name, qctrl.id);
          if (qctrl.type==V4L2_CTRL_TYPE_MENU)
          { //menu
            _controlMenu.insert(name, getControlMenu(_cam, qctrl));
          }
        }
      }
      for (qctrl.id=V4L2_CID_PRIVATE_BASE; ioctl(_cam, VIDIOC_QUERYCTRL, &qctrl)==0; qctrl.id++) 
      { //private controls
        if (!(qctrl.flags & V4L2_CTRL_FLAG_DISABLED)) 
        {
          auto name = name2var(qctrl.name);
          controlMap.insert(name, qctrl.id);
          if (qctrl.type==V4L2_CTRL_TYPE_MENU)
          { //menu
            _controlMenu.insert(name, getControlMenu(_cam, qctrl));
          }
        }
      }
    }
  }//if _cam

  //save new map
  _controlMap.swap(controlMap);
}    

bool V4L2Camera::getPropertyValue(int cam, size_t property, QVariant & value, 
                                  int *minVal, int *maxVal, int *defaultVal)
{
  if (cam<0) { return false; }

  v4l2_queryctrl queryctrl;
  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = property;
  if (ioctl(cam, VIDIOC_QUERYCTRL, &queryctrl)<0) 
  {
    if (errno != EINVAL) {
      log("[ERROR] VIDIOC_QUERYCTRL: %d %s\n", errno, strerror(errno));
    } else {
      log("[ERROR] property %d is not supported\n", property);
    }
    return false;
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    log("[WARN] property %d is disabled\n", property);
    return false;
  }

  v4l2_control control;
  memset (&control, 0, sizeof (control));
  control.id = property;

  if (ioctl(cam, VIDIOC_G_CTRL, &control)<0) 
  { //error
    log("[ERROR] VIDIOC_G_CTRL failed: %d %s\n", errno, strerror(errno));
    return false;
  }
  

  auto name = name2var(queryctrl.name);
    
  if (minVal) { *minVal = queryctrl.minimum; }
  if (maxVal) { *maxVal = queryctrl.maximum; }
  if (defaultVal) { *defaultVal = queryctrl.default_value; }

  if (queryctrl.type==V4L2_CTRL_TYPE_INTEGER)
  { //integer
    log("get property %s: val %d (default %d min %d max %d)\n", qPrintable(name),
          control.value, queryctrl.default_value, 
          queryctrl.minimum, queryctrl.maximum);
    value = QVariant::fromValue(control.value);
    return true;
  }
  if (queryctrl.type==V4L2_CTRL_TYPE_MENU)
  { //menu
    v4l2_querymenu querymenu;
    memset (&querymenu, 0, sizeof (querymenu));
    querymenu.id = queryctrl.id;
    querymenu.index = control.value;

    if (ioctl(cam, VIDIOC_QUERYMENU, &querymenu)==0)
    {
      value = QVariant::fromValue(name2var(querymenu.name));
      return true;
    }
 
    return false;
  }
  if (queryctrl.type==V4L2_CTRL_TYPE_BOOLEAN)
  { //bool (as integer)
    value = QVariant::fromValue(control.value);
    return true;
  }

  //unhandled type
  QString typeName;
  switch (queryctrl.type)
  {
    case V4L2_CTRL_TYPE_INTEGER:        typeName="INTEGER"; break;
    case V4L2_CTRL_TYPE_BOOLEAN:        typeName="BOOLEAN"; break;
    case V4L2_CTRL_TYPE_MENU:           typeName="MENU"; break;
    //case V4L2_CTRL_TYPE_INTEGER_MENU:   typeName="INTEGER_MENU"; break;
    case V4L2_CTRL_TYPE_BITMASK:        typeName="BITMASK"; break;
    case V4L2_CTRL_TYPE_BUTTON:         typeName="BUTTON"; break;
    case V4L2_CTRL_TYPE_INTEGER64:      typeName="INTEGER64"; break;
    case V4L2_CTRL_TYPE_STRING:         typeName="STRING"; break;
    case V4L2_CTRL_TYPE_CTRL_CLASS:     typeName="CLASS"; break;
    //case V4L2_CTRL_TYPE_U8:             typeName="U8"; break;
    //case V4L2_CTRL_TYPE_U16:            typeName="U16"; break;
    default:                            typeName="unknown"; break;
  }
  
  log("[WARN] property %s unsupported type %s (%d)\n", 
        qPrintable(name), qPrintable(typeName), queryctrl.type);
  return false;
}

bool V4L2Camera::setPropertyValue(int cam, size_t property, QVariant value)
{
  if (cam<0) { return false; }

  v4l2_queryctrl queryctrl;
  memset (&queryctrl, 0, sizeof (queryctrl));
  queryctrl.id = property;
  if (ioctl(cam, VIDIOC_QUERYCTRL, &queryctrl)<0) 
  {
    if (errno != EINVAL) {
      log("[ERROR] VIDIOC_QUERYCTRL: %d %s\n", errno, strerror(errno));
    } else {
      log("[ERROR] property %d is not supported\n", property);
    }
    return false;
  } else if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    log("[WARN] property %d is disabled\n", property);
    return false;
  }

  v4l2_control control;
  memset (&control, 0, sizeof (control));
  control.id = property;
  
  auto name = name2var(queryctrl.name);
  
  switch (queryctrl.type)
  {
    case V4L2_CTRL_TYPE_INTEGER:
    case V4L2_CTRL_TYPE_BOOLEAN:
      control.value = value.toInt();
      break;
    case V4L2_CTRL_TYPE_MENU:
      if (static_cast<QMetaType::Type>(value.type())==QMetaType::Int
          || static_cast<QMetaType::Type>(value.type())==QMetaType::UInt)
      { //integer given (set directly)
        control.value = value.toInt();
      }
      else
      { //search the menu option
        auto menuOption = value.toString();
        v4l2_querymenu querymenu;
        memset (&querymenu, 0, sizeof (querymenu));
        querymenu.id = queryctrl.id;
        int menuIndex = -1; 
        for (querymenu.index=queryctrl.minimum; 
              static_cast<int>(querymenu.index)<=queryctrl.maximum; ++querymenu.index)
        {
          if (ioctl(cam, VIDIOC_QUERYMENU, &querymenu)==0
              && menuOption==name2var(querymenu.name))
          { //menu option found
            menuIndex = querymenu.index; break;
          }
        }
        if (menuIndex<0)
        { //invalid menu option
          return false;
        }
        control.value = menuIndex; //found
      }
      break;
    //case V4L2_CTRL_TYPE_INTEGER_MENU:
    case V4L2_CTRL_TYPE_BITMASK:
    case V4L2_CTRL_TYPE_BUTTON:
    case V4L2_CTRL_TYPE_INTEGER64:
    case V4L2_CTRL_TYPE_STRING:
    case V4L2_CTRL_TYPE_CTRL_CLASS:
    //case V4L2_CTRL_TYPE_U8:
    //case V4L2_CTRL_TYPE_U16:
    default:
      return false; break;
  }

  if (ioctl(cam, VIDIOC_S_CTRL, &control)<0) 
  { //error
    log("[ERROR] VIDIOC_S_CTRL failed: %d %s\n", errno, strerror(errno));
    return false;
  }

  //ok
  log("set property %s: val %d\n", qPrintable(name), control.value);
  return true;
}

const char * V4L2Camera::getFormatName(size_t pixelformat)
{
  switch(pixelformat) 
  {
  case V4L2_PIX_FMT_JPEG:
  case V4L2_PIX_FMT_MJPEG:  
    return ImageBuffer().formatName(ImageBuffer::JPEG);
  case V4L2_PIX_FMT_YUYV:
    return ImageBuffer().formatName(ImageBuffer::YUV422);
  case V4L2_PIX_FMT_H264:
  case V4L2_PIX_FMT_M420:
  default:
    break;
  }

  return ImageBuffer().formatName(ImageBuffer::UnknownFormat);
}

size_t V4L2Camera::getPixelFormat(QString format)
{
  int n = format.indexOf(":");
  if (n>0) { format = format.mid(0, n); }

  if (format==ImageBuffer().formatName(ImageBuffer::JPEG))
  {
    return V4L2_PIX_FMT_MJPEG;
  }
  if (format==ImageBuffer().formatName(ImageBuffer::YUV422))
  {
    return V4L2_PIX_FMT_YUYV;
  }

  //unknown
  return 0;
}

void V4L2Camera::disableAuto(void)
{
  auto valueList = _controlMap.values();
  if (valueList.indexOf(V4L2_CID_EXPOSURE_AUTO)>=0)
  {
    bool rv = setPropertyValue(_cam, V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_MANUAL);
    log("[INFO] setting EXPOSURE_AUTO=MANUAL %s\n", (rv?"OK":"FAILED!"));
  }
  if (valueList.indexOf(V4L2_CID_FOCUS_AUTO)>=0)
  {
    bool rv = setPropertyValue(_cam, V4L2_CID_FOCUS_AUTO, 0);
    log("[INFO] setting FOCUS_AUTO=FALSE %s\n", (rv?"OK":"FAILED!"));
  }
  if (valueList.indexOf(V4L2_CID_FOCUS_ABSOLUTE)>=0)
  { //set the focus to infinity
    QVariant curVal;
    int Min, Max;
    if (getPropertyValue(_cam, V4L2_CID_FOCUS_ABSOLUTE, curVal, &Min, &Max))
    {
      bool rv = setPropertyValue(_cam, V4L2_CID_FOCUS_ABSOLUTE, QVariant::fromValue(Min));
      log("[INFO] setting FOCUS_ABSOLUTE=%d (min) %s\n", Min, (rv?"OK":"FAILED!"));
    }
  }
}

//------------------------------------------------------------------------------------------
// V4L2CameraWorker
//------------------------------------------------------------------------------------------

V4L2CameraWorker::V4L2CameraWorker(int cam) :
  QObject(NULL),
  _cam(cam),
  _buffer(),
  _state(StateStop)
{
  if (!allocateBuffers())
  { //error: allocation failed
    deallocateBuffers(); //clean up
  }
  log("created.\n");
}

V4L2CameraWorker::~V4L2CameraWorker()
{
  stopStreaming();
  deallocateBuffers();
  log("destroyed.\n");
}

bool V4L2CameraWorker::allocateBuffers(size_t n) 
{
  if (_cam<0) { return false; }

  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req)); 
  req.count = n;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if(xioctl(_cam, VIDIOC_REQBUFS, &req)<0) {
    if(EINVAL == errno) {
      log("[ERROR] device does not support memory mapping\n");
    } else {
      log("[ERROR] VIDIOC_REQBUFS\n");
    }
    return false;
  }
  if(req.count < 2) {
    log("[ERROR] insufficient buffer memory on device\n");
    return false;
  }

  _buffer.resize(req.count);
  for (size_t i=0; i<_buffer.size(); ++i)
  {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf)); 
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;
    if(xioctl(_cam, VIDIOC_QUERYBUF, &buf)<0) {
      log("[ERROR] VIDIOC_QUERYBUF\n");
      return false;
    }
    _buffer[i].data = static_cast<unsigned char*>(
                    mmap(NULL /* start anywhere */,
                        buf.length,
                        PROT_READ | PROT_WRITE /* required */,
                        MAP_SHARED /* recommended */,
                        _cam,
                        buf.m.offset));
    if(_buffer[i].data == MAP_FAILED) {
      _buffer[i].data = NULL;
      log("[ERROR] mmap\n");
      return false;
    }
  }
  return true; // success
}

bool V4L2CameraWorker::startStreaming(void) 
{
  if (_cam<0) { return false; }

  //query current format
  v4l2_format fmt;
  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (xioctl(_cam, VIDIOC_G_FMT,&fmt)<0)
  {
    log("[ERROR] VIDIOC_G_FMT\n");
    return false;
  }
  
  int format = ImageBuffer::UnknownFormat;
  switch (fmt.fmt.pix.pixelformat)
  {
  case V4L2_PIX_FMT_RGB24: format = ImageBuffer::BGR24; break; //report as BGR
  case V4L2_PIX_FMT_RGB32: format = ImageBuffer::BGR32; break; //report as BGR
  case V4L2_PIX_FMT_YUYV:  format = ImageBuffer::YUV422; break;
  case V4L2_PIX_FMT_MJPEG: format = ImageBuffer::JPEG; break;
  default: break;
  }

  log(" image format set to %s %dx%d\n", ImageBuffer().formatName(format),
          fmt.fmt.pix.width, fmt.fmt.pix.height);

  //fill metadata and queue all buffers
  for (size_t i=0; i<_buffer.size(); ++i) 
  {
    if (_buffer[i].data)
    {
      //complete buffer metadata
      _buffer[i].rows = fmt.fmt.pix.height;
      _buffer[i].cols = fmt.fmt.pix.width;
      _buffer[i].format = format;
      _buffer[i].bytes = 0;

      struct v4l2_buffer buf;
      memset(&buf, 0, sizeof(buf)); 
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      if(xioctl(_cam, VIDIOC_QBUF, &buf)<0) 
      {
        log("[ERROR] VIDIOC_QBUF\n");
        return false;
      }
    }
  }

  //start streaming
  v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(xioctl(_cam, VIDIOC_STREAMON, &type)<0) 
  {
    log("[ERROR] stream start failed (VIDIOC_STREAMON)\n");
    return false;
  }

  return true; // success
}

bool V4L2CameraWorker::stopStreaming(void)
{
  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  return (xioctl(_cam, VIDIOC_STREAMOFF, &type)>=0);
}

void V4L2CameraWorker::deallocateBuffers(void)
{
  for (size_t i=0; i<_buffer.size(); ++i)
  {
    auto & image = _buffer[i];
    if (!image.data) { continue; }
    
    //query buffer length
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf)); 
    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = i;
    if (xioctl(_cam, VIDIOC_QUERYBUF, &buf)<0) {
      log("[ERROR] deallocateBuffers: VIDIOC_QUERYBUF, index %u\n", i);
      continue;
    }

    //unmap
    if (munmap(image.data, buf.length)<0)
    {
      log("[ERROR] deallocateBuffers: munmap failed, index %u\n", i);
      continue;
    }
  }
  _buffer.clear();

  //free buffer on the device
  struct v4l2_requestbuffers req;
  memset(&req, 0, sizeof(req)); 
  req.count = 0;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if(xioctl(_cam, VIDIOC_REQBUFS, &req)<0) {
    if(EINVAL == errno) {
      log("[ERROR] deallocateBuffers: device does not support memory mapping\n");
    } else {
      log("[ERROR] deallocateBuffers: VIDIOC_REQBUFS\n");
    }
  }
}

void V4L2CameraWorker::start()
{
  log("[INFO] start.\n");
  _state = StateInit;

  if (startStreaming())
  { //start ok
    _state = StateStreaming;
 
    //wait for frames
    while (_state==StateStreaming)
    {
      // wait for frames
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET(_cam, &fds);
    
      struct timeval tv;
      tv.tv_sec  = 2;
      tv.tv_usec = 0;
   
      if (select(FD_SETSIZE, &fds, NULL, NULL, &tv)>0 && FD_ISSET(_cam,&fds))
      {
        readFrame();
      }
  
      //handle pending signals (like stop!)
      QCoreApplication::processEvents();
    }
  }

  //stop streaming
  if (!stopStreaming())
  {
    log("[ERROR] stream stop failed (VIDIOC_STREAMOFF): %d %s\n", errno, strerror(errno));
  }
  deallocateBuffers();

  _state = StateStop;
  log("[INFO] end.\n");
  QThread::currentThread()->quit(); //make sure our thread finishes
}

bool V4L2CameraWorker::readFrame(void) 
{
  //deque buffer
  struct v4l2_buffer buf;
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (xioctl(_cam, VIDIOC_DQBUF, &buf)<0) 
  {
    log("[ERROR] readFrame: VIDIOC_DQBUF failed\n");
    return false; // error
  }

  //log(" buffer size %d\n", buf.bytesused);

  //send
  if (buf.index < _buffer.size())
  {
    //log(" image buffer %d: %dx%d\n", buf.index, 
    //       _buffer[buf.index].cols, _buffer[buf.index].rows);
    _buffer[buf.index].bytes = buf.bytesused;
    emit newImage(_buffer[buf.index]);
    _buffer[buf.index].bytes = 0;
  }

  //queue buffer back
  if (xioctl(_cam, VIDIOC_QBUF, &buf)<0) 
  {
    log("[ERROR] VIDIOC_QBUF\n");
    return false; // error
  }

  return true; // success
}
