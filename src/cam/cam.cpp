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

#include "cam.hpp"

#ifdef HAVE_DSHOW
# include "DShowCamera.hpp"
#endif //HAVE_DSHOW

#ifdef HAVE_QTMULTIMEDIA
# include "QtCamera.hpp"
#endif //HAVE_QTMULTIMEDIA

QStringList cam::getCameraList(void)
{
  QStringList list;

#ifdef USE_COGNEX
  QStringList cgList = CognexCamera::getCameraList();
  foreach(QString cam, cgList)
  {
    list.append("cg:"+cam);
  }
#endif //USE_COGNEX

#ifdef HAVE_DSHOW
  QStringList dsList = DShowCamera::getCameraList();
  foreach(QString cam, dsList)
  {
    list.append("ds:"+cam);
  }
#endif //HAVE_DSHOW

#ifdef HAVE_QTMULTIMEDIA
  QStringList qtList = QtCamera::getCameraList();
  foreach(QString cam, qtList)
  {
    list.append("qt:" + cam);
  }
#endif //HAVE_QTMULTIMEDIA

#ifdef USE_PGREY
  QStringList pgList = PGreyCamera::getCameraList();
  foreach(QString cam, pgList)
  {
    list.append("pg:"+cam);
  }
#endif //USE_PGREY

#ifdef USE_BASLER
  QStringList bsList = BaslerCamera::getCameraList();
  foreach(QString cam, bsList)
  {
    list.append("bs:"+cam);
  }
#endif //USE_BASLER

  return list;
}


QSharedPointer<CameraInterface> cam::getCamera(QString cameraName)
{
#ifdef USE_COGNEX
  if (cameraName.left(3)=="cg:")
  {
    return CognexCamera::getCamera(cameraName.mid(3));
  }
#endif //USE_COGNEX

#ifdef HAVE_DSHOW
  if (cameraName.left(3)=="ds:")
  {
    return DShowCamera::getCamera(cameraName.mid(3));
  }
#endif //HAVE_DSHOW

#ifdef HAVE_QTMULTIMEDIA
  if (cameraName.left(3) == "qt:")
  {
    return QtCamera::getCamera(cameraName.mid(3));
  }
#endif //HAVE_QTMULTIMEDIA

#ifdef USE_PGREY
  if (cameraName.left(3)=="pg:")
  {
    return PGreyCamera::getCamera(cameraName.mid(3));
  }
#endif //USE_PGREY

#ifdef USE_BASLER
  if (cameraName.left(3)=="bs:")
  {
    return BaslerCamera::getCamera(cameraName.mid(3));
  }
#endif //USE_BASLER

  //unknown
  return QSharedPointer<CameraInterface>();
}
