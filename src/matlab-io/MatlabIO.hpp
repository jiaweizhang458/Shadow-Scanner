/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2012, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  File:    MatlabIO.hpp
 *  Author:  Hilton Bristow
 *  Created: Jun 27, 2012
 */

// Modified by Daniel Moreno Feb 9 2015

#ifndef MATLABIO_HPP_
#define MATLABIO_HPP_

#include <QMap>
#include <QString>
#include <QVector>
#include <QVariant>
#include <QByteArray>

#include "EFStream.hpp"
#include "typetraits.hpp"

enum {
  VERSION_5      = 5,
  VERSION_73     = 73
};

/*! @class MatlabIO
 *  @brief Matlab Mat file parser for C++ OpenCV
 *
 *  This class provides the capacity to read and write Mat files
 *  produced by Matlab. The OpenCV cv::Mat class is used for the
 *  internal storage of matrices. The class also provides methods
 *  to inspect the contents of a Mat-file, read all variables, or
 *  read a single variable by name.
 */

class MatlabIO {
private:
  // member variables
  static const int HEADER_LENGTH  = 116;
  static const int SUBSYS_LENGTH  = 8;
  static const int ENDIAN_LENGTH  = 2;
  char header_[HEADER_LENGTH+1];
  char subsys_[SUBSYS_LENGTH+1];
  char endian_[ENDIAN_LENGTH+1];

  int16_t   _version;
  bool      _byte_swap;
  int       _bytes_read;
  QString   _filename;
  EFStream  _fid;

  // internal methods
  void getHeader(void);
  bool hasVariable(void) { return _fid.peek() != EOF; }

  template<class T> QVariant constructMatrix(QVector<uint32_t>& dims, QByteArray real, QByteArray imag, uint32_t stor_type);
  QVariant constructCell(QVector<uint32_t>& dims, QByteArray real);
  QVariant constructStruct(QVector<uint32_t>& dims, QByteArray real);

  bool collateMatrixFields(uint32_t data_type, uint32_t nbytes, QByteArray data, QString & varName, QVariant & varData);
  QByteArray uncompressVariable(uint32_t& data_type, uint32_t& dbytes, uint32_t& wbytes, QByteArray data);

  const char * readVariableTag(uint32_t &data_type, uint32_t &dbytes, uint32_t &wbytes, const char *data);
  bool readVariable(uint32_t data_type, uint32_t nbytes, QByteArray data, QString & varName, QVariant & varData);
  bool readBlock(QString & varName, QVariant & varData);

public:
  // constructors
  MatlabIO() {}

  // destructor
  ~MatlabIO() { close(); }

  // get and set methods
  QString filename(void) { return _filename; }

  // read and write routines
  bool open(QString filename, QString mode);
  bool close(void);
  QMap<QString,QVariant> read(void);
  void whos(QMap<QString,QVariant> const& varMap) const;
};

#endif /* MATLABIO_HPP_ */
