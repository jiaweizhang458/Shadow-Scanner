
#include "homework1.hpp"
#include <app/matrix_util.hpp>
#include <cassert>

namespace hw1 {
  //structure representing a pixel RGBA
  struct RGBAPixel {
    union {
      uchar value[4];
      struct {
        uchar r, g, b, a;
      };
    };
    RGBAPixel(uchar r, uchar g, uchar b, uchar a) : r(r), g(g), b(b), a(a) {}
  };
}

QString hw1::getStudentName(void) {
  //TODO: uncomment the following line and add your name, it'll be shown in About
  return "Jiawei Zhang";
  return QString();
}

QImage hw1::filterImage(QImage const& inputImage, QVector<int> filter) {
  //Description:
  //  hw1::filterImage() convolves 'inputImage' with the filter coefficients
  //  in the 'filter', and returns the filtered image as result. The original
  //  'inputImage' remains unchanged. The given filter coefficients correspond
  //  to a separable filter and must be applied twice: first to convolve 
  //  columns; and second to convolve rows. The filter is commonly given 
  //  unnormalized and must be normalized.

  // Example:
  //  Qimage inputImage;
  //  QVector<int> filter(QVector<int>() << 1 << 4 << 6 << 4 << 1);
  //  QImage outImage = hw1::filterImage(inputImage, filter)
  // Results in: outImage = ((inputImage * [1 4 6 4 1]) * [1 4 6 4 1]^T) / 256
  //  where * denotes convolution, ^T denotes transpose, and 1/256 is the 
  //  normalization of this particular filter: 256 = (1+4+6++4+1)^2.


  //check image format: inputImage format must be QImage::Format_RGB32
  if (inputImage.format() != QImage::Format_RGB32) {
    //unsupported format: do nothing
    return QImage();
  }

  //half filter length
  int N = filter.length() / 2;
  int norm = 0;
  foreach(int v, filter) {
    norm += v;
  }

  //filter columns
  QImage tempImage(inputImage.size(), QImage::Format_RGB32);
  for (int h = 0; h < inputImage.height(); ++h) 
  {
	  RGBAPixel const* inRow =
		  reinterpret_cast<RGBAPixel const*>(inputImage.scanLine(h));
	  RGBAPixel * outRow =
		  reinterpret_cast<RGBAPixel *>(tempImage.scanLine(h));
	  for (int w = 0; w < inputImage.width(); ++w) 
	  {
		  //collect values
		  int r = 0, g = 0, b = 0;
		  for (int i = -N; i <= N; ++i) 
		  {
			  auto index = w + i;
			  if (index < 0)
			  {
				  index = -index;
			  }
			  if (index >= inputImage.width())
			  {
				  index = inputImage.width() - (1 + index - inputImage.width());
			  }
			  RGBAPixel const& inPixel = inRow[index];
			  int h = filter[i + N];
			  r += h*inPixel.r;
			  g += h*inPixel.g;
			  b += h*inPixel.b;
		  }

		  //normalize
		  r /= norm;
		  g /= norm;
		  b /= norm;

		  RGBAPixel & outPixel = outRow[w];
		  outPixel.r = r;
		  outPixel.g = g;
		  outPixel.b = b;
	  }
  }

  //filter rows
  QImage outputImage(inputImage.size(), QImage::Format_RGB32);
  for (int h = 0; h < inputImage.height(); ++h) {
    for (int w = 0; w < inputImage.width(); ++w) {
      //collect values
      int r = 0, g = 0, b = 0;
      for (int i = -N; i <= N; ++i) {
        auto index = w + i;
        if (index < 0) {
          index = -index;
        }
        if (index >= inputImage.height()) {
          index = inputImage.height() - (1 + index - inputImage.height());
        }
        QRgb pixel = tempImage.pixel(w, h);
        int h = filter[i + N];
        r += h*qRed(pixel);
        g += h*qGreen(pixel);
        b += h*qBlue(pixel);
      }

      //normalize
      r /= norm;
      g /= norm;
      b /= norm;

      outputImage.setPixel(w, h, qRgb(r, g, b));
    }
  }

  return outputImage;
}

QImage hw1::detectLaser(QImage const& inputImage) {
  //Description:
  //  hw1::detectLaser() takes an RGB image as input and
  //  creates a new image with all pixels set to 0, except 
  //  the ones corresponding to the projected laser line,
  //  which must be set to 255. 
  //  It is assumed that the laser is approximately a vertical line on the image.
  //  this function will scan the image row-by-row and set all the pixels to zero
  //  except one corresponding to the center of the detected laser line, 
  //  if the laser line is detected.

  //check image format
  if (inputImage.format() != QImage::Format_RGB32) { //unsupported format
    return QImage();
  }

  //inputImage format is QImage::Format_RGB32

  //TODO: implement this function as you wish to create the best possible output.
  //  In general, the detection may be implemented in several steps:
  //  - smooth the input image to remove noise
  //  - compare the red channel with the others: 
  //      e.g. value = r - (g+b)/2
  //  - search the average maximum value of rows: 
  //      e.g. avgMax = (max(row[0]) + max(row[1]) + ... + max(row(N)))/N
  //  - apply a threshold to all values much lower than the average maximum:
  //      e.g. if (pixel<0.8*avgMax) { pixel = 0; }
  //  - search the maximum of each row and set that pixel equal to 255

  //daniel ---------------------------

  //inputImage format is QImage::Format_RGB32

  //blur: f = [1 4 6 4 1]
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
  }

  return outputImage;
}

QImage hw1::turnGray(QImage const& inputImage) 
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

QImage hw1::ComputeMaxIllumination(QImage const& inputImage1 /*RGB*/, QImage const& inputImage2 /*RGB*/) 
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

QImage hw1::detectShadow(QImage const& inputImage, QImage const& inputImage1)
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
		return QImage();
	}

	//blur: f = [1 4 6 4 1]
	//QVector<int> filter(QVector<int>() << 1 << 4 << 6 << 4 << 1);
	//QImage filteredImage = hw1::filterImage(inputImage, filter);
	QImage maxImage = hw1::ComputeMaxIllumination(inputImage, inputImage1);
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

			if (abs(value) < tol) continue; //Important
			else detected.push_back(w); // Detect shadow edge
		}
		if (detected.size() < 2) continue;
		int position_tol = 10;
		if (abs(detected[0] - detected.back()) < position_tol) continue;
		outputImage.setPixel(detected[0], h, qRgb(255, 0, 0));//Mark as green
		outputImage.setPixel(detected.back(), h, qRgb(0, 255, 0));//Mark as blue
	}

	return outputImage;
}