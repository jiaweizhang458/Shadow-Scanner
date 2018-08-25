#include <iostream>
#include <fstream>
#include "homework1.hpp"
#include "homework2.hpp"
#include "homework3.hpp"
#include "homework4.hpp"
#include <app/matrix_util.hpp>
#include <Eigen/Core>
#include <opencv2/core/eigen.hpp>
#include <QDir>

// install OpenCV 3.4 in your computer and make sure that CMake knows
// how to find it

//////////////////////////////////////////////////////////////////////
int hw4::detectChessboardCorners
(/*inputs*/
 cv::Mat1b imageGray,
 cv::Size2i cornerCount, // (cols,rows)
 cv::Size2f cornerSize,  // (width,height)
 /* outputs */
 std::vector<cv::Point3f> & cornersWorld,
 std::vector<cv::Point2f> & cornersCamera) 
{

	// should return number of corners detected if successful; 0 otherwise

	// 1) clear the output arrays
	cornersWorld.clear();
	cornersCamera.clear();

	// 2) you may want to check image size and scale it down using
	// cv::resize() if too large to speed up process

	// 3) detect chessboard corners using cv::findChessboardCorners()
	bool patternfound = cv::findChessboardCorners(imageGray, cornerCount, cornersCamera);
	// 4) if no error while detecting chessboard corners {
	if (patternfound == true)
	{
		// 5) generate chessboard coordinates array cornersWorld from the
		// information stored in cornerCount and cornerSize
		for (int i = 0; i < cornerCount.height; i++)
		{
			for (int j = 0; j < cornerCount.width; j++)
			{
				cornersWorld.push_back(cv::Point3f((float)j*cornerSize.width, (float)i*cornerSize.height, 0.0));
			}
		}

		// 6) if you had scaled down the input image, adjust camera corners
		// to original scale

		// 7) refine camera corners at subpixel resolution with respect to original image
		// using cv::cornerSubPix()
		cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 40, 0.01);
		cv::cornerSubPix(imageGray, cornersCamera, cv::Size(5, 5), cv::Size(-1, -1), criteria);
	// }
	}

	return cornersCamera.size();
}

//////////////////////////////////////////////////////////////////////
double hw4::calibrateCamera
( // inputs
 std::vector<std::vector<cv::Point3f> > & worldPoints,
 std::vector<std::vector<cv::Point2f> > & imagePoints,
 cv::Size2i & imageSize,
 // outputs
 cv::Mat & K,
 cv::Mat & kc, 
 std::vector<cv::Mat> & R_vecs,
 std::vector<cv::Mat> & T_vecs) 
{

	double camError = -1.0;

	// calibrate camera using OpenCV cv::calibrateCamera()
	// if successful, return the calibration error reported by 
	camError = cv::calibrateCamera(worldPoints, imagePoints, imageSize, K, kc, R_vecs, T_vecs);
	//kc.at<double>(0, 4) = 0;
	return camError;
}

//////////////////////////////////////////////////////////////////////
void hw4::calibrateTurntable
(// inputs
 cv::Mat & K,   // camera intrinsic parameters : camera matrix
 cv::Mat & kc,  // camera intrinsic parameters : lens distortion
 std::vector<std::vector<cv::Point3f> > & worldPoints,
 std::vector<std::vector<cv::Point2f> > & imagePoints,
 // outputs
 Vector2d& C, // center of rotation in chessboard coordinates
 Matrix3d& R_world, // turntable coordinate system rotation
 Vector3d& T_world  // turntable coordinate system translation
 ) 
{

	//// 1) find an OpenCV function to perform pose estimation or write your own
	Matrix3d Eigen_K;
	cv::cv2eigen(K, Eigen_K);
	//Vector5d Eigen_kc;
	//Eigen_kc[0] = kc.at<double>(0, 0);
	//Eigen_kc[1] = kc.at<double>(0, 1);
	//Eigen_kc[2] = kc.at<double>(0, 2);
	//Eigen_kc[3] = kc.at<double>(0, 3);
	//Eigen_kc[4] = kc.at<double>(0, 4);
	//// 2) for each of the (worldPoints,imagePoints) pairs {
	//QVector<Matrix3d> R_vector;
	//QVector<Vector3d> T_vector;
	//for (int i = 0; i < worldPoints.size(); i++)
	//{
	//	// 3) perform pose estimation
	//	QVector<Vector2d> Q_worldPoints;
	//	QVector<Vector2d> Q_imagePoints;
	//	for (int j = 0; j < worldPoints[i].size(); j++)
	//	{
	//		Q_worldPoints.push_back(Vector2d(worldPoints[i][j].x, worldPoints[i][j].y));
	//		Q_imagePoints.push_back(Vector2d(imagePoints[i][j].x, imagePoints[i][j].y));
	//	}
	//	Matrix3d Rtemp;
	//	Vector3d Ttemp;
	//	hw3::estimateExtrinsics(Eigen_K, Eigen_kc, Q_worldPoints, Q_imagePoints, Rtemp, Ttemp);
	//	R_vector.push_back(Rtemp);
	//	T_vector.push_back(Ttemp);
	//// }
	//}

	QVector<Matrix3d> R_vector;
	QVector<Vector3d> T_vector;
	for (int i = 0; i < worldPoints.size(); i++)
	{
		cv::Mat R;
		cv::Mat T;
		cv::solvePnP(worldPoints[i], imagePoints[i], K, kc, R, T);
		cv::Mat Rm;
		cv::Rodrigues(R, Rm);
		Matrix3d Eigen_R;
		cv::cv2eigen(Rm, Eigen_R);
		Vector3d Eigen_T;
		Eigen_T[0] = T.at<double>(0, 0);
		Eigen_T[1] = T.at<double>(1, 0);
		Eigen_T[2] = T.at<double>(2, 0);
		R_vector.push_back(Eigen_R);
		T_vector.push_back(Eigen_T);
	}

	// 4) from all the rotations and translations compute the center of
	// rotation C in chessboard coordinates (it has the same coordinates
	// in all the chessboard coordinate systems).

	// 5) using measurements from the turntable calibration chessboard,
	// determine the projection T_world of the center of rotation C onto
	// the turntable plane

	// 6) determine the rotation matrix R_world
	hw3::estimateTurntableCenter(R_vector, T_vector, C, R_world, T_world);
	std::cout << Eigen_K*T_world/(Eigen_K*T_world)[2] << std::endl;
	std::cout << Eigen_K*(25 * R_world.col(0) + T_world) / (Eigen_K*T_world)[2] << std::endl;
	std::cout << Eigen_K*(25 * R_world.col(1) + T_world) / (Eigen_K*T_world)[2] << std::endl;
	std::cout << Eigen_K*(25 * R_world.col(2) + T_world) / (Eigen_K*T_world)[2] << std::endl;
}

//////////////////////////////////////////////////////////////////////
void hw4::calibrateLaserPlane
(// inputs
 cv::Mat & K,
 cv::Mat & kc,
 QVector<QString> const& laserImageFileName,
 cv::Size2i cornerCount,
 cv::Size2f cornerSize,
 // outputs
 Vector4d& laserPlane,
 Vector4d& laserPlane2) {
  
  std::cerr << "Project: calibrateShadowPlane() {\n";
  
  //When calibrating shadow plane, each shadow edge needs two image.
  //Therefore there has to be even number of images.
  assert(laserImageFileName.size() % 2 == 0);
  // for set of each image :
  // 1) convert to cv::Mat1b format and detect chessboard corners 
  
  int nImages = laserImageFileName.size() / 2; //set of image

  //std::vector<cv::Point3f> laserLinePoint;
  QVector<Vector3d> laserLinePoint;
  QVector<Vector3d> laserLinePoint1;

  for(int i=0;i<nImages;i++) {
	QString  fileName = laserImageFileName[2 * i];
	QString  fileName1 = laserImageFileName[2 * i + 1];
	//QDir workDir(getApp()->getMainWindow()->getWorkDirectory());
	//fileName = workDir.absolutePath();
	std::cerr << " Image set: " << i << std::endl;
    std::cerr << " img["<<2 * i<<"] = \"" << fileName.toUtf8().constData() << "\"\n";
	std::cerr << " img[" << 2 * i + 1 << "] = \"" << fileName.toUtf8().constData() << "\"\n";
    // load the image
    QImage qImg;
	QImage qImg1;
    if (qImg.load(fileName) && qImg1.load(fileName1)) {

      // 1) convert to cv::Mat1b format and use
      // hw4::detectChessboardCorners to detect the chessboard
      std::vector<cv::Point3f> cornersWorld;
	  cv::Mat1b cvImg(qImg.height(), qImg.width());
	  QImage Max_qImg = hw3::ComputeMaxIllumination(qImg, qImg1);
	  for (int j = 0; j < qImg.height(); j++)
	  {
		  for (int k = 0; k < qImg.width(); k++)
		  {
			  cvImg.at<uchar>(j, k) = (uchar)(qGray(Max_qImg.pixel(k, j)));
		  }
	  }

      // 2) fill the array of corner coordinates
      std::vector<cv::Point2f> cornersCamera;
	  hw4::detectChessboardCorners(cvImg, cornerCount, cornerSize, cornersWorld, cornersCamera);

      // 3) estimate the pose of the camera with respect to the chessboard (R,T)
      /*cv::Mat R(3,3,CV_64F);
      cv::Mat T(3,1,CV_64F);*///Replaced by Eigen_R and Eigen_T

	  Matrix3d Eigen_K;
	  Eigen_K(0, 0) = K.at<double>(0, 0);
	  Eigen_K(0, 1) = K.at<double>(0, 1);
	  Eigen_K(0, 2) = K.at<double>(0, 2);
	  Eigen_K(1, 0) = K.at<double>(1, 0);
	  Eigen_K(1, 1) = K.at<double>(1, 1);
	  Eigen_K(1, 2) = K.at<double>(1, 2);
	  Eigen_K(2, 0) = K.at<double>(2, 0);
	  Eigen_K(2, 1) = K.at<double>(2, 1);
	  Eigen_K(2, 2) = K.at<double>(2, 2);
	  Vector5d Eigen_kc;
	  Eigen_kc[0] = kc.at<double>(0, 0);
	  Eigen_kc[1] = kc.at<double>(0, 1);
	  Eigen_kc[2] = kc.at<double>(0, 2);
	  Eigen_kc[3] = kc.at<double>(0, 3);
	  Eigen_kc[4] = kc.at<double>(0, 4);
	  //QVector<Vector2d> Q_cornersWorld;
	  //QVector<Vector2d> Q_cornersCamera;
	  //Matrix3d Eigen_R;
	  //Vector3d Eigen_T;
	  //for (int k = 0; k < cornersWorld.size(); k++)
	  //{
		 // Q_cornersWorld.push_back(Vector2d(cornersWorld[k].x, cornersWorld[k].y));
		 // Q_cornersCamera.push_back(Vector2d(cornersCamera[k].x, cornersCamera[k].y));
	  //}
	  //hw3::estimateExtrinsics(Eigen_K, Eigen_kc, Q_cornersWorld, Q_cornersCamera, Eigen_R, Eigen_T);

	  cv::Mat R;
	  cv::Mat T;
	  cv::solvePnP(cornersWorld, cornersCamera, K, kc, R, T);
	  cv::Mat Rm;
	  cv::Rodrigues(R, Rm);
	  Matrix3d Eigen_R;
	  cv::cv2eigen(Rm, Eigen_R);
	  Vector3d Eigen_T;
	  Eigen_T[0] = T.at<double>(0, 0);
	  Eigen_T[1] = T.at<double>(1, 0);
	  Eigen_T[2] = T.at<double>(2, 0);

      //Corner Contour
	  std::vector<std::vector<cv::Point>>contour;
	  std::vector<cv::Vec4i> hierarchy;
	  cv::Mat1b cvCorner = cv::Mat::zeros(qImg.height(), qImg.width(), CV_8UC1);

	  int upperleft = 0;
	  int upperright = cornerCount.width - 1;
	  int lowerleft = (cornerCount.width)*(cornerCount.height - 1);
	  int lowerright = cornerCount.width*cornerCount.height - 1;
	  cvCorner.at<uchar>(cornersCamera[upperleft].y, cornersCamera[upperleft].x) = 255;
	  cvCorner.at<uchar>(cornersCamera[upperright].y, cornersCamera[upperright].x) = 255;
	  cvCorner.at<uchar>(cornersCamera[lowerleft].y, cornersCamera[lowerleft].x) = 255;
	  cvCorner.at<uchar>(cornersCamera[lowerright].y, cornersCamera[lowerright].x) = 255;
	  cv::line(cvCorner, cv::Point(cornersCamera[upperleft].x, cornersCamera[upperleft].y), cv::Point(cornersCamera[upperright].x, cornersCamera[upperright].y), cv::Scalar(255));
	  cv::line(cvCorner, cv::Point(cornersCamera[upperright].x, cornersCamera[upperright].y), cv::Point(cornersCamera[lowerright].x, cornersCamera[lowerright].y), cv::Scalar(255));
	  cv::line(cvCorner, cv::Point(cornersCamera[lowerright].x, cornersCamera[lowerright].y), cv::Point(cornersCamera[lowerleft].x, cornersCamera[lowerleft].y), cv::Scalar(255));
	  cv::line(cvCorner, cv::Point(cornersCamera[upperleft].x, cornersCamera[upperleft].y), cv::Point(cornersCamera[lowerleft].x, cornersCamera[lowerleft].y), cv::Scalar(255));
	  cv::findContours(cvCorner, contour, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	  //cv::imwrite("corner.png", cvCorner);
	  //std::cout << contour.size() << std::endl;
	  /*cv::Mat1b drawing(qImg.height(), qImg.width(), 255);
	  drawContours(drawing, contour, 0, 0, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point());
	  cv::imwrite("hull.png", drawing);*/

	  // 4) detect laser line points in this image
	  QVector<Vector2d> laserLineCoord;
	  QVector<Vector2d> laserLineCoord1;
	  hw3::detectShadow(qImg, qImg1, laserLineCoord, laserLineCoord1);

	  //Remove the points outside the contour (checkerboard)
	  cv::Mat1b LP(qImg.height(), qImg.width(), 255);
	  QVector<Vector2d>::iterator it = laserLineCoord.begin();
	  while (it != laserLineCoord.end())
	  {
		  if (cv::pointPolygonTest(contour[0], cv::Point2f((*it).x(), (*it).y()), false) >= 0)
		  {
			  LP.at<uchar>((*it)[1], (*it)[0]) = 0;
			  it++;
		  }
		  else { it = laserLineCoord.erase(it); }
	  }
	  /*std::ostringstream name;
	  name << "Shadow1_" << i << ".png";
	  cv::imwrite(name.str(), LP);*/

	  cv::Mat1b LP1(qImg1.height(), qImg1.width(), 255);
	  QVector<Vector2d>::iterator it1 = laserLineCoord1.begin();
	  while (it1 != laserLineCoord1.end())
	  {
		  if (cv::pointPolygonTest(contour[0], cv::Point2f((*it1).x(), (*it1).y()), false) >= 0)
		  {
			  LP1.at<uchar>((*it1)[1], (*it1)[0]) = 0;
			  it1++;
		  }
		  else { it1 = laserLineCoord1.erase(it1); }
	  }
	  /*std::ostringstream name;
	  name << "Shadow2_" << i << ".png";
	  cv::imwrite(name.str(), LP1);*/

	  Vector4d cbPlane;
	  hw3::checkerboardPlane(Eigen_R, Eigen_T, cbPlane);

      // 5) convert the detected points to the 3D camera coordinate system
      // 6) save the reconstructed 3D points in the laserLinePoint array
	  
	  //PLane 1
	  hw3::triangulate(Eigen_K, Eigen_kc, cbPlane, laserLineCoord, laserLinePoint);
	  std::ofstream fout;
	  fout.open("output.txt");
	  for (int l = 0; l < laserLinePoint.size(); l++)
	  {
		  fout << laserLinePoint[l][0] << " " << laserLinePoint[l][1] << " " << laserLinePoint[l][2] << std::endl;
	  }
	  fout.close();
	  //Plane 2
	  hw3::triangulate(Eigen_K, Eigen_kc, cbPlane, laserLineCoord1, laserLinePoint1);
	  std::ofstream fout1;
	  fout1.open("output1.txt");
	  for (int ll = 0; ll < laserLinePoint1.size(); ll++)
	  {
		  fout1 << laserLinePoint1[ll][0] << " " << laserLinePoint1[ll][1] << " " << laserLinePoint1[ll][2] << std::endl;
	  }
	  fout1.close();
	  
    } else {
      continue; // ERROR
    }
  }

  // fit a plane to the points saved in the laserLinePoint array using Eigen routines
  hw3::estimatePlane(laserLinePoint, laserPlane);
  std::cout << "Shadow Plane 1:" << std::endl;
  std::cout << laserPlane << std::endl;
  hw3::estimatePlane(laserLinePoint1, laserPlane2);
  std::cout << "Shadow Plane 2:" << std::endl;
  std::cout << laserPlane2 << std::endl;
  std::cerr << "}\n";
}

//////////////////////////////////////////////////////////////////////
void hw4::scan
(// inputs
 Matrix3d const& K,
 Vector5d const& kc,
 Matrix3d const& worldRotation,
 Vector3d const& worldTranslation,
 float const angle,
 QImage const &  backgroundImage, // with LED 7
 QImage const &  foregroundImage, // with LED 0 
 Vector4d const& laserPlane,
 // outputs
 QVector<Vector3d>& worldCoord) 
{
	std::cerr << "hw4::scan() {\n";
	//Detect
	worldCoord.clear();
	if (backgroundImage.size() != foregroundImage.size()) { //unsupported format
		return;
	}
	if (backgroundImage.format() != foregroundImage.format()) { //unsupported format
		return;
	}

	QImage outputImage(backgroundImage.size(), QImage::Format_RGB32);
	for (int j = 0; j < outputImage.height(); j++)
	{
		for (int k = 0; k < outputImage.width(); k++)
		{
			QRgb pixelback = backgroundImage.pixel(k, j);
			QRgb pixelfore = foregroundImage.pixel(k, j);
			int value = qRed(pixelfore) - qRed(pixelback);
			if (value < 0) {
				value = 0;
			}
			outputImage.setPixel(k, j, qRgb(value, 0, 0));
		}
	}
	//Trangulate
	//worldCoord = hw2::triangulate(K, kc, worldRotation, worldTranslation, laserPlane, angle, outputImage);
	double angle_r = angle * 3.14159265358979 / 180.0;
	worldCoord = hw2::triangulate(K, kc, worldRotation, worldTranslation, laserPlane, angle_r, outputImage);
	std::cout << "angle: " << angle_r << std::endl;

	std::cerr << "}\n";
}