
#ifndef _homework2_hpp_
#define _homework2_hpp_

#include <QSize>
#include <QImage>
#include <QVector>

#include <app/matrix_util.hpp>
#include <app/CameraCalib.hpp>
#include <app/TurntableCalib.hpp>

namespace hw2
{
  double reprojectPoints(Matrix3d const& K, Vector5d const& kc, 
                          Matrix3d const& R, Vector3d const& T,
                          QVector<Vector3d> const& worldPoints, 
                          QVector<Vector2d> const& imagePoints, 
                          QVector<Vector2d> & reprojectedPoints);

  QVector<Vector3d> triangulate(Matrix3d const& K, Vector5d const& kc, 
                                Matrix3d const& R, Vector3d const& T,
                                Vector4d const& laserPlane, double turntableAngle,
                                QImage image);
};

#endif //_homework2_hpp_
