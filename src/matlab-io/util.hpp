
#ifndef _matab_io_util_hpp_
#define _matab_io_util_hpp_

#include <QString>
#include <QVector>
#include <QVariantMap>

#include "../app/matrix_util.hpp"

typedef QVector<QVector<QVector<double>>> MatlabMatrixType;

namespace matlab
{
  enum MatrixType { ColMajor, RowMajor };

  bool get2DMatrixVector2d
  (MatlabMatrixType & value, QVector<Vector2d>& data , int rows);

  bool get2DMatrixDouble
  (MatlabMatrixType & value,
   double * data, int rows, int cols, MatrixType type = RowMajor);

  bool get2DMatrixDouble
  (QString name, QVariantMap const& varMap,
   double * data, int rows, int cols, MatrixType type = RowMajor);

};

#endif //_matab_io_util_hpp_
