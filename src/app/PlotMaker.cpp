// Software developed for the Spring 2016 Brown University course
// ENGN 2502 3D Photography
// Copyright (c) 2016, Daniel Moreno and Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL DANIEL MORENO NOR GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "PlotMaker.hpp"

#include <QPainter>

PlotMaker::PlotMaker(size_t width, size_t height) :
  _width(width),
  _height(height)
{
}

PlotMaker::~PlotMaker()
{
}

void PlotMaker::setData(uchar *data, size_t count)
{
  _data.clear();
  for (size_t i = 0; i < count; ++i)
  {
    _data.append(static_cast<float>(data[i]));
  }
}

void PlotMaker::setData(QVector<float> const& data)
{
  _data.clear();
  for (int i = 0; i < data.length(); ++i)
  {
    _data.append(static_cast<float>(data[i]));
  }
}

QImage PlotMaker::getImage(void)
{ //draw the plot
  int cols = static_cast<int>(_width);
  int rows = static_cast<int>(_height);

  QImage plot(cols, rows, QImage::Format_RGB32);
  QPainter painter(&plot);

  //erase background
  painter.setBackground(Qt::white);
  painter.eraseRect(0, 0, cols, rows);

  float maxVal = 0;
  float minVal = 0;
  foreach(auto val, _data)
  {
    if (val > maxVal) { maxVal = val; }
    if (val < minVal) { minVal = val; }
  }
  float delta = maxVal - minVal;
  if (delta <= 0.f) {
    delta = 1.f;
  }
  float maxCoord = maxVal + 0.05*delta;
  float minCoord = minVal - 0.05*delta;

  //draw horizontal axis
  // 0 = maxCoord
  // rows-1 = minCoord
  // x ?
  // alpha = (x-minCoord)/(maxCoord-minCoord)
  // p = a + alpha*b = (rows-1) - alpha*(rows-1) = (rows-1)*(1-alpha)
  float alpha = (0 - minCoord) / (maxCoord - minCoord);
  float hAxis = (1 - alpha)*(rows - 1);
  painter.drawLine(0, hAxis, cols - 1, hAxis);

  //plot data
  int N = _data.length();
  painter.setPen(Qt::red);
  if (N > 0)
  {
    QPoint p0(0.f + ((0 - 0.f) / (N - 1.f - 0.f))*(cols - 1.f), (1.f-((_data[0] - minCoord) / (maxCoord - minCoord)))*(rows - 1));
    for (int i = 1; i < N; ++i)
    {
      QPoint p1(0.f + ((i - 0.f) / (N - 1.f - 0.f))*(cols - 1.f), (1.f-((_data[i] - minCoord) / (maxCoord - minCoord)))*(rows - 1));
      painter.drawLine(p0, p1);
      p0 = p1;
    }
  }

  return plot;
}
