#include "homework3.hpp"
#include "homework2.hpp"
#include "homework1.hpp"
#include <iostream>

// Turntable Calibration

void hw3::estimateExtrinsics
(Matrix3d const& K, /* intrinsic */
 Vector5d const& kc, /* intrinsic */
 QVector<Vector2d> const& worldCoord, /* input */
 QVector<Vector2d> const& imageCoord, /* input */
 Matrix3d& R, /* output */
 Vector3d& T  /* output */) 
{
	Eigen::MatrixXd A(2*worldCoord.size(), 9);
	
	for (int j = 0; j < worldCoord.size(); j++)
	{
		Vector3d p = Vector3d(worldCoord[j][0], worldCoord[j][1], 0);
		Vector3d utilde = K.inverse()*Vector3d(imageCoord[j][0], imageCoord[j][1], 1);
		A(2 * j, 0) = 0;
		A(2 * j, 1) = -p[0];
		A(2 * j, 2) = utilde[1] * p[0];
		A(2 * j, 3) = 0;
		A(2 * j, 4) = -p[1];
		A(2 * j, 5) = utilde[1] * p[1];
		A(2 * j, 6) = 0;
		A(2 * j, 7) = -1;
		A(2 * j, 8) = utilde[1];
		A(2 * j + 1, 0) = p[0];
		A(2 * j + 1, 1) = 0;
		A(2 * j + 1, 2) = -utilde[0] * p[0];
		A(2 * j + 1, 3) = p[1];
		A(2 * j + 1, 4) = 0;
		A(2 * j + 1, 5) = -utilde[0] * p[1];
		A(2 * j + 1, 6) = 1;
		A(2 * j + 1, 7) = 0;
		A(2 * j + 1, 8) = -utilde[0];
	}
	//Method 1: eigen vector WRONG METHOD!!!
	//Eigen::MatrixXd AA = A.adjoint()*A;
	//Eigen::VectorXd x = Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd>(AA).eigenvectors().col(0);
	//Method 2: kernel WRONG METHOD!!!
	//Eigen::MatrixXd ker = A.fullPivLu().kernel();
	//Eigen::VectorXd x = ker.col(0);
	//x.normalize();
	//Method 3: svd Reference: http://math.mit.edu/classes/18.095/2016IAP/lec2/SVD_Notes.pdf
	Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullV);
	Eigen::VectorXd x = svd.matrixV().col(8);
	x.normalize();
	//std::cout << "A*x:" << std::endl;
	//std::cout << A*x << std::endl;

	Vector3d r1(x[0], x[1], x[2]);
	Vector3d r2(x[3], x[4], x[5]);
	Vector3d Ttemp(x[6], x[7], x[8]);
	double s = 2 / (r1.norm() + r2.norm());
	r1 *= s;
	Vector3d r3 = r1.cross(s*r2);
	r2 = r3.cross(r1);
	R.col(0) = r1;
	R.col(1) = r2;
	R.col(2) = r3;
	T = Ttemp*s;
	
}

void hw3::estimateTurntableCenter
(QVector<Matrix3d> const& R, /* input */
 QVector<Vector3d> const& T, /* input */
 Vector2d& centerCoord, /* output */
 Matrix3d& worldRotation, /* output */
 Vector3d& worldTranslation /* output */) 
{
	int num = R.size();
	Eigen::MatrixXd A(3 * num, 5);
	Eigen::VectorXd b(3 * num);
	Matrix3d I = Matrix3d::Identity(3, 3);
	for (int i = 0; i < num; i++)
	{
		A(3 * i + 0, 0) = (R[i])(0, 0);
		A(3 * i + 0, 1) = (R[i])(0, 1);
		A(3 * i + 0, 2) = -I(0, 0);
		A(3 * i + 0, 3) = -I(0, 1);
		A(3 * i + 0, 4) = -I(0, 2);
		b[3 * i + 0] = -T[i][0];

		A(3 * i + 1, 0) = (R[i])(1, 0);
		A(3 * i + 1, 1) = (R[i])(1, 1);
		A(3 * i + 1, 2) = -I(1, 0);
		A(3 * i + 1, 3) = -I(1, 1);
		A(3 * i + 1, 4) = -I(1, 2);
		b[3 * i + 1] = -T[i][1];
		
		A(3 * i + 2, 0) = (R[i])(2, 0);
		A(3 * i + 2, 1) = (R[i])(2, 1);
		A(3 * i + 2, 2) = -I(2, 0);
		A(3 * i + 2, 3) = -I(2, 1);
		A(3 * i + 2, 4) = -I(2, 2);
		b[3 * i + 2] = -T[i][2];
	}
	//Eigen::VectorXd X = A.colPivHouseholderQr().solve(b);//Solve AX=b
	Eigen::VectorXd X = A.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV).solve(b);
	centerCoord[0] = X[0];
	centerCoord[1] = X[1];
	worldTranslation[0] = X[2];
	worldTranslation[1] = X[3];
	worldTranslation[2] = X[4];
	worldRotation = R[0];
}

double hw3::reprojectPoints
(Matrix3d const& K, /* input */
 Vector5d const& kc, /* input */
 Matrix3d const& R, /* input */
 Vector3d const& T, /* input */
 QVector<Vector2d> const& checkerboardPoints, /* input */
 QVector<Vector2d> const& imagePoints, /* input */
 QVector<Vector2d> & reprojectedPoints, /* output */
 Matrix3d const& R_world, /* input */
 Vector3d& T_world, /* input */
 QVector<Vector3d> & worldPoints /* output */ ) 
{
	double rmse = 0.0;
	auto nPoints = checkerboardPoints.size();
	for (int i = 0; i < nPoints; ++i)
	{
		auto const& p = Vector3d(checkerboardPoints[i][0], checkerboardPoints[i][1], 0);

		//project 'p' to the image plane
		Vector3d XXc = R * p + T;
		Vector2d u(XXc[0] / XXc[2], XXc[1] / XXc[2]);

		//apply lens distortion to 'u'
		double r2 = (u[0] * u[0]) + (u[1] * u[1]);
		Vector2d dx(2 * kc[3] * u[0] * u[1] + kc[4] * (r2 + 2 * (u[0] * u[0])), kc[3] * (r2 + 2 * u[1] * u[1]) + 2 * kc[4] * u[0] * u[1]);
		u = (1 + kc[1] * r2 + kc[2] * (r2*r2) + kc[4] * r2*r2*r2)*u + dx;

		//convert 'u' to pixel coords
		u[0] = K(0, 0)*u[0] + K(0, 1)*u[1] + K(0, 2) * 1;
		u[1] = K(1, 1)*u[1] + K(1, 2) * 1;

		//save result
		reprojectedPoints.push_back(u);

		//compare with the original image point 'v'
		auto const& v = imagePoints[i];
		rmse += (u[0] - v[0])*(u[0] - v[0]) + (u[1] - v[1])*(u[1] - v[1]);

		//convert camera coordinate back to world coordinate

		Vector3d w = R.transpose()*(XXc - T_world);
		worldPoints.push_back(w);
	}
	
	//RMSE
	rmse = std::sqrt(rmse / (2 * nPoints));

	return rmse;
}

// Laser Plane Calibration

void hw3::detectLaser
(QImage const& inputImage, /* input */
 QVector<Vector2d>& laserLineCoord /* output */ ) 
{
	if (inputImage.format() != QImage::Format_RGB32) //unsupported format
	{ 
		return;
	}

	QVector<int> filter(QVector<int>() << 1 << 4 << 6 << 4 << 1);
	QImage filteredImage = hw1::filterImage(inputImage, filter);

	QImage outputImage(inputImage.size(), QImage::Format_RGB32);

	//isolate red channel
	for (int h = 0; h < filteredImage.height(); ++h) {
		for (int w = 0; w < filteredImage.width(); ++w) {
			QRgb pixel = filteredImage.pixel(w, h);

			int value = qRed(pixel) - (qBlue(pixel) / 2) - (qGreen(pixel) / 2);
			if (value < 0) {
				value = 0;
			}

			outputImage.setPixel(w, h, qRgb(value, value, value));
		}
	}

	//search average maximum of each row
	int maxValue = 0;
	for (int h = 0; h < outputImage.height(); ++h) {
		int maxRowValue = 0;
		for (int w = 0; w < outputImage.width(); ++w) {
			QRgb pixel = outputImage.pixel(w, h);
			int r = qRed(pixel);
			if (r > maxRowValue) {
				maxRowValue = r;
			}
		}
		maxValue += maxRowValue;
	}
	maxValue /= outputImage.height();

	//remove low values
	int threshold = static_cast<int>(0.8*maxValue);
	for (int h = 0; h < outputImage.height(); ++h) {
		for (int w = 0; w < outputImage.width(); ++w) {
			QRgb pixel = outputImage.pixel(w, h);
			int r = qRed(pixel);
			if (r < threshold) {
				r = 0;
			}
			outputImage.setPixel(w, h, qRgb(r, r, r));
		}
	}

	// peak detector: f = [-1 -1 -1 -1 8 -1 -1 -1 -1]
	// QVector<int> peakFilter(QVector<int>()
	//   << -1 << -1 << -1 << -1 << 8 << -1 << -1 << -1 << -1);
	//outputImage = hw1::filterImage(outputImage, filter);

	//search the maximum of each row
	for (int h = 0; h < outputImage.height(); ++h) {
		int maxRowValue = 0;
		int col = 0;
		for (int w = 0; w < outputImage.width(); ++w) {
			QRgb pixel = outputImage.pixel(w, h);
			int r = qRed(pixel);
			if (r > maxRowValue) {
				maxRowValue = r;
				col = w;
			}
			outputImage.setPixel(w, h, qRgb(0, 0, 0));
		}
		outputImage.setPixel(col, h, qRgb(255, 255, 255));
		///////HW3
		laserLineCoord.push_back(Vector2d(col, h));
		///////HW3
	}
	//outputImage.save("output.png");
	return;
}

QImage hw3::turnGray(QImage const& inputImage)
{
	QImage outputImage(inputImage.size(), QImage::Format_RGB32);
	for (int h = 0; h < inputImage.height(); h++) {
		for (int w = 0; w < inputImage.width(); w++) {
			int gray = qGray(inputImage.pixel(w, h));
			outputImage.setPixel(w, h, qRgb(gray, gray, gray));
		}
	}
	return outputImage;
}

QImage hw3::ComputeMaxIllumination(QImage const& inputImage1 /*RGB*/, QImage const & inputImage2 /*RGB*/)
{//Also get the color of the image
	assert(inputImage1.size() == inputImage2.size());
	QImage inputImage1_gray = hw1::turnGray(inputImage1);
	QImage inputImage2_gray = hw1::turnGray(inputImage2);
	QImage MaxIlluminationImage(inputImage1.size(), QImage::Format_RGB32);
	QImage ColorImage(inputImage1.size(), QImage::Format_RGB32);// Store the color for later use
	// Calculate Max Illuminated Image using two input images.
	for (int h = 0; h < inputImage1_gray.height(); h++) {
		for (int w = 0; w < inputImage1_gray.width(); w++) {
			QRgb pixel1 = inputImage1_gray.pixel(w, h);
			int value1 = qRed(pixel1);
			QRgb pixel2 = inputImage2_gray.pixel(w, h);
			int value2 = qRed(pixel2);
			if (value1 > value2)
			{
				QRgb pixel_color = inputImage1.pixel(w, h);
				MaxIlluminationImage.setPixel(w, h, qRgb(value1, value1, value1));
				ColorImage.setPixel(w, h, pixel_color);
			}
			else
			{
				QRgb pixel_color = inputImage2.pixel(w, h);
				MaxIlluminationImage.setPixel(w, h, qRgb(value2, value2, value2));
				ColorImage.setPixel(w, h, pixel_color);
			}
		}
	}
	return MaxIlluminationImage;
}

void hw3::detectShadow(QImage const& inputImage, QImage const& inputImage1,
	QVector<Vector2d>& laserLineCoord, /* output */
	QVector<Vector2d>& laserLineCoord1 /* output */)
{
	//Description:
	//  hw1::detectShadow() takes an RGB image and corresponding fully illuminated image as input and
	//  creates a new image with all pixels set to 0, except 
	//  the ones corresponding to the projected shadow lines,
	//  which must be set to 255. 
	//  It is assumed that the shadow is approximately a vertical line on the image.
	//  this function will scan the image row-by-row and set all the pixels to zero
	//  except two corresponding to the edges of the detected shadow lines, 

	//check image format
	if (inputImage.format() != QImage::Format_RGB32) { //unsupported format
		return;
	}
	if (inputImage1.format() != QImage::Format_RGB32) { //unsupported format
		return;
	}

	//blur: f = [1 4 6 4 1]
	//QVector<int> filter(QVector<int>() << 1 << 4 << 6 << 4 << 1);
	//QImage filteredImage = hw1::filterImage(inputImage, filter);
	QImage maxImage = hw3::ComputeMaxIllumination(inputImage, inputImage1);
	QImage outputImage(inputImage.size(), QImage::Format_RGB32);
	QImage detectallImage(inputImage.size(), QImage::Format_RGB32);
	outputImage.fill(0);
	detectallImage.fill(0);

	int tol = 150;

	for (int h = 0; h < inputImage.height(); h++) //row
	{
		QVector<int> detected;
		for (int w = 0; w < inputImage.width(); w++)  // column
		{
			QRgb pixel = inputImage.pixel(w, h);
			QRgb max_pixel = maxImage.pixel(w, h);
			int value = qRed(max_pixel) - qRed(pixel);//should be >= 0

			if (abs(value) < tol) continue;
			else
			{
				detectallImage.setPixel(w, h, qRgb(255, 255, 255));
				detected.push_back(w);
			}
				
		}
		if (detected.size() < 2) continue;
		int position_tol = 10;
		if (abs(detected[0] - detected.back()) < position_tol) continue;
		outputImage.setPixel(detected[0], h, qRgb(255, 0, 0));//Mark as green
		laserLineCoord.push_back(Vector2d(detected[0], h));
		outputImage.setPixel(detected.back(), h, qRgb(0, 255, 0));//Mark as blue
		laserLineCoord1.push_back(Vector2d(detected.back(), h));
	}
	outputImage.save("shadowoutput.png");
	detectallImage.save("shadowall.png");
	return;
}



void hw3::checkerboardPlane
(Matrix3d& R, /* input */
 Vector3d& T, /* input */
 Vector4d & checkerboardPlane /* output */) 
{
	Vector3d temp = R.col(2);
	temp.normalize();
	checkerboardPlane[0] = temp[0];
	checkerboardPlane[1] = temp[1];
	checkerboardPlane[2] = temp[2];
	checkerboardPlane[3] = -T.dot(temp);
}

void hw3::triangulate
(Matrix3d const& K, /* input */
 Vector5d const& kc, /* input */
 Vector4d& planeCoefficients,
 QVector<Vector2d> const& imagePoints, /* input */
 QVector<Vector3d> & worldPoints /* output */ ) 
{
	for (int i = 0; i < imagePoints.size(); i++)
	{
		Vector3d u(imagePoints[i][0], imagePoints[i][1], 1);
		Vector3d a = K.inverse()*u;
		double lambda = -planeCoefficients[3] / (planeCoefficients[0] * a[0] + planeCoefficients[1] * a[1] + planeCoefficients[2] * a[2]);
		lambda = abs(lambda);
		Vector3d v = a*lambda;
		//if (abs(v[0]) <= 100 && abs(v[1]) <= 500 && v[1] <= 80 && abs(v[2]) <= 440)
		//{
		//	//lambda *= -1;
		//	worldPoints.push_back(v);
		//}
		//lambda *= -1;
		worldPoints.push_back(v);
	}
}

void hw3::estimatePlane
(QVector<Vector3d> & laserPoints, /* input */
 Vector4d & laserPlane /* output */) 
{
	//Reference: https://www.ltu.se/cms_fs/1.51590!/svd-fitting.pdf
	Vector3d c(0.0, 0.0, 0.0);
	Eigen::MatrixXd A(3, laserPoints.size());
	for (int i = 0; i < laserPoints.size(); i++)
	{
		A.col(i) = laserPoints[i];
		c += laserPoints[i];
	}
	c /= laserPoints.size();
	for (int i = 0; i < laserPoints.size(); i++)
	{
		A.col(i) -= c;
	}
	Eigen::JacobiSVD<Eigen::MatrixXd> svd(A, Eigen::ComputeFullU);
	Vector3d n = svd.matrixU().col(2);
	double w = -n.dot(c);
	laserPlane = Vector4d(n[0], n[1], n[2], w);
	//std::cout << c << std::endl;
}
