//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-03-12 14:06:26 taubin>
//------------------------------------------------------------------------
//
// WrlToolsWidget.cpp
//
// Software developed for the Fall 2015 Brown University course
// ENGN 2912B Scientific Computing in C++
// Copyright (c) 2015, Gabriel Taubin
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
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <iostream>

#include "WrlToolsWidget.hpp"
#include "MainWindow.hpp"

#include "wrl/SceneGraphProcessor.h"

//////////////////////////////////////////////////////////////////////
WrlToolsWidget::WrlToolsWidget(MainWindow *mw):
  QWidget(),
  _mainWindow(mw) {

  static const char* bg0 =
    "QLabel { background-color : rgb(200,200,200); color : black; }";

  QFont font;
  font.setPointSize(10);

  //////////////////////////////////////////////////
  _labelCanvas = new QLabel("3D CANVAS",this);
  _labelCanvas->setFont(font);
  _labelCanvas->setAlignment(Qt::AlignCenter);
  _labelCanvas->setStyleSheet(bg0);

  _labelCanvasWidth = new QLabel("WIDTH",this);
  _labelCanvasWidth->setIndent(5);
  _labelCanvasWidth->setFont(font);
  // _labelCanvasWidth->setAlignment(Qt::AlignCenter);

  _editCanvasWidth = new QLineEdit(this);
  _editCanvasWidth->setFont(font);
  _editCanvasWidth->setReadOnly(true);

  _labelCanvasHeight = new QLabel("HEIGHT",this);
  _labelCanvasHeight->setIndent(5);
  _labelCanvasHeight->setFont(font);
  // _labelCanvasHeight->setAlignment(Qt::AlignCenter);

  _editCanvasHeight = new QLineEdit(this);
  _editCanvasHeight->setFont(font);
  _editCanvasHeight->setReadOnly(true);

  //////////////////////////////////////////////////
  _labelBBox = new QLabel("BBOX",this);
  _labelBBox->setFont(font);
  _labelBBox->setAlignment(Qt::AlignCenter);
  _labelBBox->setStyleSheet(bg0);

  _labelBBoxSize = new QLabel("SIZE",this);
  _labelBBoxSize->setIndent(5);
  _labelBBoxSize->setFont(font);
  // _labelBBoxSize->setAlignment(Qt::AlignCenter);

  _spinBoxBBoxDepth = new QSpinBox(this);
  _spinBoxBBoxDepth->setFont(font);
  _spinBoxBBoxDepth->setRange(0,10);
  _spinBoxBBoxDepth->setValue(0);
  _spinBoxBBoxDepth->setAlignment(Qt::AlignCenter);
  connect(_spinBoxBBoxDepth,
          SIGNAL(valueChanged(int)), this, SLOT(bboxSetDepth(int)));

  _pushButtonBBoxAdd = new QPushButton("ADD",this);
  _pushButtonBBoxAdd->setFont(font);
  connect(_pushButtonBBoxAdd,
          SIGNAL(clicked(bool)), this, SLOT(bboxAdd()));

  _pushButtonBBoxRemove = new QPushButton("REMOVE",this);
  _pushButtonBBoxRemove->setFont(font);
  connect(_pushButtonBBoxRemove,
          SIGNAL(clicked(bool)), this, SLOT(bboxRemove()));

  _labelBBoxScale = new QLabel("SCALE",this);
  _labelBBoxScale->setIndent(5);
  _labelBBoxScale->setFont(font);

  _editBBoxScale = new QLineEdit(this);
  _editBBoxScale->setFont(font);
  _editBBoxScale->setReadOnly(false);
  connect(_editBBoxScale,
          SIGNAL(returnPressed()), this, SLOT(bboxScaleEnter()));

  _checkBoxBBoxCube = new QCheckBox("CUBE",this);
  _checkBoxBBoxCube->setFont(font);
  _checkBoxBBoxCube->setChecked(true);
  connect(_checkBoxBBoxCube,
          SIGNAL(stateChanged(int)), this, SLOT(bboxCubeSetState(int)));

  _labelGridCells = new QLabel("CELLS",this);
  _labelGridCells->setIndent(5);
  _labelGridCells->setFont(font);

  _editGridCells = new QLineEdit(this);
  _editGridCells->setFont(font);
  _editGridCells->setReadOnly(true);

  _labelGridVertices = new QLabel("VERTICES",this);
  _labelGridVertices->setIndent(5);
  _labelGridVertices->setFont(font);

  _editGridVertices = new QLineEdit(this);
  _editGridVertices->setFont(font);
  _editGridVertices->setReadOnly(true);

  //////////////////////////////////////////////////
  _labelSceneGraph = new QLabel("SCENE GRAPH",this);
  _labelSceneGraph->setFont(font);
  _labelSceneGraph->setAlignment(Qt::AlignCenter);
  _labelSceneGraph->setStyleSheet(bg0);

  _labelSceneGraphNormalBinding = new QLabel("NORMAL BINDING",this);
  _labelSceneGraphNormalBinding->setIndent(5);
  _labelSceneGraphNormalBinding->setFont(font);

  _pushButtonSceneGraphNormalNone = new QPushButton("NONE",this);
  _pushButtonSceneGraphNormalNone->setFont(font);
  connect(_pushButtonSceneGraphNormalNone,
          SIGNAL(clicked(bool)), this, SLOT(setNormalNone()));

  _pushButtonSceneGraphNormalPerVertex = new QPushButton("VERTEX",this);
  _pushButtonSceneGraphNormalPerVertex->setFont(font);
  connect(_pushButtonSceneGraphNormalPerVertex,
          SIGNAL(clicked(bool)), this, SLOT(setNormalPerVertex()));

  _pushButtonSceneGraphNormalPerFace = new QPushButton("FACE",this);
  _pushButtonSceneGraphNormalPerFace->setFont(font);
  connect(_pushButtonSceneGraphNormalPerFace,
          SIGNAL(clicked(bool)), this, SLOT(setNormalPerFace()));

  _pushButtonSceneGraphNormalPerCorner = new QPushButton("CORNER",this);
  _pushButtonSceneGraphNormalPerCorner->setFont(font);
  connect(_pushButtonSceneGraphNormalPerCorner,
          SIGNAL(clicked(bool)), this, SLOT(setNormalPerCorner()));

  _pushButtonSceneGraphNormalInvert = new QPushButton("INVERT",this);
  _pushButtonSceneGraphNormalInvert->setFont(font);
  connect(_pushButtonSceneGraphNormalInvert,
          SIGNAL(clicked(bool)), this, SLOT(normalInvert()));

  _labelSceneIndexedFaceSets = new QLabel("INDEXED FACE SETS",this);
  _labelSceneIndexedFaceSets->setIndent(5);
  _labelSceneIndexedFaceSets->setFont(font);

  _pushButtonSceneGraphIndexedFaceSetsShow = new QPushButton("SHOW",this);
  _pushButtonSceneGraphIndexedFaceSetsShow->setFont(font);
  connect(_pushButtonSceneGraphIndexedFaceSetsShow,
          SIGNAL(clicked(bool)), this, SLOT(shapeIfsShow()));

  _pushButtonSceneGraphIndexedFaceSetsHide = new QPushButton("HIDE",this);
  _pushButtonSceneGraphIndexedFaceSetsHide->setFont(font);
  connect(_pushButtonSceneGraphIndexedFaceSetsHide,
          SIGNAL(clicked(bool)), this, SLOT(shapeIfsHide()));

  _labelSceneIndexedLineSets = new QLabel("INDEXED LINE SETS",this);
  _labelSceneIndexedLineSets->setIndent(5);
  _labelSceneIndexedLineSets->setFont(font);

  _pushButtonSceneGraphIndexedLineSetsShow = new QPushButton("SHOW",this);
  _pushButtonSceneGraphIndexedLineSetsShow->setFont(font);
  connect(_pushButtonSceneGraphIndexedLineSetsShow,
          SIGNAL(clicked(bool)), this, SLOT(shapeIlsShow()));

  _pushButtonSceneGraphIndexedLineSetsHide = new QPushButton("HIDE",this);
  _pushButtonSceneGraphIndexedLineSetsHide->setFont(font);
  connect(_pushButtonSceneGraphIndexedLineSetsHide,
          SIGNAL(clicked(bool)), this, SLOT(shapeIlsHide()));

  //////////////////////////////////////////////////
  _labelPoints = new QLabel("POINTS",this);
  _labelPoints->setFont(font);
  _labelPoints->setAlignment(Qt::AlignCenter);
  _labelPoints->setStyleSheet(bg0);

  _editPointsNumber = new QLineEdit(this);
  _editPointsNumber->setFont(font);

  _checkBoxPointsHasNormals = new QCheckBox("HAS NORMALS",this);
  _checkBoxPointsHasNormals->setFont(font);
  // _checkBoxPointsHasNormals->setCheckable(false);
  _checkBoxPointsHasNormals->setEnabled(false);

  _checkBoxPointsHasColors = new QCheckBox("HAS COLORS", this);
  _checkBoxPointsHasColors->setFont(font);
  // _checkBoxPointsHasColors->setCheckable(false);
  _checkBoxPointsHasColors->setEnabled(false);

  _pushButtonPointsRemove = new QPushButton("REMOVE",this);
  _pushButtonPointsRemove->setFont(font);
  connect(_pushButtonPointsRemove,
          SIGNAL(clicked(bool)), this, SLOT(pointsRemove()));

  _pushButtonPointsShow = new QPushButton("SHOW",this);
  _pushButtonPointsShow->setFont(font);
  connect(_pushButtonPointsShow,
          SIGNAL(clicked(bool)), this, SLOT(pointsShow()));

  _pushButtonPointsHide = new QPushButton("HIDE",this);
  _pushButtonPointsHide->setFont(font);
  connect(_pushButtonPointsHide,
          SIGNAL(clicked(bool)), this, SLOT(pointsHide()));

  //////////////////////////////////////////////////
  _labelEdges = new QLabel("EDGES",this);
  _labelEdges->setFont(font);
  _labelEdges->setAlignment(Qt::AlignCenter);
  _labelEdges->setStyleSheet(bg0);

  _pushButtonSceneGraphEdgesAdd = new QPushButton("ADD",this);
  _pushButtonSceneGraphEdgesAdd->setFont(font);
  connect(_pushButtonSceneGraphEdgesAdd,
          SIGNAL(clicked(bool)), this, SLOT(edgesAdd()));

  _pushButtonSceneGraphEdgesRemove = new QPushButton("REMOVE",this);
  _pushButtonSceneGraphEdgesRemove->setFont(font);
  connect(_pushButtonSceneGraphEdgesRemove,
          SIGNAL(clicked(bool)), this, SLOT(edgesRemove()));

  _pushButtonSceneGraphEdgesShow = new QPushButton("SHOW",this);
  _pushButtonSceneGraphEdgesShow->setFont(font);
  connect(_pushButtonSceneGraphEdgesShow,
          SIGNAL(clicked(bool)), this, SLOT(edgesShow()));

  _pushButtonSceneGraphEdgesHide = new QPushButton("HIDE",this);
  _pushButtonSceneGraphEdgesHide->setFont(font);
  connect(_pushButtonSceneGraphEdgesHide,
          SIGNAL(clicked(bool)), this, SLOT(edgesHide()));

  //////////////////////////////////////////////////
  _labelSurface = new QLabel("SURFACE",this);
  _labelSurface->setFont(font);
  _labelSurface->setAlignment(Qt::AlignCenter);
  _labelSurface->setStyleSheet(bg0);

  _labelSurfaceVertices = new QLabel("VERTICES",this);
  _labelSurfaceVertices->setIndent(5);
  _labelSurfaceVertices->setFont(font);

  _editSurfaceVertices = new QLineEdit(this);
  _editSurfaceVertices->setFont(font);

  _labelSurfaceFaces = new QLabel("FACES",this);
  _labelSurfaceFaces->setIndent(5);
  _labelSurfaceFaces->setFont(font);

  _editSurfaceFaces = new QLineEdit(this);
  _editSurfaceFaces->setFont(font);

  _pushButtonSurfaceRemove = new QPushButton("REMOVE",this);
  _pushButtonSurfaceRemove->setFont(font);
  connect(_pushButtonSurfaceRemove,
          SIGNAL(clicked(bool)), this, SLOT(surfaceRemove()));

  _pushButtonSurfaceShow = new QPushButton("SHOW",this);
  _pushButtonSurfaceShow->setFont(font);
  connect(_pushButtonSurfaceShow,
          SIGNAL(clicked(bool)), this, SLOT(surfaceShow()));

  _pushButtonSurfaceHide = new QPushButton("HIDE",this);
  _pushButtonSurfaceHide->setFont(font);
  connect(_pushButtonSurfaceHide,
          SIGNAL(clicked(bool)), this, SLOT(surfaceHide()));

  //////////////////////////////////////////////////
  _labelHeuristicReconstruction = new QLabel("HEURISTIC RECONSTRUCTION",this);
  _labelHeuristicReconstruction->setFont(font);
  _labelHeuristicReconstruction->setAlignment(Qt::AlignCenter);
  _labelHeuristicReconstruction->setStyleSheet(bg0);

  _pushButtonSurfaceMultiplePlanesFit = new QPushButton("MULT-PLANES",this);
  _pushButtonSurfaceMultiplePlanesFit->setFont(font);
  connect(_pushButtonSurfaceMultiplePlanesFit,
          SIGNAL(clicked(bool)), this, SLOT(fitMultiplePlanes()));

  _pushButtonSurfaceContinuousFit = new QPushButton("CONTINUOUS",this);
  _pushButtonSurfaceContinuousFit->setFont(font);
  connect(_pushButtonSurfaceContinuousFit,
          SIGNAL(clicked(bool)), this, SLOT(fitContinuous()));

  _pushButtonSurfaceWatertightFit = new QPushButton("WATERTIGHT",this);
  _pushButtonSurfaceWatertightFit->setFont(font);
  connect(_pushButtonSurfaceWatertightFit,
          SIGNAL(clicked(bool)), this, SLOT(fitWatertight()));

  //////////////////////////////////////////////////////////////////////
  _labelOptimalReconstruction = new QLabel("OPTIMAL RECONSTRUCTION",this);
  _labelOptimalReconstruction->setFont(font);
  _labelOptimalReconstruction->setAlignment(Qt::AlignCenter);
  _labelOptimalReconstruction->setStyleSheet(bg0);

  _pushButtonOptimalCGHard = new QPushButton("CG HARD",this);
  _pushButtonOptimalCGHard->setFont(font);
  connect(_pushButtonOptimalCGHard,
          SIGNAL(clicked(bool)), this, SLOT(optimalCGHard()));

  _pushButtonOptimalCGSoft = new QPushButton("CG SOFT",this);
  _pushButtonOptimalCGSoft->setFont(font);
  connect(_pushButtonOptimalCGSoft,
          SIGNAL(clicked(bool)), this, SLOT(optimalCGSoft()));

  _pushButtonOptimalJacobi = new QPushButton("JACOBI",this);
  _pushButtonOptimalJacobi->setFont(font);
  connect(_pushButtonOptimalJacobi,
          SIGNAL(clicked(bool)), this, SLOT(optimalJacobi()));

  //////////////////////////////////////////////////////////////////////
  _labelMultigridReconstruction = new QLabel("MULTIGRID RECONSTRUCTION",this);
  _labelMultigridReconstruction->setFont(font);
  _labelMultigridReconstruction->setAlignment(Qt::AlignCenter);
  _labelMultigridReconstruction->setStyleSheet(bg0);

  _pushButtonMultigridFiner = new QPushButton("FINER",this);
  _pushButtonMultigridFiner->setFont(font);
  connect(_pushButtonMultigridFiner,
          SIGNAL(clicked(bool)), this, SLOT(multigridFiner()));

  _pushButtonMultigridCoarser = new QPushButton("COARSER",this);
  _pushButtonMultigridCoarser->setFont(font);
  connect(_pushButtonMultigridCoarser,
          SIGNAL(clicked(bool)), this, SLOT(multigridCoarser()));

  _pushButtonMultigridRun = new QPushButton("RUN",this);
  _pushButtonMultigridRun->setFont(font);
  connect(_pushButtonMultigridRun,
          SIGNAL(clicked(bool)), this, SLOT(multigridRun()));

}

//////////////////////////////////////////////////////////////////////
WrlToolsWidget::~WrlToolsWidget() {
}

//////////////////////////////////////////////////////////////////////
void WrlToolsWidget::resizeEvent(QResizeEvent * event) {

  int width  = event->size().width();
  // int height = event->size().height();

  int rowH = 20;
  int rowBorder = 0;
  int colBorder = 0;
  int rowSpace = 3;
  int colSpace = 3;

  int x,y,w,x0,x1,x2,x3,w0,w1,w2,w3;

  x = colBorder;
  w = width-2*colBorder;
  y = rowBorder;

  _labelCanvas->setFixedHeight(rowH);
  _labelCanvas->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  w0 = (width-3*colSpace-2*colBorder)/4;
  w1 = w0;
  w2 = w1;
  w3 = width-3*colSpace-2*colBorder-w0-w1-w2;
  x0 = colBorder;
  x1 = x0+w0+colSpace;
  x2 = x1+w1+colSpace;
  x3 = x2+w2+colSpace;

  _labelCanvasWidth->setFixedHeight(rowH);
  _labelCanvasWidth->setGeometry(x0,y,w0,rowH);
  _editCanvasWidth->setFixedHeight(rowH);
  _editCanvasWidth->setGeometry(x1,y,w1,rowH);
  _labelCanvasHeight->setFixedHeight(rowH);
  _labelCanvasHeight->setGeometry(x2,y,w2,rowH);
  _editCanvasHeight->setFixedHeight(rowH);
  _editCanvasHeight->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelBBox->setFixedHeight(rowH);
  _labelBBox->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _labelBBoxSize->setFixedHeight(rowH);
  _labelBBoxSize->setGeometry(x0,y,w0,rowH);
  _spinBoxBBoxDepth->setFixedHeight(rowH);
  _spinBoxBBoxDepth->setGeometry(x1,y,w1,rowH);
  _pushButtonBBoxAdd->setFixedHeight(rowH);
  _pushButtonBBoxAdd->setGeometry(x2,y,w2,rowH);
  _pushButtonBBoxRemove->setFixedHeight(rowH);
  _pushButtonBBoxRemove->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelBBoxScale->setFixedHeight(rowH);
  _labelBBoxScale->setGeometry(x0,y,w0,rowH);
  _editBBoxScale->setFixedHeight(rowH);
  _editBBoxScale->setGeometry(x1,y,w1,rowH);
  _checkBoxBBoxCube->setFixedHeight(rowH);
  _checkBoxBBoxCube->setGeometry(x2,y,w2+colSpace+w3,rowH);

  y += rowH+rowSpace;

  _labelGridCells->setFixedHeight(rowH);
  _labelGridCells->setGeometry(x0,y,w0,rowH);
  _editGridCells->setFixedHeight(rowH);
  _editGridCells->setGeometry(x1,y,w1,rowH);
  _labelGridVertices->setFixedHeight(rowH);
  _labelGridVertices->setGeometry(x2,y,w2,rowH);
  _editGridVertices->setFixedHeight(rowH);
  _editGridVertices->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelSceneGraph->setFixedHeight(rowH);
  _labelSceneGraph->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _labelSceneGraphNormalBinding->setFixedHeight(rowH);
  _labelSceneGraphNormalBinding->setGeometry(x0,y,w0+colSpace+w1,rowH);
  _pushButtonSceneGraphNormalNone->setFixedHeight(rowH);
  _pushButtonSceneGraphNormalNone->setGeometry(x2,y,w3,rowH);
  _pushButtonSceneGraphNormalPerVertex->setFixedHeight(rowH);
  _pushButtonSceneGraphNormalPerVertex->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _pushButtonSceneGraphNormalInvert->setFixedHeight(rowH);
  _pushButtonSceneGraphNormalInvert->setGeometry(x1,y,w1,rowH);
  _pushButtonSceneGraphNormalPerFace->setFixedHeight(rowH);
  _pushButtonSceneGraphNormalPerFace->setGeometry(x2,y,w2,rowH);
  _pushButtonSceneGraphNormalPerCorner->setFixedHeight(rowH);
  _pushButtonSceneGraphNormalPerCorner->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelSceneIndexedFaceSets->setFixedHeight(rowH);
  _labelSceneIndexedFaceSets->setGeometry(x0,y,w0+colSpace+w1,rowH);
  _pushButtonSceneGraphIndexedFaceSetsShow->setFixedHeight(rowH);
  _pushButtonSceneGraphIndexedFaceSetsShow->setGeometry(x2,y,w2,rowH);
  _pushButtonSceneGraphIndexedFaceSetsHide->setFixedHeight(rowH);
  _pushButtonSceneGraphIndexedFaceSetsHide->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelSceneIndexedLineSets->setFixedHeight(rowH);
  _labelSceneIndexedLineSets->setGeometry(x0,y,w0+colSpace+w1,rowH);
  _pushButtonSceneGraphIndexedLineSetsShow->setFixedHeight(rowH);
  _pushButtonSceneGraphIndexedLineSetsShow->setGeometry(x2,y,w2,rowH);
  _pushButtonSceneGraphIndexedLineSetsHide->setFixedHeight(rowH);
  _pushButtonSceneGraphIndexedLineSetsHide->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelPoints->setFixedHeight(rowH);
  _labelPoints->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _editPointsNumber->setFixedHeight(rowH);
  _editPointsNumber->setGeometry(x0,y,w0,rowH);
  {
    int w123 = w1+colSpace+w2+colSpace+w2;
    int wL = (w123-colSpace)/2;
    int wR = w123-colSpace-wL;

    _checkBoxPointsHasNormals->setFixedHeight(rowH);
    _checkBoxPointsHasNormals->setGeometry(x1,y,wL,rowH);
    _checkBoxPointsHasColors->setFixedHeight(rowH);
    _checkBoxPointsHasColors->setGeometry(x1+wL+colSpace,y,wR,rowH);
  }

  y += rowH+rowSpace;

  _pushButtonPointsRemove->setFixedHeight(rowH);
  _pushButtonPointsRemove->setGeometry(x1,y,w1,rowH);
  _pushButtonPointsShow->setFixedHeight(rowH);
  _pushButtonPointsShow->setGeometry(x2,y,w2,rowH);
  _pushButtonPointsHide->setFixedHeight(rowH);
  _pushButtonPointsHide->setGeometry(x3,y,w3,rowH);


  y += rowH+rowSpace;

  _labelEdges->setFixedHeight(rowH);
  _labelEdges->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _pushButtonSceneGraphEdgesAdd->setFixedHeight(rowH);
  _pushButtonSceneGraphEdgesAdd->setGeometry(x0,y,w0,rowH);
  _pushButtonSceneGraphEdgesRemove->setFixedHeight(rowH);
  _pushButtonSceneGraphEdgesRemove->setGeometry(x1,y,w1,rowH);
  _pushButtonSceneGraphEdgesShow->setFixedHeight(rowH);
  _pushButtonSceneGraphEdgesShow->setGeometry(x2,y,w2,rowH);
  _pushButtonSceneGraphEdgesHide->setFixedHeight(rowH);
  _pushButtonSceneGraphEdgesHide->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _labelSurface->setFixedHeight(rowH);
  _labelSurface->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _labelSurfaceVertices->setFixedHeight(rowH);
  _labelSurfaceVertices->setGeometry(x0,y,w0,rowH);
  _editSurfaceVertices->setFixedHeight(rowH);
  _editSurfaceVertices->setGeometry(x1,y,w1,rowH);
  _labelSurfaceFaces->setFixedHeight(rowH);
  _labelSurfaceFaces->setGeometry(x2,y,w2,rowH);
  _editSurfaceFaces->setFixedHeight(rowH);
  _editSurfaceFaces->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  _pushButtonSurfaceRemove->setFixedHeight(rowH);
  _pushButtonSurfaceRemove->setGeometry(x1,y,w1,rowH);
  _pushButtonSurfaceShow->setFixedHeight(rowH);
  _pushButtonSurfaceShow->setGeometry(x2,y,w2,rowH);
  _pushButtonSurfaceHide->setFixedHeight(rowH);
  _pushButtonSurfaceHide->setGeometry(x3,y,w3,rowH);

  y += rowH+rowSpace;

  w0 = (width-2*colSpace-2*colBorder)/3;
  w1 = w0;
  w2 = width-3*colSpace-2*colBorder-w0-w1;
  x0 = colBorder;
  x1 = x0+w0+colSpace;
  x2 = x1+w1+colSpace;

  _labelHeuristicReconstruction->setFixedHeight(rowH);
  _labelHeuristicReconstruction->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _pushButtonSurfaceMultiplePlanesFit->setFixedHeight(rowH);
  _pushButtonSurfaceMultiplePlanesFit->setGeometry(x0,y,w0,rowH);
  _pushButtonSurfaceContinuousFit->setFixedHeight(rowH);
  _pushButtonSurfaceContinuousFit->setGeometry(x1,y,w1,rowH);
  _pushButtonSurfaceWatertightFit->setFixedHeight(rowH);
  _pushButtonSurfaceWatertightFit->setGeometry(x2,y,w2,rowH);

  y += rowH+rowSpace;

  _labelOptimalReconstruction->setFixedHeight(rowH);
  _labelOptimalReconstruction->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _pushButtonOptimalCGHard->setFixedHeight(rowH);
  _pushButtonOptimalCGHard->setGeometry(x0,y,w0,rowH);
  _pushButtonOptimalCGSoft->setFixedHeight(rowH);
  _pushButtonOptimalCGSoft->setGeometry(x1,y,w1,rowH);
  _pushButtonOptimalJacobi->setFixedHeight(rowH);
  _pushButtonOptimalJacobi->setGeometry(x2,y,w2,rowH);

  y += rowH+rowSpace;

  _labelMultigridReconstruction->setFixedHeight(rowH);
  _labelMultigridReconstruction->setGeometry(x,y,w,rowH);

  y += rowH+rowSpace;

  _pushButtonMultigridFiner->setFixedHeight(rowH);
  _pushButtonMultigridFiner->setGeometry(x0,y,w0,rowH);
  _pushButtonMultigridCoarser->setFixedHeight(rowH);
  _pushButtonMultigridCoarser->setGeometry(x1,y,w1,rowH);
  _pushButtonMultigridRun->setFixedHeight(rowH);
  _pushButtonMultigridRun->setGeometry(x2,y,w2,rowH);

}

//////////////////////////////////////////////////////////////////////
void WrlToolsWidget::updateState() {

  std::cout << "WrlToolsWidget::updateState() {\n";

  _editCanvasWidth->setText
    ("  "+QString::number(_mainWindow->getWrlGLWidgetWidth()));
  _editCanvasHeight->setText
    ("  "+QString::number(_mainWindow->getWrlGLWidgetHeight()));

  WrlViewerData& data  = _mainWindow->getData();

  int   bboxDepth = data.getBBoxDepth();
  bool  bboxCube  = data.getBBoxCube();
  float bboxScale = data.getBBoxScale();

  _spinBoxBBoxDepth->setValue(bboxDepth);
  _checkBoxBBoxCube->setChecked(bboxCube);
  _editBBoxScale->setText("  "+QString::number(bboxScale,'f',2));

  int N = 1<<bboxDepth;
  int nGridCells = N*N*N;
  int nGridVertices = (N+1)*(N+1)*(N+1);

  _editGridCells->setText("  "+QString::number(nGridCells));
  _editGridVertices->setText("  "+QString::number(nGridVertices));
  
  SceneGraph* wrl = data.getSceneGraph();
  if(wrl==(SceneGraph*)0) {

    _pushButtonBBoxAdd->setEnabled(false);
    _pushButtonBBoxRemove->setEnabled(false);
    _pushButtonSceneGraphNormalNone->setEnabled(false);
    _pushButtonSceneGraphNormalPerVertex->setEnabled(false);
    _pushButtonSceneGraphNormalPerFace->setEnabled(false);
    _pushButtonSceneGraphNormalPerCorner->setEnabled(false);
    _pushButtonSceneGraphNormalInvert->setEnabled(false);
    _pushButtonSceneGraphEdgesAdd->setEnabled(false);
    _pushButtonSceneGraphEdgesRemove->setEnabled(false);
    _pushButtonSceneGraphEdgesShow->setEnabled(false);
    _pushButtonSceneGraphEdgesHide->setEnabled(false);
    _pushButtonSceneGraphIndexedFaceSetsShow->setEnabled(false);
    _pushButtonSceneGraphIndexedFaceSetsHide->setEnabled(false);
    _pushButtonSceneGraphIndexedLineSetsShow->setEnabled(false);
    _pushButtonSceneGraphIndexedLineSetsHide->setEnabled(false);
    _pushButtonPointsRemove->setEnabled(false);
    _pushButtonPointsShow->setEnabled(false);
    _pushButtonPointsHide->setEnabled(false);
    _pushButtonSurfaceRemove->setEnabled(false);
    _pushButtonSurfaceShow->setEnabled(false);
    _pushButtonSurfaceHide->setEnabled(false);

    _pushButtonSurfaceMultiplePlanesFit->setEnabled(false);
    _pushButtonSurfaceContinuousFit->setEnabled(false);
    _pushButtonSurfaceWatertightFit->setEnabled(false);
    _pushButtonOptimalCGHard->setEnabled(false);
    _pushButtonOptimalCGSoft->setEnabled(false);
    _pushButtonOptimalJacobi->setEnabled(false);
    _pushButtonMultigridFiner->setEnabled(false);
    _pushButtonMultigridCoarser->setEnabled(false);
    _pushButtonMultigridRun->setEnabled(false);

  } else /* if(wrl!=(SceneGraph*)0) */ {

    SceneGraphProcessor processor(*wrl);

    bool hasBBox   = processor.hasBBox();
    _pushButtonBBoxAdd->setEnabled(!hasBBox);
    _pushButtonBBoxRemove->setEnabled(hasBBox);

    bool hasNormal = false;
    bool hasFaces  = processor.hasIndexedFaceSetFaces();
    bool value     = processor.hasIndexedFaceSetNormalNone();
    _pushButtonSceneGraphNormalNone->setEnabled(!value);
    value = processor.hasIndexedFaceSetNormalPerVertex();
    hasNormal |= value;
    _pushButtonSceneGraphNormalPerVertex->setEnabled(hasFaces && !value);
    value = processor.hasIndexedFaceSetNormalPerFace();
    hasNormal |= value;
    _pushButtonSceneGraphNormalPerFace->setEnabled(hasFaces && !value);
    value = processor.hasIndexedFaceSetNormalPerCorner();
    hasNormal |= value;
    _pushButtonSceneGraphNormalPerCorner->setEnabled(hasFaces && !value);
    _pushButtonSceneGraphNormalInvert->setEnabled(hasNormal);

    value = processor.hasIndexedFaceSetShown();
    _pushButtonSceneGraphIndexedFaceSetsHide->setEnabled(value);
    value = processor.hasIndexedFaceSetHidden();
    _pushButtonSceneGraphIndexedFaceSetsShow->setEnabled(value);

    value = processor.hasIndexedLineSetShown();
    _pushButtonSceneGraphIndexedLineSetsHide->setEnabled(value);
    value = processor.hasIndexedLineSetHidden();
    _pushButtonSceneGraphIndexedLineSetsShow->setEnabled(value);

    Node* points = wrl->find("POINTS"); // should be a Shape node
    bool  hasPoints = (points!=(Node*)0 && points->isShape());

    std::cout << "  hasPoints = "<< ((hasPoints)?"true":"false") <<"\n";

    if(hasPoints) {

      _pushButtonPointsRemove->setEnabled(true);
      bool show = points->getShow();
      _pushButtonPointsShow->setEnabled(!show);
      _pushButtonPointsHide->setEnabled(show);

      Shape* shape = (Shape*)points;
      bool hasPointNormals = false;
      bool hasPointColors  = false;
      Node* node = shape->getGeometry();
      if(node!=(Node*)0 && node->isIndexedFaceSet()) {
        IndexedFaceSet* ifs = (IndexedFaceSet*)node;
        hasPointNormals = (ifs->getNormalBinding()==IndexedFaceSet::PB_PER_VERTEX);
        hasPointColors  = (ifs->getColorBinding() ==IndexedFaceSet::PB_PER_VERTEX);

        std::cout << "  hasPointsNormals = "
                  << ((hasPointNormals)?"true":"false") <<"\n";
        std::cout << "  hasPointsColors  = "
                  << ((hasPointColors)?"true":"false") <<"\n";
        
        // ifs->printInfo("  ");
      }

      _checkBoxPointsHasNormals->setChecked(hasPointNormals);
      _checkBoxPointsHasColors->setChecked(hasPointColors);
    } else {
      _pushButtonPointsRemove->setEnabled(false);
      _pushButtonPointsShow->setEnabled(false);
      _pushButtonPointsHide->setEnabled(false);
    }

    _pushButtonSurfaceMultiplePlanesFit->setEnabled(hasPoints);
    _pushButtonSurfaceContinuousFit->setEnabled(hasPoints);
    _pushButtonSurfaceWatertightFit->setEnabled(hasPoints);

    vector<float>& fGridVertices = data.getFunctionVertices();
    bool refine = ((int)fGridVertices.size()==nGridVertices);
    _pushButtonOptimalCGHard->setEnabled(refine);
    _pushButtonOptimalCGSoft->setEnabled(refine);
    _pushButtonOptimalJacobi->setEnabled(refine);

    _pushButtonMultigridFiner->setEnabled(refine && (bboxDepth<10));
    _pushButtonMultigridCoarser->setEnabled(refine && (bboxDepth>0));
    _pushButtonMultigridRun->setEnabled(hasPoints);

    Node* edges = wrl->find("EDGES"); // should be a Shape node
    bool  hasEdges = (edges!=(Node*)0 && edges->isShape());
    if(hasEdges) {
      _pushButtonSceneGraphEdgesAdd->setEnabled(false);
      _pushButtonSceneGraphEdgesRemove->setEnabled(true);
      bool show = edges->getShow();
      _pushButtonSceneGraphEdgesShow->setEnabled(!show);
      _pushButtonSceneGraphEdgesHide->setEnabled(show);

    } else {
      _pushButtonSceneGraphEdgesAdd->setEnabled(true);
      _pushButtonSceneGraphEdgesRemove->setEnabled(false);
      _pushButtonSceneGraphEdgesShow->setEnabled(false);
      _pushButtonSceneGraphEdgesHide->setEnabled(false);
    }

    Node* surface    = wrl->find("SURFACE"); // should be a Shape node
    bool  hasSurface = (surface!=(Node*)0 && surface->isShape());
    if(hasSurface) {
      _pushButtonSurfaceRemove->setEnabled(true);
      bool show = surface->getShow();
      _pushButtonSurfaceShow->setEnabled(!show);;
      _pushButtonSurfaceHide->setEnabled(show);;
    } else {
      _pushButtonSurfaceRemove->setEnabled(false);
      _pushButtonSurfaceShow->setEnabled(false);;
      _pushButtonSurfaceHide->setEnabled(false);;
    }
    
    //     // get number of points
    int nPts = 0;
    if(hasPoints) {
      Shape* shape = (Shape*)points;
      Node* node = shape->getGeometry();
      if(node!=0 && node->isIndexedFaceSet()) {
        IndexedFaceSet* ifs = (IndexedFaceSet*)node;
        nPts = ifs->getNumberOfCoord();
      }
    }
    _editPointsNumber->setText("  "+QString::number(nPts));

    // get number of surface vertices and faces
    int nVertices = 0;
    int nFaces = 0;
    if(hasSurface) {
      Shape* shape = (Shape*)surface;
      Node* node = shape->getGeometry();
      if(node!=0 && node->isIndexedFaceSet()) {
        IndexedFaceSet* ifs = (IndexedFaceSet*)node;
        nVertices = ifs->getNumberOfCoord();
        nFaces = ifs->getNumberOfFaces();
      }
    }
    _editSurfaceVertices->setText("  "+QString::number(nVertices));
    _editSurfaceFaces->setText("  "+QString::number(nFaces));

  }

  std::cout << "}\n";

}

void WrlToolsWidget::bboxSetDepth(int depth) {
  WrlViewerData& data = _mainWindow->getData();
  int newDepth = (depth<0)?0:(depth>10)?10:depth;
  int oldDepth = data.getBBoxDepth();
  if(newDepth!=oldDepth) {
    data.setBBoxDepth(newDepth);
    // is scene graph has a BOUNDING-BOX node => rebuild it
    SceneGraph* pWrl = data.getSceneGraph();
    SceneGraphProcessor processor(*pWrl);
    if(processor.hasBBox()) {
      float scale = data.getBBoxScale();
      bool  cube  = data.getBBoxCube();
      processor.bboxAdd(newDepth,scale,cube);
      _mainWindow->setSceneGraph(pWrl,false);
      _mainWindow->refresh();
    }
    updateState();
  }
}

void WrlToolsWidget::bboxDepthUp() {
  WrlViewerData& data = _mainWindow->getData();
  bboxSetDepth(data.getBBoxDepth()+1);
}

void WrlToolsWidget::bboxDepthDown() {
  WrlViewerData& data = _mainWindow->getData();
  bboxSetDepth(data.getBBoxDepth()-1);
}

void WrlToolsWidget::bboxAdd() {
  WrlViewerData& data = _mainWindow->getData();
  int depth = data.getBBoxDepth();
  SceneGraph* pWrl = data.getSceneGraph();
  SceneGraphProcessor processor(*pWrl);
  data.setBBoxDepth(depth);
  float scale = data.getBBoxScale();
  bool  cube  = data.getBBoxCube();
  processor.bboxAdd(depth,scale,cube);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::bboxRemove() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph* pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.bboxRemove();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::bboxScaleEnter() {
  WrlViewerData& data = _mainWindow->getData();
  float scale = data.getBBoxScale();
  QString str = _editBBoxScale->text();
  float value = str.toFloat();
  if(value!=scale) {
    data.setBBoxScale(value);
    SceneGraphProcessor processor(*(data.getSceneGraph()));
    if(processor.hasBBox()) {
      int   depth = data.getBBoxDepth();
      float scale = data.getBBoxScale();
      bool  cube  = data.getBBoxCube();
      processor.bboxAdd(depth,scale,cube);
      _mainWindow->setSceneGraph(data.getSceneGraph(),false);
      _mainWindow->refresh();
      updateState();
    }
  }
}

void WrlToolsWidget::bboxCubeSetState(int state) {
  WrlViewerData& data = _mainWindow->getData();
  data.setBBoxCube((state!=0));
  SceneGraphProcessor processor(*(data.getSceneGraph()));
  if(processor.hasBBox()) {
    int   depth = data.getBBoxDepth();
    float scale = data.getBBoxScale();
    bool  cube  = data.getBBoxCube();
    processor.bboxAdd(depth,scale,cube);
    _mainWindow->setSceneGraph(data.getSceneGraph(),false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::bboxCube() {
  bboxCubeSetState((_checkBoxBBoxCube->isChecked())?2:0);
}

void WrlToolsWidget::normalInvert() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.normalInvert();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::setNormalNone() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.normalClear();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::setNormalPerVertex() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.computeNormalPerVertex();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::setNormalPerFace() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.computeNormalPerFace();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::setNormalPerCorner() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.computeNormalPerCorner();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::pointsRemove() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.pointsRemove();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::pointsShow() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("POINTS");
  if(node==(Node*)0) return;
  node->setShow(true);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::pointsHide() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("POINTS");
  if(node==(Node*)0) return;
  node->setShow(false);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::edgesAdd() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.edgesAdd();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::edgesRemove() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.edgesRemove();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::edgesShow() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("EDGES");
  if(node==(Node*)0) return;
  node->setShow(true);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::edgesHide() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("EDGES");
  if(node==(Node*)0) return;
  node->setShow(false);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::shapeIfsShow() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.shapeIndexedFaceSetShow();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::shapeIfsHide() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.shapeIndexedFaceSetHide();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::shapeIlsShow() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.shapeIndexedLineSetShow();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::shapeIlsHide() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.shapeIndexedLineSetHide();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::surfaceRemove() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    processor.surfaceRemove();
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::surfaceShow() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("SURFACE");
  if(node==(Node*)0) return;
  node->setShow(true);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::surfaceHide() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl==(SceneGraph*)0) return;
  Node* node = pWrl->find("SURFACE");
  if(node==(Node*)0) return;
  node->setShow(false);
  _mainWindow->setSceneGraph(pWrl,false);
  _mainWindow->refresh();
  updateState();
}

void WrlToolsWidget::fitMultiplePlanes() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    vector<float> fVec;
    processor.fitMultiplePlanes(center,size,depth,scale,cube,fVec);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::fitContinuous() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    processor.fitContinuous(center,size,depth,scale,cube);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}

void WrlToolsWidget::fitWatertight() {
  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    vector<float>& fGrid = data.getFunctionVertices();
    fGrid.clear();
    processor.fitWatertight(center,size,depth,scale,cube,fGrid);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }
}


void WrlToolsWidget::optimalCGHard() {
  std::cout << "WrlToolsWidget::optimalCGHard() {\n";

  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    vector<float>& fGrid = data.getFunctionVertices();
    processor.optimalCGHard(center,size,depth,scale,cube,fGrid);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }

  std::cout << "}\n";
}

void WrlToolsWidget::optimalCGSoft() {
  std::cout << "WrlToolsWidget::optimalCGSoft() {\n";

  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    vector<float>& fGrid = data.getFunctionVertices();
    processor.optimalCGSoft(center,size,depth,scale,cube,fGrid);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }

  std::cout << "}\n";
}

void WrlToolsWidget::optimalJacobi() {
  std::cout << "WrlToolsWidget::optimalJacobi() {\n";

  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth = data.getBBoxDepth();
    vector<float>& fGrid = data.getFunctionVertices();
    processor.optimalJacobi(center,size,depth,scale,cube,fGrid);
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }

  std::cout << "}\n";
}

void WrlToolsWidget::multigridFiner() {
  std::cout << "WrlToolsWidget::multigridFiner() {\n";

  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth  = data.getBBoxDepth();
    if(depth<10) {
      vector<float>& fGrid = data.getFunctionVertices();
      processor.multiGridFiner(center,size,depth,scale,cube,fGrid);
      bboxSetDepth(depth+1);
    }
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }

  std::cout << "}\n";
}

void WrlToolsWidget::multigridCoarser() {
  std::cout << "WrlToolsWidget::multigridCoarser() {\n";

  WrlViewerData& data = _mainWindow->getData();
  SceneGraph*    pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    SceneGraphProcessor processor(*pWrl);
    Vec3f& center = pWrl->getBBoxCenter();
    Vec3f& size   = pWrl->getBBoxSize();
    float  scale  = data.getBBoxScale();
    bool   cube   = data.getBBoxCube();
    int    depth  = data.getBBoxDepth();
    if(depth>0) {
      vector<float>& fGrid = data.getFunctionVertices();
      processor.multiGridCoarser(center,size,depth,scale,cube,fGrid);
      bboxSetDepth(depth-1);
    }
    _mainWindow->setSceneGraph(pWrl,false);
    _mainWindow->refresh();
    updateState();
  }

  std::cout << "}\n";
}

void WrlToolsWidget::multigridRun() {
  std::cout << "WrlToolsWidget::multigridRun() {\n";

  WrlViewerData& data = _mainWindow->getData();
  int maxDepth = data.getBBoxDepth();
  bboxSetDepth(0);
  fitWatertight();
  
  for(int depth=1;depth<=maxDepth;depth++) {
    multigridFiner();    
    optimalJacobi();
  }

  SceneGraph* pWrl = data.getSceneGraph();
  if(pWrl!=(SceneGraph*)0) {
    Node* node = pWrl->find("SURFACE"); // should be a Shape node
    if(Shape* shape = dynamic_cast<Shape*>(node)) {
      node = shape->getGeometry();
      if(IndexedFaceSet* ifs = dynamic_cast<IndexedFaceSet*>(node)) {
        ifs->printInfo("  ");
      }
    }
  }

  std::cout << "}\n";
}
