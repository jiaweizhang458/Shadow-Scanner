#include "util.hpp"

bool matlab::get2DMatrixVector2d
(MatlabMatrixType & value, QVector<Vector2d>& data , int rows) {

  if (value.size() != 1) return false;
  auto & mat = value[0];
  if (mat.size() != 2) return false;
  auto & col0 = mat[0];
  if (col0.size() != rows) return false;
  auto & col1 = mat[1];
  if (col1.size() != rows) return false;

  data.clear();
  for (int i = 0; i < rows; ++i) {
    Vector2d v2(col0[i], col1[i]);
    data.push_back(v2);
  }
  return true;
}


bool matlab::get2DMatrixDouble
(MatlabMatrixType & value,
 double * data, int rows, int cols, MatrixType type) {

  if (value.size() != 1) {
    return false;
  }

  auto & mat = value[0];
  if (mat.size() != cols) {
    return false;
  }

  for (int w = 0; w < cols; ++w) {
    auto & col = mat[w];
    if (col.size() != rows) {
      return false;
    }
    for (int h = 0; h < rows; ++h) {
      auto index = (type == RowMajor ? h*cols + w : w*rows + h);
      data[index] = col[h];
    }
  }
  
  return true;
}

bool matlab::get2DMatrixDouble
(QString name, QVariantMap const& varMap,
 double * data, int rows, int cols, MatrixType type) {

  if (name.isEmpty() || !varMap.contains(name)) {
    return false;
  }

  MatlabMatrixType value = varMap[name].value<MatlabMatrixType>();

  return get2DMatrixDouble(value,data,rows,cols,type);
}

