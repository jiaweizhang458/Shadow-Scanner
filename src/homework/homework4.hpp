#ifndef _homework4_hpp_
#define _homework4_hpp_

#include <QSize>
#include <QImage>
#include <QVector>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <app/matrix_util.hpp>
#include <app/CalibrationDataCamera.hpp>
#include <app/TurntableCalib.hpp>

namespace hw4 {

  int detectChessboardCorners
  (// inputs
   cv::Mat1b image,
   cv::Size2i cornerCount,
   cv::Size2f cornerSize,
   // outputs
   std::vector<cv::Point3f> & cornersWorld,
   std::vector<cv::Point2f> & cornersCamera);

  double calibrateCamera
  (// inputs
   std::vector<std::vector<cv::Point3f> > & worldPoints,
   std::vector<std::vector<cv::Point2f> > & imagePoints,
   cv::Size2i & imageSize,
   // outputs
   cv::Mat & K,
   cv::Mat & kc, 
   std::vector<cv::Mat> & R_vecs,
   std::vector<cv::Mat> & T_vecs);

  void calibrateTurntable
  (// inputs
   cv::Mat & K,   // camera intrinsic parameters : camera matrix
   cv::Mat & kc,  // camera intrinsic parameters : lens distortion
   std::vector<std::vector<cv::Point3f> > & worldPoints,
   std::vector<std::vector<cv::Point2f> > & imagePoints,
   // outputs
   Vector2d& C, // center of rotation in chessboard coordinates
   Matrix3d& R_world, // turntable coordinate system rotation
   Vector3d& T_world  // turntable coordinate system rotation
   );

  void calibrateLaserPlane
  (// inputs
   cv::Mat & K,
   cv::Mat & kc,
   QVector<QString> const& laserImageFileName,
   cv::Size2i cornerCount,
   cv::Size2f cornerSize,
   // outputs
   Vector4d& laserPlane,
   Vector4d& laserPlane2);
  
  void scan
  (// inputs
   Matrix3d const&    K,
   Vector5d const&    kc,
   Matrix3d const&    worldRotation,
   Vector3d const&    worldTranslation,
   float const        angle,
   QImage const &     backgroundImage,
   QImage const &     foregroundImage,
   Vector4d const&    laserPlane,
   // outputs
   QVector<Vector3d>& worldCoord);

};

#endif //_homework4_hpp_
