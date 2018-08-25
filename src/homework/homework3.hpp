#ifndef _homework3_hpp_
#define _homework3_hpp_

#include <QSize>
#include <QImage>
#include <QVector>

#include <app/matrix_util.hpp>
#include <app/CameraCalib.hpp>
#include <app/TurntableCalib.hpp>

namespace hw3
{
  // Turntable Calibration

  void estimateExtrinsics(Matrix3d const& K, /* intrinsic */
                          Vector5d const& kc, /* intrinsic */
                          QVector<Vector2d> const& worldCoord, /* input */
                          QVector<Vector2d> const& imageCoord, /* input */
                          Matrix3d& R, /* output */
                          Vector3d& T  /* output */);

  void estimateTurntableCenter(QVector<Matrix3d> const& R, /* input */
                               QVector<Vector3d> const& T, /* input */
                               Vector2d& centerCoord, /* output */
                               Matrix3d& worldRotation, /* output */
                               Vector3d& worldTranslation /* output */);

  double reprojectPoints(Matrix3d const& K, /* input */
                         Vector5d const& kc, /* input */
                         Matrix3d const& R, /* input */
                         Vector3d const& T, /* input */
                         QVector<Vector2d> const& checkerboardPoints, /* input */
                         QVector<Vector2d> const& imagePoints, /* input */
                         QVector<Vector2d> & reprojectedPoints, /* output */
                         Matrix3d const& R_world, /* input */
                         Vector3d& T_world, /* input */
                         QVector<Vector3d> & worldPoints /* output */ );

  // Laser Plane Calibration

  void detectLaser(QImage const& inputImage, /* input */
                   QVector<Vector2d>& laserLineCoord /* output */ );

  void detectShadow(QImage const& inputImage, QImage const& inputImage1,
	                QVector<Vector2d>& laserLineCoord, /* output */
	                QVector<Vector2d>& laserLineCoord1 /* output */);

  QImage turnGray(QImage const& inputImage);

  QImage ComputeMaxIllumination(QImage const& inputImage1 /*RGB*/, 
	                            QImage const & inputImage2 /*RGB*/);

  void checkerboardPlane(Matrix3d& R, /* input */
                         Vector3d& T, /* input */
                         Vector4d & checkerboardPlane /* output */);

  void triangulate(Matrix3d const& K, /* input */
                   Vector5d const& kc, /* input */
                   Vector4d& planeCoefficients,
                   QVector<Vector2d> const& imagePoints, /* input */
                   QVector<Vector3d> & worldPoints /* output */ );
  
  void estimatePlane(QVector<Vector3d> & laserPoints, /* input */
                     Vector4d & laserPlane /* output */);
};

#endif //_homework3_hpp_
