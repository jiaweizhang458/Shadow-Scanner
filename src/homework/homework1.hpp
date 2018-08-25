
#ifndef _homework1_hpp_
#define _homework1_hpp_

#include <QImage>
#include <QString>

namespace hw1
{
  QString getStudentName(void);
  QImage filterImage(QImage const& inputImage, QVector<int> filter);
  QImage detectLaser(QImage const& inputImage);
  QImage detectShadow(QImage const& inputImage, QImage const& inputImage1);
  QImage ComputeMaxIllumination(QImage const& inputImage1, QImage const& inputImage2);
  QImage turnGray(QImage const& inputImage);
};

#endif //_homework1_hpp_
