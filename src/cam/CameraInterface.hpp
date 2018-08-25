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

#ifndef __CameraInterface_hpp__
#define __CameraInterface_hpp__

#include <QObject>

#ifdef HAVE_IMG
class ImageBuffer;
#endif //HAVE_IMG

#define CAM_PROP_FORMAT     "format"
#define CAM_PROP_EXPOSURE   "exposure"
#define CAM_PROP_CONTRAST   "contrast"
#define CAM_PROP_BRIGHTNESS "brightness"

class CameraInterface : public QObject
{
  Q_OBJECT;

public:
  virtual bool start()=0;
  virtual void stop()=0;

  virtual QStringList getPropertyList(void)=0;
  virtual QVariant getCameraProperty(QString propertyName)=0;
  virtual void setCameraProperty(QString propertyName, QVariant value)=0;

  virtual QStringList const& getFormatList(void) const=0;

signals:
#ifdef HAVE_IMG
  void newImage(ImageBuffer const& image);
#endif //HAVE_IMG
};

#endif // __CameraInterface_hpp__
