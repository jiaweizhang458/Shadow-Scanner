
#include "homework2.hpp"
#include "homework1.hpp"
#include <iostream>

//homework functions

double hw2::reprojectPoints(Matrix3d const& K, Vector5d const& kc, 
                            Matrix3d const& R, Vector3d const& T,
                            QVector<Vector3d> const& worldPoints, 
                            QVector<Vector2d> const& imagePoints, 
                            QVector<Vector2d> & reprojectedPoints)
{
  //Description:
  //  hw2::reprojectPoints() receives camera intrisics (K,kc) and
  //  extrinsics (R,T) parameters, as well as, the 3D coordinates of the 
  //  checkerboard coordinates (worlPoints). Your task is to use these
  //  parameters to reproject each of the 3D points onto the image and 
  //  save the reprojected 2D points in the output 'reprojectedPoints'.
  //  In addition, you must compare your reprojected points with the 
  //  original 2D image points, given in 'imagePoints' and compute 
  //  the Root Mean Square Error (RMSE). The RMSE is the return value 
  //  of this function.

  //We use Eigen Lib for matrix and vector operations
  // check this for help: http://eigen.tuxfamily.org/dox/group__QuickRefPage.html
  
  // Check the Camera Calibration Toolbox webpage to learn how to project 
  //  a 3D point, including lens distortion:
  // http://www.vision.caltech.edu/bouguetj/calib_doc/htmls/parameters.html

  double rmse = 0.0;
  auto nPoints = worldPoints.size();
  for (int i=0; i<nPoints; ++i)
  {
    auto const& p = worldPoints[i];

    //project 'p' to the image plane
	Vector3d XXc = R * p + T;
	Vector2d u(XXc[0] / XXc[2], XXc[1] / XXc[2]);

    //apply lens distortion to 'u'
	double r2 = (u[0]*u[0]) + (u[1]*u[1]);
	Vector2d dx(2 * kc[3] * u[0] * u[1] + kc[4] * (r2 + 2 * (u[0] * u[0])), kc[3] * (r2 + 2 * u[1] * u[1]) + 2 * kc[4] * u[0] * u[1]);
	u = (1 + kc[0] * r2 + kc[1] * (r2*r2) + kc[4] * r2*r2*r2)*u + dx;

    //convert 'u' to pixel coords
	u[0] = K(0, 0)*u[0] + K(0, 1)*u[1] + K(0, 2) * 1;
	u[1] = K(1, 1)*u[1] + K(1, 2) * 1;

    //save result
    reprojectedPoints.push_back(u);

    //compare with the original image point 'v'
    auto const& v = imagePoints[i];
	rmse += (u[0] - v[0])*(u[0] - v[0]) + (u[1] - v[1])*(u[1] - v[1]);
    
  }

  //RMSE
  rmse = std::sqrt(rmse/(2*nPoints));

  return rmse;
}

QVector<Vector3d> hw2::triangulate(Matrix3d const& K, Vector5d const& kc, 
                                   Matrix3d const& R, Vector3d const& T,
                                   Vector4d const& laserPlane, double turntableAngle,
                                   QImage image)
{
  //Description:
  // hw2::triangulate() receives the Camera and Turntable calibration as input parameters,
  // together with an image. The function will call hw1::detectLaser() to find a vertical
  // laser line, and it will triangulate using plane-ray intersection each of the laser
  // line pixels. The function returns a vector of 3D points.

  //Coordinate systems:
  // The world coordinate system is at the center of the turntable, with the xy-plane on 
  // the turntable and z-axis upwards. The camera coordinate system is located at the 
  // center of projection and following the camera orientation as usual.
  // At each image the turntable was rotated by 'turntableAngle' radians, thus, after
  // triangulation each 3D point must be rotated backwards so that points are located
  // at the correct position in the unrotated scanning target. 

  //Laser Light Plane:
  // The laser plane is given as a 4-vector in the camera coordinate systems. This is 
  // convenient because we can calculate the intersection point of the camera ray with 
  // this plane directly in camera coords and later transform the point to world coords.
  // If we call the plane 4-vector 'n', then a 3D point in homogeneous coordinates 
  // p = (X,Y,Z,1) belongs to the plane if and only if n.transpose()*p = 0.
  
  //Camera Lens Distortion:
  // The model commonly used to compensate for lens distortion does not have a closed 
  // form inverse. We will ignore the lens distortion here to simplify the task.
  Q_UNUSED(kc)

  //detect laser
  QImage detectionImage = hw1::detectLaser(image);

  //output vector of triangulated 3D points
  QVector<Vector3d> points;

  //triangulate
  for (int h=0; h<detectionImage.height(); ++h)
  {
    for (int w=0; w<detectionImage.width(); ++w)
    {
      auto pixel = detectionImage.pixel(w, h);
      int value = qRed(pixel);
      if (!value) { /* skip */ continue; }

      //transform from pixel to camera coordinates
      Vector3d u(w, h, 1);

      //ray-plane intersection (NO RADIAL DISTORTION)
	  //pW = R^t*(K^{-1}*(\lambda*u) - T)
	  //Vector3d a = R.transpose()*K.inverse()*u;//R^{-1}*K^{-1}*u
	  //Vector3d b = R.transpose()*T;//R^{-1}*T
	  //double lambda = (laserPlane[0] * b[0] + laserPlane[1] * b[1] + laserPlane[2] * b[2] - laserPlane[3]) / (laserPlane[0] * a[0] + laserPlane[1] * a[1] + laserPlane[2] * a[2]);
	  Vector3d a = K.inverse()*u;
	  double lambda = -laserPlane[3] / (laserPlane[0] * a[0] + laserPlane[1] * a[1] + laserPlane[2] * a[2]);
	  //lambda *= -1;
	  lambda = abs(lambda);
	  
	  Vector3d v = a*lambda;

	  //transform from camera to turntable coords
	  Vector3d p = R.inverse()*(v - T);
	  
      //undo the turntable location to place the point in world coords
	  Matrix3d Ralpha;
	  Ralpha(0, 0) = cos(turntableAngle);
	  Ralpha(0, 1) = -sin(turntableAngle);
	  Ralpha(0, 2) = 0;
	  Ralpha(1, 0) = sin(turntableAngle);
	  Ralpha(1, 1) = cos(turntableAngle);
	  Ralpha(1, 2) = 0;
	  Ralpha(2, 0) = 0;
	  Ralpha(2, 1) = 0;
	  Ralpha(2, 2) = 1;
	  //p = Ralpha.inverse()*p;
	  p = Ralpha*p;

      //save the point
	  if (abs(p[0])*abs(p[0]) + abs(p[1])*abs(p[1]) <= 8100 && abs(p[2]) <= 100)
	  {
		 points.push_back(p);
	  }
	  //points.push_back(p);

    }//for cols
  }//for rows

  return points;
}