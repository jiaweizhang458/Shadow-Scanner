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
 *  File:    MatlabIO.cpp
 *  Author:  Hilton Bristow
 *  Created: Jun 27, 2012
 */

// Modified by Daniel Moreno  Feb 9 2015
// Modified by Gabriel Taubin Mar 8 2016

#include "MatlabIO.hpp"

#include <QVector>

#include <cerrno>
#include <cstring>
#include <cassert>
#include <iostream>

/*! @brief Open a filestream for reading or writing
 *
 * @param filename the full name and filepath of the file
 * @param mode either "r" for reading or "w" for writing
 * @return true if the file open succeeded, false otherwise
 */

bool MatlabIO::open(QString filename, QString mode) {
  // open the file
  _filename = filename;
  if (mode.compare("r") == 0)
    _fid.open(qPrintable(filename), std::fstream::in  | std::fstream::binary);
  if (mode.compare("w") == 0)
    _fid.open(qPrintable(filename), std::fstream::out | std::fstream::binary);
  return !_fid.fail();
}

/*! @brief close the filestream and release all resources
 *
 * @return true if the filestream was successfully closed,
 * false otherwise. Even in the case of failure, the filestream
 * will no longer point to a valid object
 */

bool MatlabIO::close(void) {
  // close the file and release any associated objects
  _fid.close();
  return !_fid.fail();
}

/*! @brief product of the elements of a vector
 *
 * The function is useful for calculating the total number
 * of elements in an array given a vector of dims.
 * @param vec the input vector
 * @return the product of elements in the input
 */

template<typename T>
T product(const QVector<T>& vec) {
  T acc = 1;
  for (int n = 0; n < vec.size(); ++n)
    acc *= vec[n];
  return acc;
}

/*! @brief convert the type of a variable
 *
 * Given a vector of type char, interpret the data as an vector of
 * type T1, and convert it to a vector of type T2.
 *
 * @param in the input char vector
 * @return the same data, reinterpreted as type T2 through
 * storage type T1
 */

template<class T1, class T2>
QVector<T2> convertPrimitiveType(QByteArray in) {
  // firstly reinterpret the input as type T1
  const unsigned int T1_size = in.size() / sizeof(T1);
  const T1* in_ptr = reinterpret_cast<const T1*>(in.data());

  // construct the new vector
  QVector<T2> out;
  for (unsigned int i = 0U; i < T1_size; ++i) {
    out.push_back(static_cast<T2>(in_ptr[i]));
  }
  
  return out;
}

/*! @brief get the .Mat file header information
 *
 * The fields read are: header_ the matlab header as a human readable
 * string, subsys_ subsystem specific information, _version the .Mat
 * file version (5 or 73), endian_ the bye ordering of the .Mat
 * file. If the byte ordering needs reversal, this is automatically
 * handled by esfstream.
 */

void MatlabIO::getHeader(void) {

  // get the header information from the Mat file
  for (unsigned int n = 0; n < HEADER_LENGTH+1; ++n) header_[n] = '\0';
  for (unsigned int n = 0; n < SUBSYS_LENGTH+1; ++n) subsys_[n] = '\0';
  for (unsigned int n = 0; n < ENDIAN_LENGTH+1; ++n) endian_[n] = '\0';
  _fid.read(header_, sizeof(char)*HEADER_LENGTH);
  _fid.read(subsys_, sizeof(char)*SUBSYS_LENGTH);
  _fid.read((char *)&_version, sizeof(int16_t));
  _fid.read(endian_, sizeof(char)*ENDIAN_LENGTH);

  // get the actual version
  if (_version == 0x0100) _version = VERSION_5;
  if (_version == 0x0200) _version = VERSION_73;

  // get the endianess
  if (strcmp(endian_, "IM") == 0) _byte_swap = false;
  if (strcmp(endian_, "MI") == 0) _byte_swap = true;
  // turn on byte swapping if necessary
  _fid.setByteSwap(_byte_swap);

  //printf("Header: %s\nSubsys: %s\nVersion: %d\nEndian: %s\nByte
  // Swap: %d\n", header_, subsys_, version_, endian_, byte_swap_);

  _bytes_read = 128;
}

/*! @brief interpret the variable header information
 *
 * Given a binary data blob, determine the data type and number of
 * bytes that constitute the data blob. This internally handles
 * whether the header is in long or short format.
 *
 * @param data_type the returned data type
 * @param dbytes the returned number of bytes that constitute the data blob
 * @param wbytes the whole number of bytes that include the header size,
 * the size of the data and any padding to 64-bit boundaries. This is equivalent
 * to the entire number of bytes effectively used by a variable
 * @param data the input binary blob
 * @return a pointer to the beginning of the data segment of the binary blob
 */

const char * MatlabIO::readVariableTag
(uint32_t &data_type, uint32_t &dbytes, uint32_t &wbytes, const char *data) {    
  bool small = false;
  const uint32_t *datai = reinterpret_cast<const uint32_t *>(data);
  data_type = datai[0];

  if ((data_type >> 16) != 0) {
    // small data format
    dbytes = data_type >> 16;
    data_type = (data_type << 16) >> 16;
    small = true;
  } else {
    // regular format
    dbytes = datai[1];
  }

  // get the whole number of bytes (wbytes) consumed by this variable,
  // including header and padding
  if (small) wbytes = 8;
  else if (data_type == MAT_COMPRESSED) wbytes = 8 + dbytes;
  else wbytes = 8 + dbytes + ((8-dbytes) % 8);

  // return the seek head positioned over the data payload
  return data + (small ? 4 : 8);
}

/*! @brief construct a structure
 *
 * TODO: implement this
 * @param dims
 * @param real
 * @return
 */

QVariant MatlabIO::constructStruct(QVector<uint32_t>& dims, QByteArray real) {

  // get the length of each field
  uint32_t length_type, length_dbytes, length_wbytes;
  uint32_t length =
    *reinterpret_cast<const uint32_t*>
    (readVariableTag(length_type, length_dbytes, length_wbytes, real.data()));

  // get the total number of fields
  uint32_t nfields_type, nfields_dbytes, nfields_wbytes;
  const char* nfields_ptr =
    readVariableTag(nfields_type, nfields_dbytes, nfields_wbytes,
                    real.data()+length_wbytes);
  assert((nfields_dbytes % length) == 0);
  uint32_t nfields = nfields_dbytes / length;

  // populate a vector of field names
  QStringList field_names;
  for (unsigned int n = 0; n < nfields; ++n) {
    field_names.push_back(nfields_ptr+(n*length));
  }

  // iterate through each of the cells and construct the matrices
  QVector<QMap<QString,QVariant>> array;
  const char* field_ptr = real.data()+length_wbytes+nfields_wbytes;
  for (unsigned int m = 0; m < product<uint32_t>(dims); ++m) {
    QMap<QString,QVariant> strct;
    for (unsigned int n = 0; n < nfields; ++n) {
      uint32_t data_type, dbytes, wbytes;
      const char* data_ptr =
        readVariableTag(data_type, dbytes, wbytes, field_ptr);
      assert(data_type == MAT_MATRIX);
      
      QString varName;
      QVariant varData;
      if (collateMatrixFields(data_type, dbytes,
                              QByteArray(data_ptr, dbytes), varName, varData)) {
        strct.insert(field_names[n], varData);
      }
      field_ptr += wbytes;
    }
    array.push_back(strct);
  }
  return QVariant::fromValue(array);
}

/*! @brief construct a cell array
 *
 * If the variable is of type MAT_CELL, construct a cell array. This
 * is done by iteratively calling collateMatrixFields() on each
 * element of the cell, and storing the result in a
 * vector<MatlabIOContainer>.  Cell fields may not have a name, but
 * are still required to have a name tag. In this case, placeholder
 * names are substituted. The dimensionality of the cell array is
 * ignored, and the size is linearized in column major format.
 *
 * @param dims the dimesionality of the cell array (ignored)
 * @param real the real part
 * @return the wrapped cell array
 */

QVariant MatlabIO::constructCell(QVector<uint32_t>& dims, QByteArray real) {

  QVector<QVariant> cell;
  const char* field_ptr = real.data();
  for (unsigned int n = 0; n < product<uint32_t>(dims); ++n) {
    uint32_t data_type, dbytes, wbytes;
    const char* data_ptr = readVariableTag(data_type, dbytes, wbytes, field_ptr);
    //printf("cell data_type: %d,  dbytes: %d\n", data_type, dbytes);
    assert(data_type == MAT_MATRIX);
    
    QString varName;
    QVariant varData;
    if (collateMatrixFields(data_type, dbytes,
                            QByteArray(data_ptr, dbytes), varName, varData)) {
      cell.push_back(varData);
    }
    field_ptr += wbytes;
  }
  return QVariant::fromValue(cell);
}

/*! @brief construct a matrix from an extracted set of fields
 *
 * Given the variable size, name, data and data type, construct a
 * matrix.  Note that Matlab may store variables in a different data
 * type to the actual variable data type (T) to save space. For
 * example matrix a = [1 2 3 4 5]; in Matlab will intrinsically be of
 * type double (everything is unless otherwise explicitly stated) but
 * could be stored as a uint8_t to save space.  The type of the
 * variable returned should necessarily be double, since it's
 * impossible to know at compile time which data types Matlab has
 * decided to store a set of variables in.
 *
 * @param dims the variable dimensionality (i, j, k, ...)
 * @param real the real part
 * @param imag the imaginary part (imag.size() == 0 if the data is real)
 * @param stor_type the storage type of the value
 * @return the wrapped matrix
 */
template<class T>
QVariant MatlabIO::constructMatrix
(QVector<uint32_t>& dims, QByteArray real, QByteArray imag, uint32_t stor_type) {

  QVector<T> vec_real;
  QVector<T> vec_imag;
  switch (stor_type) {
  case MAT_INT8:
    vec_real = convertPrimitiveType<int8_t, T>(real);
    vec_imag = convertPrimitiveType<int8_t, T>(imag);
    break;
  case MAT_UINT8:
    vec_real = convertPrimitiveType<uint8_t, T>(real);
    vec_imag = convertPrimitiveType<uint8_t, T>(imag);
    break;
  case MAT_INT16:
    vec_real = convertPrimitiveType<int16_t, T>(real);
    vec_imag = convertPrimitiveType<int16_t, T>(imag);
    break;
  case MAT_UINT16:
    vec_real = convertPrimitiveType<uint16_t, T>(real);
    vec_imag = convertPrimitiveType<uint16_t, T>(imag);
    break;
  case MAT_INT32:
    vec_real = convertPrimitiveType<int32_t, T>(real);
    vec_imag = convertPrimitiveType<int32_t, T>(imag);
    break;
  case MAT_UINT32:
    vec_real = convertPrimitiveType<uint32_t, T>(real);
    vec_imag = convertPrimitiveType<uint32_t, T>(imag);
    break;
  case MAT_INT64:
    vec_real = convertPrimitiveType<int64_t, T>(real);
    vec_imag = convertPrimitiveType<int64_t, T>(imag);
    break;
  case MAT_UINT64:
    vec_real = convertPrimitiveType<uint64_t, T>(real);
    vec_imag = convertPrimitiveType<uint64_t, T>(imag);
    break;
  case MAT_FLOAT:
    vec_real = convertPrimitiveType<float, T>(real);
    vec_imag = convertPrimitiveType<float, T>(imag);
    break;
  case MAT_DOUBLE:
    vec_real = convertPrimitiveType<double, T>(real);
    vec_imag = convertPrimitiveType<double, T>(imag);
    break;
  case MAT_UTF8:
    vec_real = convertPrimitiveType<char, T>(real);
    vec_imag = convertPrimitiveType<char, T>(imag);
    break;
  default:
    return QVariant();
  }

  // assert that the conversion has not modified the number of elements
  uint32_t numel = 1;
  for (int n = 0; n < dims.size(); ++n) numel *= dims[n];
  assert(vec_real.size() == numel);

  // if the data is a scalar, don't write it to a matrix
  if (vec_real.size() == 1 && vec_imag.size() == 0) {
    return QVariant::fromValue(vec_real.first());
  }

  // get the number of channels
  const unsigned int channels = dims.size() == 3 ? dims[2] : 1;
  //bool complx = vec_imag.size() != 0;

  // put each plane of the image into a vector
  QVector<QVector<QVector<T>>> mat;
  mat.resize(channels);
  int index = 0;
  for (unsigned int n = 0; n < channels; ++n) {
    auto & channel = mat[n];
    channel.resize(dims[1]);
    for (unsigned int w = 0; w < dims[1]; ++w) {
      auto & col = channel[w];
      col.resize(dims[0]);
      for (unsigned int h = 0; h < dims[0]; ++h) {
        col[h] = vec_real[index++];
      }
    }
  }
  
  return QVariant::fromValue(mat); 
}

/*! @brief interpret all fields of a matrix
 *
 * collateMatrixFields takes a binary blob of data and strips out the
 * matrix fields.  These fields necessarily include: the variable
 * dimensionality, the variable name and the real part of the variable
 * data. It optionally includes the imaginary part of the variable
 * data if that exists too. The extracted fields are used to either
 * construct a matrix, cell array or struct, or a scalar in the case
 * where the variable dimensionality is (1,1)
 *
 * @param data_type the type of the data stored in the binary blob
 * @param nbytes the number of bytes that constitute the binary blob
 * @param data the binary blob
 *
 * @return the variable (matrix, struct, cell, scalar) wrapped in a container
 */

bool MatlabIO::collateMatrixFields
(uint32_t data_type, uint32_t nbytes, QByteArray data,
 QString & varName, QVariant & varData) {

  // get the flags
  bool complx  = data[9] & (1 << 3);
  //bool logical = data[9] & (1 << 1);
    
  // get the type of the encapsulated data
  char enc_data_type = data[8];
  // the preamble size is 16 bytes
  uint32_t pre_wbytes = 16;

  // get the dimensions
  uint32_t dim_type, dim_dbytes, dim_wbytes;
  const uint32_t * dim_data =
    reinterpret_cast<const uint32_t *>
    (readVariableTag(dim_type, dim_dbytes, dim_wbytes, data.data() + pre_wbytes));
  QVector<uint32_t> dims;
  int nDims = dim_dbytes/sizeof(uint32_t);
  for (int i=0; i<nDims; ++i) {
    dims.push_back(dim_data[i]);
  }
  //printf("Complex?: %d\n", complx);
  //printf("Logical?: %d\n", logical);
  //printf("Dimensions: ");
  //for(int n = 0; n < dims.size(); ++n) printf("%d  ", dims[n]);
  //printf("\n");
  //printf("Dim bytes: %d\n", dim_dbytes);

  // get the variable name
  uint32_t name_type, name_dbytes, name_wbytes;
  const char* name_data =
    readVariableTag(name_type, name_dbytes, name_wbytes,
                    data.data() + pre_wbytes + dim_wbytes);
  varName = QByteArray(name_data, name_dbytes);
  //printf("The variable name is: %s\n", &(name[0]));

  // if the encoded data type is a cell array, bail out now
  if (enc_data_type == MAT_CELL_CLASS) {
    QByteArray real(data.data()+pre_wbytes+dim_wbytes+name_wbytes,
                    data.size()-pre_wbytes+dim_wbytes+name_wbytes);
    varData = constructCell(dims, real);
    return true;
  } else if (enc_data_type == MAT_STRUCT_CLASS) {
    QByteArray real(data.data()+pre_wbytes+dim_wbytes+name_wbytes,
                    data.size()-pre_wbytes+dim_wbytes+name_wbytes);
    varData = constructStruct(dims, real);
    return true;
  }

  // get the real data
  uint32_t real_type, real_dbytes, real_wbytes;
  const char* real_data =
    readVariableTag(real_type, real_dbytes,
                    real_wbytes, data.data() + pre_wbytes + dim_wbytes+name_wbytes);
  QByteArray real(real_data, real_dbytes);
  //printf("The variable type is: %d\n", enc_data_type);
  //printf("Total number of bytes in data segment: %d\n", real_dbytes);

  QByteArray imag;
  if (complx) {
    // get the imaginery data
    uint32_t imag_type, imag_dbytes, imag_wbytes;
    const char* imag_data =
      readVariableTag(imag_type, imag_dbytes, imag_wbytes,
                      data.data() + pre_wbytes + dim_wbytes +
                      name_wbytes + real_wbytes);
    assert(imag_type == real_type);
    for ( ; imag_data != imag_data+imag_dbytes; imag_data++)
      imag.push_back(*imag_data);
  }

  // construct whatever object we happened to get
  switch (enc_data_type) {
    // integral types
  case MAT_INT8_CLASS:
    varData = constructMatrix<int8_t>(dims, real, imag, real_type); break;
  case MAT_UINT8_CLASS:
    varData = constructMatrix<uint8_t>(dims, real, imag, real_type); break;
  case MAT_INT16_CLASS:
    varData = constructMatrix<int16_t>(dims, real, imag, real_type); break;
  case MAT_UINT16_CLASS:
    varData = constructMatrix<uint16_t>(dims, real, imag, real_type); break;
  case MAT_INT32_CLASS:
    varData = constructMatrix<int32_t>(dims, real, imag, real_type); break;
  case MAT_UINT32_CLASS:
    varData = constructMatrix<uint32_t>(dims, real, imag, real_type); break;
  case MAT_FLOAT_CLASS:
    varData = constructMatrix<float>(dims, real, imag, real_type); break;
  case MAT_DOUBLE_CLASS:
    varData = constructMatrix<double>(dims, real, imag, real_type); break;
  case MAT_INT64_CLASS:
    varData = constructMatrix<int64_t>(dims, real, imag, real_type); break;
  case MAT_UINT64_CLASS:
    varData = constructMatrix<uint64_t>(dims, real, imag, real_type); break;
  case MAT_CHAR_CLASS:
    varData = QString(real); break;
    // non-handled types
  case MAT_SPARSE_CLASS:
  case MAT_OBJECT_CLASS:
  default:
    return false;
  }

  return true;
}

/*! @brief uncompress a variable
 *
 * If the data type of a variable is MAT_COMPRESSED, then the binary
 * data blob has been compressed using zlib compression. This function
 * uncompresses the blob, then calls readVariable() to interpret the
 * actual data
 *
 * @param data_type the type of the data stored in the binary blob
 * @param dbytes the number of bytes that constitue the binary blob
 * @param wbytes the whole number of bytes that consistute the header,
 * the binary blob, and any padding to 64-bit boundaries
 * @param data the binary blob
 * @return the binary blob, uncompressed
 */

QByteArray MatlabIO::uncompressVariable
(uint32_t& data_type, uint32_t& dbytes, uint32_t& wbytes, QByteArray data)  {

  // inflate the variable header
  QByteArray buf = qUncompress(data);

  // get the headers
  readVariableTag(data_type, dbytes, wbytes, buf);
  
  // convert to a vector
  assert(buf.size() == dbytes + 8);
  QByteArray udata(buf.data() + 8, dbytes);

  return udata;
}

/*! @brief Interpret a variable from a binary block of data
 *
 * This function may be called recursively when either uncompressing
 * data or interpreting fields of a struct or cell array
 *
 * @param data_type the type of the data stored in the binary blob
 * @param nbytes the number of bytes that constitute the binary blob
 * @param data the binary blob @return an interpreted variable
 */

bool MatlabIO::readVariable
(uint32_t data_type, uint32_t nbytes, QByteArray data,
 QString & varName, QVariant & varData) {

  switch (data_type) {
    case MAT_COMPRESSED:
      { // uncompress the data
        uint32_t udata_type;
        uint32_t udbytes;
        uint32_t uwbytes;
        uchar dSize[4] = {static_cast<uchar>(0xff & (nbytes >> 24)),
                          static_cast<uchar>(0xff & (nbytes >> 16)),
                          static_cast<uchar>(0xff & (nbytes >>  8)),
                          static_cast<uchar>(0xff & (nbytes >>  0))};
        QByteArray zData = QByteArray(reinterpret_cast<char*>(dSize), 4) + data;
        QByteArray udata = uncompressVariable(udata_type, udbytes, uwbytes, zData);
        return readVariable(udata_type, udbytes, udata, varName, varData);
      }
    case MAT_MATRIX:
      { // deserialize the matrix
        return collateMatrixFields(data_type, nbytes, data, varName, varData);
      }
    default: break;
    }

  return false;
}

/*! @brief read a block of data from the file being parsed
 *
 * This function attempts to read an entire variable from the file
 * being parsed.  The data block is then encapsulated in a vector and
 * passed onto readVariable() for interpretation. This design means
 * that the file is touched a minimal number of times, and later
 * manipulation of the data can make use of automatic memory
 * management, reference counting, etc.
 *
 * @return the block of data interpreted as a variable and stored in a
 * generic container
 */

bool MatlabIO::readBlock(QString & varName, QVariant & varData) {

  // get the data type and number of bytes consumed
  // by this variable. Check to see if it's using
  // the small data format (seriously, who thought of that? You save
  // at best 8 bytes...)

  uint32_t data_type;
  uint32_t dbytes;
  uint32_t wbytes;
  char buf[8];
  _fid.read(buf, sizeof(char)*8);
  readVariableTag(data_type, dbytes, wbytes, buf);

  // read the binary data block
  //printf("\nReading binary data block...\n"); fflush(stdout);
  QByteArray data(dbytes, 0);
  _fid.read(data.data(), sizeof(char)*data.size());

  // move the seek head position to the next 64-bit boundary
  // (but only if the data is uncompressed. Saving yet another 8 tiny bytes...)
  if (data_type != MAT_COMPRESSED) {
    //printf("Aligning seek head to next 64-bit boundary...\n");
    std::streampos head_pos = _fid.tellg();
    int padding = head_pos % 8;
    _fid.seekg(padding, std::fstream::cur);
  }

  // now read the variable contained in the block
  return readVariable(data_type, dbytes, data, varName, varData);
}

/*! @brief Read all variables from a file
 *
 * Reads every variable encountered when parsing a valid Matlab .Mat file.
 * If any of the variables is a function pointer, or other Matlab specific
 * object, it will be passed. Most integral types will be parsed successfully.
 * Matlab matrices will be converted to OpenCV matrices of the same type.
 * Note: Matlab stores images in RGB format whereas OpenCV stores images in
 * BGR format, so if displaying a parsed image using cv::imshow(), the
 * colours will be inverted.
 * @return a vector of containers storing the name and data of each variable
 * in the file
 */

QMap<QString,QVariant> MatlabIO::read(void) {
    std::cerr << "QMap<QString,QVariant> MatlabIO::read(void) {\n";
    
  // allocate the output
  QMap<QString,QVariant> varMap;

  // read the header information
  getHeader();

  // get all of the variables
  while(hasVariable()) {
    QString varName;
    QVariant varData;
    if (readBlock(varName, varData)) {
      // std::cerr << "  varName = \"" << qPrintable(varName) << "\"\n";
      varMap.insert(varName, varData);
    }
  }

  std::cerr << "}\n";
    
  return varMap;
}

/*! @brief Print a formatted list of the contents of a file
 *
 * Similar to the 'whos' function in matlab, this function prints to stdout
 * a list of variables and their C++ datatypes stored in the associated .Mat file
 * @param variables the variables read from the .Mat file using the read() function
 */

void MatlabIO::whos(QMap<QString,QVariant> const& varMap) const {
  // get the longest filename
  size_t flmax = 0;
  auto nameList = varMap.keys();
  foreach(auto name, nameList) { 
    if (name.length() > flmax) {
      flmax = name.length();
    }
  }
  
  printf("-------------------------\n");
  printf("File: %s\n", qPrintable(_filename));
  printf("%s\n", header_);
  printf("Variables:\n");
  foreach(auto name, nameList) {

    // Tue Mar  8 22:34:27 2016
    // modified by Gabriel Taubin to make it print the proper type
    
    QVariant::Type type = varMap[name].type();
    QString typeStr = "";
    switch(type) {
    case QVariant::Invalid:
      typeStr = "Invalid";
      break;
    case QVariant::Bool:
      typeStr = "Bool";
      break;
    case QVariant::Int:
      typeStr = "Int";
      break;
    case QVariant::UInt:
      typeStr = "UInt";
      break;
    case QVariant::LongLong:
      typeStr = "LongLong";
      break;
    case QVariant::ULongLong:
      typeStr = "ULongLong";
      break;
    case QVariant::Double:
      typeStr = "Double";
      break;
    case QVariant::Char:
      typeStr = "Char";
      break;
    case QVariant::Map:
      typeStr = "Map";
      break;
    case QVariant::List:
      typeStr = "List";
      break;
    case QVariant::String:
      typeStr = "String";
      break;
    case QVariant::StringList:
      typeStr = "StringList";
      break;
    case QVariant::ByteArray:
      typeStr = "ByteArray";
      break;
    case QVariant::BitArray:
      typeStr = "BitArray";
      break;
    case QVariant::Date:
      typeStr = "Date";
      break;
    case QVariant::Time:
      typeStr = "Time";
      break;
    case QVariant::DateTime:
      typeStr = "DateTime";
      break;
    case QVariant::Url:
      typeStr = "Url";
      break;
    case QVariant::Locale:
      typeStr = "Locale";
      break;
    case QVariant::Rect:
      typeStr = "Rect";
      break;
    case QVariant::RectF:
      typeStr = "RectF";
      break;
    case QVariant::Size:
      typeStr = "Size";
      break;
    case QVariant::SizeF:
      typeStr = "SizeF";
      break;
    case QVariant::Line:
      typeStr = "Line";
      break;
    case QVariant::LineF:
      typeStr = "LineF";
      break;
    case QVariant::Point:
      typeStr = "Point";
      break;
    case QVariant::PointF:
      typeStr = "PointF";
      break;
    case QVariant::RegExp:
      typeStr = "RegExp";
      break;
    case QVariant::RegularExpression:
      typeStr = "RegularExpression";
      break;
    case QVariant::Hash:
      typeStr = "Hash";
      break;
    case QVariant::EasingCurve:
      typeStr = "EasingCurve";
      break;
    case QVariant::Uuid:
      typeStr = "Uuid";
      break;
    case QVariant::ModelIndex:
      typeStr = "ModelIndex";
      break;
    case QVariant::PersistentModelIndex:
      typeStr = "PersistentModelIndex";
      break;
 // case QVariant::LastCoreType:
 //   break;
    case QVariant::Font:
      typeStr = "Font";
      break;
    case QVariant::Pixmap:
      typeStr = "Pixmap";
      break;
    case QVariant::Brush:
      typeStr = "Brush";
      break;
    case QVariant::Color:
      typeStr = "Color";
      break;
    case QVariant::Palette:
      typeStr = "Palette";
      break;
    case QVariant::Image:
      typeStr = "Image";
      break;
    case QVariant::Polygon:
      typeStr = "Polygon";
      break;
    case QVariant::Region:
      typeStr = "Region";
      break;
    case QVariant::Bitmap:
      typeStr = "Bitmap";
      break;
    case QVariant::Cursor:
      typeStr = "Cursor";
      break;
    case QVariant::KeySequence:
      typeStr = "KeySequence";
      break;
    case QVariant::Pen:
      typeStr = "Pen";
      break;
    case QVariant::TextLength:
      typeStr = "TextLength";
      break;
    case QVariant::TextFormat:
      typeStr = "TextFormat";
      break;
    case QVariant::Matrix:
      typeStr = "Matrix";
      break;
    case QVariant::Transform:
      typeStr = "Transform";
      break;
    case QVariant::Matrix4x4:
      typeStr = "Matrix4x4";
      break;
    case QVariant::Vector2D:
      typeStr = "Vector2D";
      break;
    case QVariant::Vector3D:
      typeStr = "Vector3D";
      break;
    case QVariant::Vector4D:
      typeStr = "Vector4D";
      break;
    case QVariant::Quaternion:
      typeStr = "Quaternion";
      break;
    case QVariant::PolygonF:
      typeStr = "PolygonF";
      break;
    case QVariant::Icon:
      typeStr = "Icon";
      break;
 // case QVariant::LastGuiType:
 //   break;
    case QVariant::SizePolicy:
      typeStr = "SizePolicy";
      break;
    case QVariant::UserType:
      typeStr = "UserType";
      break;
    case QVariant::LastType:
      typeStr = "LastType";
      break;
    }

    printf("%*s:  %s\n", static_cast<int>(flmax), qPrintable(name),
           qPrintable(typeStr));
  }
  printf("-------------------------\n");
  fflush(stdout);
}
