//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-04-30 18:34:37 taubin>
//------------------------------------------------------------------------
//
// LineDetector6.cpp
//
// Copyright (c) 2016, Gabriel Taubin
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

#include <math.h>
#include "LineDetector6.hpp"
#include "Graph.hpp"
#include "Heap.hpp"
#include "Partition.hpp"

//////////////////////////////////////////////////////////////////////
LineDetector6::LineDetector6
(Img&       maskedChecherboardImg,
 Polylines& polylines,
 VecFloat&  checkerboardCorners,
 VecInt&    checkerboardCornersColor,
 bool       extendToBorder,
 int        checkerboardRows,
 int        checkerboardCols,
 int        rowMin,
 int        rowMax):
  _maskedChecherboardImg(maskedChecherboardImg),
  _polylines(polylines),
  _histogram(),
  _checkerboardRows(checkerboardRows),
  _checkerboardCols(checkerboardCols),
  _rowMin(rowMin),
  _rowMax(rowMax) {

  _log(QString("LineDetector6() {"));

  bool joinFlatSegments = true;
      
  Polylines polylinesFlatSegments;
  
  int nV = polylines.getNumberOfVertices();
  int nP = polylines.getNumberOfPolylines();
  int nE = polylines.getNumberOfEdges();
  
  _log(QString("  nV = %1").arg(nV,6));
  _log(QString("  nP = %1").arg(nP,6));
  _log(QString("  nE = %1").arg(nE,6));

  const VecFloat& coord = polylines.getCoord();
  const VecInt&   first = polylines.getFirst();
  const VecInt&   next  = polylines.getNext();

  int widthImg = _maskedChecherboardImg.getWidth();
  int heightImg = _maskedChecherboardImg.getHeight();
  
  float    W = (float)widthImg;
  float    H = (float)heightImg;

  int      i,i0,i1,iP,iP0,iP1,iV,iVp,iVf,iV0,iV1,iVL0,iVL1,jV0,jV1,iPFS;
  int      segment_length,segment_backwards,segment_forward;

  VecInt   vertex;
  VecInt   position(nV,-1);
  VecInt   vertexToFlatSegment(nV,-1);
  VecFloat flatSegmentLinearEquation;
  VecInt   flatSegmentEnds;

  float    x,y,nx,ny,nd, xx0,xx1,yy0,yy1,t;
  float    dist,max_dist,mean_dist;
  bool     closed_path;

  int      min_segment_length = 20;
  float    max_dist_thresh    = 0.95; // 1.1f;
  float    mean_dist_thresh   = 0.1f;

  // convert into doubly linked list
  VecInt prev(nV,-1);
  for(iV0=0;iV0<nV;iV0++)
    if((iV1=next[iV0])>=0)
      prev[iV1] = iV0;

  Graph segments(nV,true /*oriented*/);

  // find flat segments
  for(iP=0;iP<nP;iP++) { // for(each polyline) ...

    _log(QString("  polyline[%1] {").arg(iP));

    vertex.clear();
    position.fill(-1);
    vertexToFlatSegment.fill(-1);
      
    // convert single linked list into vector
    iVf = first[iP];
    position[iVf] = vertex.size();
    vertex.append(iVf);
    for(iV0=iVf;(iV1=next[iV0])>=0 && iV1!=iVf;iV0=iV1) {
      position[iV1] = vertex.size();
      vertex.append(iV1);
    }
    _log(QString("    nV polyline = %1").arg(vertex.size()));
    // determine wether or not this polyline is open or closed
    closed_path = (iV1==iVf);
    _log(QString("    %1 path").arg((closed_path)?"closed":"open"));
    // skip short polylines
    if(vertex.size()<100) {
      _log(QString("    too short ... skipping"));
      continue;
    }

    // search for flat segments
    _log(QString("    flat segments {"));

    for(i=0;i<vertex.size();i++) {

      // find a first vertex not yet assigned to a flat segment
      iVL0 = vertex[i];
      if(vertexToFlatSegment[iVL0]>=0) continue;

      // advance min_segment_length edges
      iVL1=iVL0;
      for(segment_length=0;segment_length<min_segment_length;segment_length++) {
        if(vertexToFlatSegment[iVL1]>=0) break;
        if(next[iVL1]<0) break;
        iVL1 = next[iVL1];
      }

      if(segment_length<min_segment_length) continue;

      do {

        fitLineLeastSquares(iVL0,iVL1,nx,ny,nd,max_dist,mean_dist);

        // try to extend the line support backwards
        segment_backwards = 0;
        for(iVp=iVL0;(iV=prev[iVp])>=0;iVp=iV) {
          if(vertexToFlatSegment[iV]>=0) break;
          x = W*coord[2*iV  ]; y = H*coord[2*iV+1];
          dist = nx*x+ny*y+nd;
          if(fabs(dist)>max_dist_thresh) break;
          segment_backwards++;
        }
        segment_length += segment_backwards;
        if(iVp!=iVL0) iVL0 = iVp;

        // try to extend the line support forward
        segment_forward = 0;
        for(iVp=iVL1;(iV=next[iVp])>=0;iVp=iV) {
          if(vertexToFlatSegment[iV]>=0) break;
          x = W*coord[2*iV  ]; y = H*coord[2*iV+1];
          dist = nx*x+ny*y+nd;
          if(fabs(dist)>max_dist_thresh) break;
          segment_forward++;
        }
        segment_length += segment_forward;
        if(iVp!=iVL1) iVL1 = iVp;

        _log(QString("    [%1]->[%2:%3] %4 (-%5,+%6)")
             .arg(vertex[i],4).arg(iVL0,4).arg(iVL1,4).arg(segment_length,4)
             .arg(segment_backwards).arg(segment_forward));

      } while(segment_backwards>0 || segment_forward>0);

      if(fabs(mean_dist)>mean_dist_thresh || max_dist>max_dist_thresh) {
        _log(QString("  large error"));
      } else if(segments.getEdge(iVL0,iVL1)!=(GraphEdge*)0) {
        _log(QString("  segment exists"));
      } else /* if(segments.getEdge(iVL0,iVL1)==(GraphEdge*)0) */ {

        _log(QString("  NEW segment"));
        _log(QString("    I = [%1:%2] ->(%3,%4,%5)")
             .arg(position[iVL0]).arg(position[iVL1])
             .arg(nx,8).arg(ny,8).arg(nd,8));

        segments.insertEdge(iVL0,iVL1);

        // project first vertex onto fitted line
        xx0 = W*coord[2*iVL0  ]; yy0 = H*coord[2*iVL0+1];
        // project (xx0,yy0) onto line { (x,y) : nx*x+ny*y+nd=0 }
        // find t : nx*(xx0+t*nx)+ny*(yy0+t*ny)+nd=0
        t = -(nx*xx0+ny*yy0+nd); xx0 += t*nx; xx0 /= W; yy0 += t*ny; yy0 /= H;
        
        // project last vertex onto fitted line
        xx1 = W*coord[2*iVL1  ]; yy1 = H*coord[2*iVL1+1];
        // project (xx1,yy1) onto line { (x,y) : nx*x+ny*y+nd=0 }
        // find t : nx*(xx1+t*nx)+ny*(yy1+t*ny)+nd=0
        t = -(nx*xx1+ny*yy1+nd); xx1 += t*nx; xx1 /= W; yy1 += t*ny; yy1 /= H;
        
        iPFS = polylinesFlatSegments.getNumberOfPolylines();
        for(iV=iVL0;iV!=iVL1;iV=next[iV])
          vertexToFlatSegment[iV] = iPFS;

        // save the fitted line segment as a polyline
        jV0 = polylinesFlatSegments.pushBackCoord(xx0,yy0);
        jV1 = polylinesFlatSegments.pushBackCoord(xx1,yy1);
        polylinesFlatSegments.pushBackFirst(jV0);
        polylinesFlatSegments.setNext(jV0,jV1);

        // save the line equation and original segment bounds
        flatSegmentLinearEquation.append(nx);
        flatSegmentLinearEquation.append(ny);
        flatSegmentLinearEquation.append(nd);
        flatSegmentEnds.append(iVL0);
        flatSegmentEnds.append(iVL1);

      } // if(segments.getEdge(iVL0,iVL1)==(GraphEdge*)0)

    } // for(i=0;i<vertex.size();i++)

    _log(QString("    } flat segments "));
    _log(QString("  } polyline"));
    
  } // for(iP=0;iP<nP;iP++)

  int nFlatSegments = polylinesFlatSegments.getNumberOfPolylines();
  _log(QString("  nFlatSegments = %1").arg(nFlatSegments));

  // partition the flat segments
  // look for almost colinear segements and join them 

  VecFloat& pfsCoord = polylinesFlatSegments.getCoord();

  Partition partition(nFlatSegments);

  _log(QString("  number of flat segments = %1").arg(nFlatSegments));
  _log(QString("  pairwise comparison of flat segments {"));

  int   iV00,iV01,iV10,iV11,j,nVpart;
  float xC,x0,x00,x01,x10,x11;
  float yC,y0,y00,y01,y10,y11;
  float tMax,tMin;

  float nx0,ny0,x1,y1,nd0,nx1,ny1,nd1;
  int iVL00,iVL01,iVL10,iVL11,nP0,nP1;

  VecInt vertex0;
  VecInt vertex1;
  VecInt vertexPair;

  for(iP0=0;iP0<nFlatSegments;iP0++) {

    // first segment ends
    iV00  = polylinesFlatSegments.first(0);
    x00   = W*pfsCoord[2*iV00  ];
    y00   = H*pfsCoord[2*iV00+1];
    iV01  = polylinesFlatSegments.next(iV00);
    x01   = W*pfsCoord[2*iV01  ];
    y01   = H*pfsCoord[2*iV01+1];
    // retrieve the linear equation coefficients
    nx0   = flatSegmentLinearEquation[3*iP0  ]; 
    ny0   = flatSegmentLinearEquation[3*iP0+1]; 
    nd0   = flatSegmentLinearEquation[3*iP0+2]; 
    // second segment ends within the list of input vertices
    iVL00 = flatSegmentEnds[2*iP0  ];
    iVL01 = flatSegmentEnds[2*iP0+1];

    // collect all the vertex indices in a first vector
    vertex0.clear();
    for(nP0=0,iV=iVL00;;iV=next[iV]) {
      vertex0.append(iV); nP0++;
      if(iV==iVL01) break;
    }

    for(iP1=iP0+1;iP1<nFlatSegments;iP1++) {

      if(partition.find(iP0)==partition.find(iP1)) continue;

      // second segment ends
      iV10 = polylinesFlatSegments.first(iP1);
      x10  = W*pfsCoord[2*iV10  ];
      y10  = H*pfsCoord[2*iV10+1];
      iV11 = polylinesFlatSegments.next(iV10);
      x11  = W*pfsCoord[2*iV11  ];
      y11  = H*pfsCoord[2*iV11+1];
      // retrieve the linear equation coefficients
      nx1 = flatSegmentLinearEquation[3*iP1  ]; 
      ny1 = flatSegmentLinearEquation[3*iP1+1]; 
      nd1 = flatSegmentLinearEquation[3*iP1+2]; 
      // second segment ends within the list of input vertices
      iVL10 = flatSegmentEnds[2*iP1  ];
      iVL11 = flatSegmentEnds[2*iP1+1];

      // collect all the vertex indices in a second vector
      vertex1.clear();
      for(nP1=0,iV=iVL10;;iV=next[iV]) {
        vertex1.append(iV); nP1++;
        if(iV==iVL11) break;
      }

      // concatenate the two lists of input vertices
      vertexPair.clear();
      vertexPair << vertex0;
      vertexPair << vertex1;

      if(fitLineLeastSquares(vertexPair,nx,ny,nd,max_dist,mean_dist)<0)
        continue;

      _log(QString("  iP0=%1 & iP1=%2 {").arg(iP0).arg(iP1));

      if(joinFlatSegments &&
         fabs(mean_dist)<=1.75f*mean_dist_thresh &&
         max_dist<=1.75f*max_dist_thresh) {
        _log(QString("    --> joining"));
        partition.join(iP0,iP1);
      } else {
        _log(QString("    --> skipping"));
      }

      _log(QString("  }"));

    } // for(iP1=iP0+1;iP1<nFlatSegments;iP1++)
  } // for(iP0=0;iP0<nFlatSegments;iP0++)

  _log(QString("  }"));

  partition.makeParts();
  int nParts = partition.getNumberOfParts();
  _log(QString("  nParts = %1 {").arg(nParts));

  Polylines polylinesPartition;
  VecInt    part,vertexPart;
  VecFloat  partLinearEquation;
  for(i=0;i<nParts;i++) {
    partition.getPart(i,part);
    nVpart = part.size();

    _log(QString("    nParts[%1] = %2").arg(i,3).arg(nVpart,3));

    // collect all the vertex indices in a single vector
    vertexPart.clear();
    for(j=0;j<nVpart;j++) {
      iP  = part[j];
      iV0 = flatSegmentEnds[2*iP  ];
      iV1 = flatSegmentEnds[2*iP+1];
      for(iV=iV0;;iV=next[iV]) {
        vertexPart.append(iV);
        if(iV==iV1) break;
      }
    }

    _log(QString("      nPoints = %1").arg(vertexPart.size()));

    if(fitLineLeastSquares(vertexPart,nx,ny,nd,max_dist,mean_dist)<0) continue;

    partLinearEquation.append(nx);
    partLinearEquation.append(ny);
    partLinearEquation.append(nd);

    _log(QString("      (nx ,ny ,nd ) = (%1,%2,%3)").arg(nx,8).arg(ny,8).arg(nd,8));
    _log(QString("      max=%1 mean=%2").arg(max_dist,8).arg(mean_dist,8));

    // compute centroid of supporting points
    xC = yC = 0;
    for(j=0;j<vertexPart.size();j++) {
      iV = vertexPart[j];
      xC += W*coord[2*iV  ];
      yC += H*coord[2*iV+1];
    }
    xC /= ((float)(vertexPart.size()));
    yC /= ((float)(vertexPart.size()));
    // project centroid onto fitted line
    // (xC,yC)=(xC+t*nx,yC+t*ny)
    // where t is such that nx*(xC+t*nx)+ny*(yC+t*ny)+nd = 0;
    t = -(nx*xC+ny*yC+nd); xC += t*nx; yC += t*ny;
      
    tMax = 0; tMin = 1;
    for(j=0;j<vertexPart.size();j++) {
      iV = vertexPart[j];
      x  = W*coord[2*iV  ];
      y  = H*coord[2*iV+1];
      t = ny*(x-xC)-nx*(y-yC);
      if(j==0) { tMin = tMax = t; }
      if(t<tMin) tMin = t;
      if(t>tMax) tMax = t;
    }

    xx0 = (xC+tMin*ny)/W; yy0 = (yC-tMin*nx)/H;
    xx1 = (xC+tMax*ny)/W; yy1 = (yC-tMax*nx)/H;

    if(extendToBorder) extendSegmentToBorder(xx0,yy0,xx1,yy1);

    jV0 = polylinesPartition.pushBackCoord(xx0,yy0);
    jV1 = polylinesPartition.pushBackCoord(xx1,yy1);
    polylinesPartition.pushBackFirst(jV0);
    polylinesPartition.setNext(jV0,jV1);

  } // for(i=0;i<nParts;i++)

  _log(QString("  }"));

  // now determine the checkerboard corner coordinates
  checkerboardCorners.clear();
  checkerboardCornersColor.clear();
  if(partLinearEquation.size()==3*nParts &&
     nParts==(_checkerboardRows+1)+(_checkerboardCols+1)) {
    // we should find a group of (_checkerboardRows+1) lines close to
    // parallel to each other, and another group of (_checkerboardCols+1)

    // partLinearEquation.append(nx);
    // partLinearEquation.append(ny);
    // partLinearEquation.append(nd);

    // symmetric matric represented as a lower triangular matrix
    // C[row][col] = C[(row*(row+1)/2)+col] if 0<=col<=row
    // C[row][col] = C[(col*(col+1)/2)+row] if 0<=row<=col
    VecFloat Cos(nParts*(nParts-1)/2,0.0f);

    Partition p(nParts);

    // row
    for(i=iP0=0;iP0<nParts;iP0++) {

      nx0 = partLinearEquation[3*iP0  ];
      ny0 = partLinearEquation[3*iP0+1];
      nd0 = partLinearEquation[3*iP0+2];

      // col
      for(iP1=0;iP1<iP0;iP1++,i++) {

        nx1 = partLinearEquation[3*iP1  ];
        ny1 = partLinearEquation[3*iP1+1];
        nd1 = partLinearEquation[3*iP1+2];

        Cos[i] = nx0*nx1+ny0*ny1;

        _log(QString(" Cos[%1][%2] = %3")
             .arg(iP0,2).arg(iP1,2).arg(Cos[i],8));

        if(fabs(Cos[i])>0.95f) p.join(iP0,iP1);
          
      }
    }

    int nLineGroups = p.getNumberOfParts();
    _log(QString(" nLineGroups = %1").arg(nLineGroups));
    if(nLineGroups==2) {
      p.makeParts();
      VecInt group0,group1;
      p.getPart(0,group0);
      p.getPart(1,group1);

      if((group0.size()==_checkerboardRows+1 &&
          group1.size()==_checkerboardCols+1) ||
         (group0.size()==_checkerboardCols+1 &&
          group1.size()==_checkerboardRows+1)) {

        if(group0.size()==_checkerboardCols+1) group0.swap(group1);

        VecInt& rowLines  = group0;
        VecInt& colLines  = group1;
        int     nRows = rowLines.size(); // == _checkerboardRows+1
        int     nCols = colLines.size(); // == _checkerboardCols+1

        _log(QString("  nRows = %1").arg(nRows));
        for(i=0;i<nRows;i++)
          _log(QString("    rowLine[%1] =  %2").arg(i).arg(rowLines[i]));
        _log(QString("  }"));

        _log(QString("  nCols = %1 {").arg(nCols));
        for(i=0;i<nCols;i++)
          _log(QString("    colLine[%1] =  %2").arg(i).arg(colLines[i]));
        _log(QString("  }"));

        // find the point where the first row intersects the first col
        iP0 = rowLines[0];
        nx0 = partLinearEquation[3*iP0  ];
        ny0 = partLinearEquation[3*iP0+1];
        nd0 = partLinearEquation[3*iP0+2];

        iP1 = colLines[0];
        nx1 = partLinearEquation[3*iP1  ];
        ny1 = partLinearEquation[3*iP1+1];
        nd1 = partLinearEquation[3*iP1+2];

        float w,vCx,vCy,vRx,vRy;
        int row,col;

        w  = ( nx0*ny1-nx1*ny0)  ;
        y0 = (-nx0*nd1+nx1*nd0)/w;
        x0 = ( ny0*nd1-ny1*nd0)/w;

        _log(QString("    xyRC = (%1,%2)")
             .arg(x0,8).arg(y0,8));


        // parametric equation of rowLine[0]
        // (x,y) = (x0,y0)+t*(vRx,vRy)
        vRx =  ny0;
        vRy = -nx0;

        // parametric equation of colLine[0]
        // (x,y) = (x0,y0)+t*(vCx,vCy)
        vCx =  ny1;
        vCy = -nx1;

        Heap heapCol;
        for(i=0;i<nCols;i++) {
          iP1 = colLines[i];
          nx1 = partLinearEquation[3*iP1  ];
          ny1 = partLinearEquation[3*iP1+1];
          nd1 = partLinearEquation[3*iP1+2];
          // compute the intersection of colLines[i] with rowLines[0]
          // t : nx1*(x0+t*vRx)+ny1*(y0+t*vRy)+nd1=0
          // t : (nx1*x0+ny1*y0+nd1)+t*(nx1*vRy+ny1*vRx)=0
          t = -(nx1*x0+ny1*y0+nd1)/(nx1*vRx+ny1*vRy);
          heapCol.add(t,iP1);
        }

        VecFloat tCol;
        colLines.clear();
        while(heapCol.length()>0) {
          i   = heapCol.delMin();
          t   = heapCol.getLastFKey();
          iP1 = heapCol.getLastIKey();
          colLines.append(iP1);
          tCol.append(t);
        }

        _log(QString("  {"));
        for(col=0;col<nCols;col++) {
          _log(QString("    tCol[%1] =  %2").arg(colLines[col]).arg(tCol[col]));
        }
        _log(QString("  }"));

        Heap heapRow;
        for(i=0;i<nRows;i++) {
          iP0 = rowLines[i];
          nx0 = partLinearEquation[3*iP0  ];
          ny0 = partLinearEquation[3*iP0+1];
          nd0 = partLinearEquation[3*iP0+2];
          // compute the intersec\tion of rowLines[i] with colLines[0]
          // t : nx0*(x0+t*vCx)+ny0*(y0+t*vCy)+nd0=0
          // t : (nx0*x0+ny0*y0+nd0)+t*(nx0*vCx+ny0*vCy)=0
          t = -(nx0*x0+ny0*y0+nd0)/(nx0*vCx+ny0*vCy);
          heapRow.add(t,iP0);
        }

        VecFloat tRow;
        rowLines.clear();
        while(heapRow.length()>0) {
          i   = heapRow.delMin();
          t   = heapRow.getLastFKey();
          iP0 = heapRow.getLastIKey();
          rowLines.append(iP0);
          tRow.append(t);
        }

        _log(QString("  {"));
        for(row=0;row<nRows;row++) {
          _log(QString("    tRow[%1] =  %2").arg(rowLines[row]).arg(tRow[row]));
        }           
        _log(QString("  }"));

        _log(QString("  checkerboard corners {"));
        // compute the checherboard corner coordinates
        for(i=0,row=0;row<nRows;row++) {
          iP0 = rowLines[row];
          nx0 = partLinearEquation[3*iP0  ];
          ny0 = partLinearEquation[3*iP0+1];
          nd0 = partLinearEquation[3*iP0+2];

          for(col=0;col<nCols;col++,i++) {
            iP1 = colLines[col];
            nx1 = partLinearEquation[3*iP1  ];
            ny1 = partLinearEquation[3*iP1+1];
            nd1 = partLinearEquation[3*iP1+2];

            // intersection point
            w = ( nx0*ny1-nx1*ny0)  ;
            y = (-nx0*nd1+nx1*nd0)/w;
            x = ( ny0*nd1-ny1*nd0)/w;

            // _log(QString("    xy[%1][%2] = (%3,%4)")
            //      .arg(row,2).arg(col,2).arg(x,8).arg(y,8));

            // need coordinates normalized to [0:1]x[0:2]
            checkerboardCorners.append(x/W);
            checkerboardCorners.append(y/H);
          }
        }
        _log(QString("  }"));

          _log(QString("  checking orientation ..."));
        // check orientation and fix if necessary
        i0 = 1; // second vertex of the first row v2=[0->i0]
        x0 = checkerboardCorners[2*i0  ]-checkerboardCorners[2*0  ];
        y0 = checkerboardCorners[2*i0+1]-checkerboardCorners[2*0+1];
        i1 = nCols;  // first vertex of the second row v1=[0->i1]
        x1 = checkerboardCorners[2*i1  ]-checkerboardCorners[2*0  ];
        y1 = checkerboardCorners[2*i1+1]-checkerboardCorners[2*0+1];
        if(x0*y1-x1*y0<0) { // if sign of vector product (v2)x(v1) is negative
          _log(QString("  orientation is inverted ... correcting"));
          // mirror transform along columns
          for(row=0;row<nRows;row++) {
            for(col=0;col<nCols-1-col;col++) {
              // swap columns [col] & [nCols-1-col]
              i0 = (        col)+(row)*nCols;
              i1 = (nCols-1-col)+(row)*nCols;
              x = checkerboardCorners[2*i0  ];
              checkerboardCorners[2*i0  ] = checkerboardCorners[2*i1  ];
              checkerboardCorners[2*i1  ] = x;
              y = checkerboardCorners[2*i0+1];
              checkerboardCorners[2*i0+1] = checkerboardCorners[2*i1+1];
              checkerboardCorners[2*i1+1] = y;
            }
          }
        } else {
          _log(QString("  orientation is correct"));
        }

        _log(QString("  checking origin color ... "));

        // the origin should be a corner of a black square
        // and the oposite end should be a corner of a white square

        int pix0,pix1,gr0,gr1,r,g,b;

        // center of first square
        //          col=0     col=1
        //            |         |
        // row=0 ---  0 ------- 1
        //            |    X    |
        // row=1 --- nCols -- nCols+1

        x00 = W*checkerboardCorners[2*((0)*nCols+(0))  ];
        x01 = W*checkerboardCorners[2*((0)*nCols+(1))  ];
        x10 = W*checkerboardCorners[2*((1)*nCols+(0))  ];
        x11 = W*checkerboardCorners[2*((1)*nCols+(1))  ];
        x = (x00+x01+x10+x11)/4.0f;
        y00 = H*checkerboardCorners[2*((0)*nCols+(0))+1];
        y01 = H*checkerboardCorners[2*((0)*nCols+(1))+1];
        y10 = H*checkerboardCorners[2*((1)*nCols+(0))+1];
        y11 = H*checkerboardCorners[2*((1)*nCols+(1))+1];
        y = (y00+y01+y10+y11)/4.0f;
        pix0 = 0xffffff&maskedChecherboardImg.get((int)x,(int)y);
        r = pix0; b = r&0xff; r>>=8; g = r&0xff; r>>=8; gr0 = r+g+b;
        _log(QString("  pix0(%1,%2) = 0x%3 -> %4")
             .arg((int)x,4).arg((int)y,4).arg(pix0,4,16,QChar('0')).arg(gr0));

        // center of last square
        //                    col=(nCols-2)     col=(nCols-1)
        //                          |                  |
        // row=nRows-2 --- (nRows-2)*nCols -- (nRows-2)*nCols+1
        //                          |          X       |
        // row-nRows-1 --- (nRows-1)*nCols -- (nRows-1)*nCols+1
        
        x00 = W*checkerboardCorners[2*((nRows-2)*nCols+(nCols-2))  ];
        x01 = W*checkerboardCorners[2*((nRows-2)*nCols+(nCols-1))  ];
        x10 = W*checkerboardCorners[2*((nRows-1)*nCols+(nCols-2))  ];
        x11 = W*checkerboardCorners[2*((nRows-1)*nCols+(nCols-1))  ];
        x = (x00+x01+x10+x11)/4.0f;
        y00 = H*checkerboardCorners[2*((nRows-2)*nCols+(nCols-2))+1];
        y01 = H*checkerboardCorners[2*((nRows-2)*nCols+(nCols-1))+1];
        y10 = H*checkerboardCorners[2*((nRows-1)*nCols+(nCols-2))+1];
        y11 = H*checkerboardCorners[2*((nRows-1)*nCols+(nCols-1))+1];
        y = (y00+y01+y10+y11)/4.0f;
        pix1 = 0xffffff&maskedChecherboardImg.get((int)x,(int)y);
        r = pix1; b = r&0xff; r>>=8; g = r&0xff; r>>=8; gr1 = r+g+b;
        _log(QString("  pix1(%1,%2) = 0x%3 -> %4")
             .arg((int)x,4).arg((int)y,4).arg(pix1,4,16,QChar('0')).arg(gr1));

        if(gr0>gr1) {
          _log(QString("  origin color is inverted ... correcting"));

          // mirror transform columns     
          for(row=0;row<nRows;row++) {
            for(col=0;col<nCols-1-col;col++) {
              // swap columns [col] & [nCols-1-col]
              i0 = (        col)+(row)*nCols;
              i1 = (nCols-1-col)+(row)*nCols;
              x = checkerboardCorners[2*i0  ];
              checkerboardCorners[2*i0  ] = checkerboardCorners[2*i1  ];
              checkerboardCorners[2*i1  ] = x;
              y = checkerboardCorners[2*i0+1];
              checkerboardCorners[2*i0+1] = checkerboardCorners[2*i1+1];
              checkerboardCorners[2*i1+1] = y;
            }
          }
          // mirror transform rows
          for(row=0;row<nRows-1-row;row++) {
            for(col=0;col<nCols;col++) {
              // swap columns [col] & [nCols-1-col]
              i0 = (col)+(        row)*nCols;
              i1 = (col)+(nRows-1-row)*nCols;
              x = checkerboardCorners[2*i0  ];
              checkerboardCorners[2*i0  ] = checkerboardCorners[2*i1  ];
              checkerboardCorners[2*i1  ] = x;
              y = checkerboardCorners[2*i0+1];
              checkerboardCorners[2*i0+1] = checkerboardCorners[2*i1+1];
              checkerboardCorners[2*i1+1] = y;
            }
          }

        } else {
          _log(QString("  origin color is correct"));
        }

        // assign corner colors linearly interpolating
        // the four corners
        //
        //               0         nCols-1
        //               |           |    
        //       0 ---- RED --->--- BLUE
        //               |           |
        // nRows-1 -- GREEN -->-- YELLOW

        int r0,r1,r00,r01,r10,r11;
        int g0,g1,g00,g01,g10,g11;
        int b0,b1,b00,b01,b10,b11;
        int argb;
        // [    0,    0] RED
        r00 = 0xff; g00 = 0x00; b00 = 0x00;
        // [nCols,    0] GREEN
        r01 = 0x00; g01 = 0xff; b01 = 0x00;
        // [    0,nRows] BLUE
        r10 = 0x00; g10 = 0x00; b10 = 0xff;
        // [nCols,nRows] YELLOW
        r11 = 0xff; g11 = 0xff; b11 = 0x00;
        for(row=0;row<nRows;row++) {
          r0 = ((nRows-1-row)*r00+(row)*r01)/(nRows-1);
          g0 = ((nRows-1-row)*g00+(row)*g01)/(nRows-1);
          b0 = ((nRows-1-row)*b00+(row)*b01)/(nRows-1);
          r1 = ((nRows-1-row)*r10+(row)*r11)/(nRows-1);
          g1 = ((nRows-1-row)*g10+(row)*g11)/(nRows-1);
          b1 = ((nRows-1-row)*b10+(row)*b11)/(nRows-1);
          for(col=0;col<nCols;col++) {
            // bilinear interpolation
            r = ((nCols-1-col)*r0+(col)*r1)/(nCols-1);
            g = ((nCols-1-col)*g0+(col)*g1)/(nCols-1);
            b = ((nCols-1-col)*b0+(col)*b1)/(nCols-1);
            argb = 0xff000000 | (r<<16) | (g<<8) | (b<<0);
            checkerboardCornersColor.append(argb);
          }
        }

      }

    } // if(nLineGroups==2)
  }

  // replace input polylines by fitted line segments  
  // polylines.swap(polylinesFlatSegments);
  polylines.swap(polylinesPartition);
  
  _log(QString("}"));
}

//////////////////////////////////////////////////////////////////////
int LineDetector6::testLineFit
(const int iVL0, const int iVL1,
 const float nx, const float ny, const float nd,
 float& max_dist, float& mean_dist) {

  int nV = _polylines.getNumberOfVertices();
  if(iVL0<0 || iVL0>=nV) return -1;
  if(iVL1<0 || iVL1>=nV) return -1;
  if(iVL0==iVL1) return -1;

  const VecFloat& coord = _polylines.getCoord();
  const VecInt&   next  = _polylines.getNext();
    const float W = (float)_maskedChecherboardImg.getWidth();
    const float H = (float)_maskedChecherboardImg.getHeight();

  int iV,n_edges,n_vertices;
  float x,y,dist;
  // make sure that iVL1 can be reached from iVL0
  for(n_edges=0,iV=iVL0;iV>=0 && iV!=iVL1;iV=next[iV],n_edges++);
  if(iV!=iVL1) return -1;
  n_vertices = n_edges+1;

  // implicit equation of line is
  // { (x,y) : nx*x+ny*y+nd=0 }
  // signed distance, in pixel units, from point (x,y) to line is
  // sign_dist(x,y) = nx*x+ny*y+nd

  // maximum distance from internal vertices to the line [(x0,y0):(x1,y1)]
  mean_dist = 0.0f;
  max_dist  = 0.0f;
  for(iV=iVL0;;iV=next[iV]) {
    x = W*coord[2*iV  ];
    y = H*coord[2*iV+1];
    dist = nx*x+ny*y+nd;
    mean_dist += dist;
    dist = fabs(dist);
    if(dist>max_dist) max_dist = dist;
    if(iV==iVL1) break;
  }
  mean_dist /= ((float)n_vertices-1);

  return 0;
}

int LineDetector6::testLineFit
(const VecInt& vertex,
 const float nx, const float ny, const float nd,
 float& max_dist, float& mean_dist) {

  const VecFloat& coord = _polylines.getCoord();
    const float W = (float)_maskedChecherboardImg.getWidth();
    const float H = (float)_maskedChecherboardImg.getHeight();

  int   i,iV,n_vertices;
  float x,y,dist;
  // make sure that iVL1 can be reached from iVL0
  n_vertices = vertex.size();

  // implicit equation of line is
  // { (x,y) : nx*x+ny*y+nd=0 }
  // signed distance, in pixel units, from point (x,y) to line is
  // sign_dist(x,y) = nx*x+ny*y+nd

  // maximum distance from internal vertices to the line [(x0,y0):(x1,y1)]
  mean_dist = 0.0f;
  max_dist  = 0.0f;
  for(i=0;i<n_vertices;i++) {
    iV = vertex[i];
    x = W*coord[2*iV  ];
    y = H*coord[2*iV+1];
    dist = nx*x+ny*y+nd;
    mean_dist += dist;
    dist = fabs(dist);
    if(dist>max_dist) max_dist = dist;
  }
  mean_dist /= ((float)n_vertices-1);

  return 0;
}

//////////////////////////////////////////////////////////////////////
int LineDetector6::fitLineFixedEnds
(int iVL0, int iVL1,
 float& nx, float& ny, float& nd, float& max_dist, float& mean_dist) {

  int nV = _polylines.getNumberOfVertices();
  if(iVL0<0 || iVL0>=nV) return -1;
  if(iVL1<0 || iVL1>=nV) return -1;
  if(iVL0==iVL1) return -1;

  const VecFloat& coord = _polylines.getCoord();
  const VecInt&   next  = _polylines.getNext();
    const float W = (float)_maskedChecherboardImg.getWidth();
    const float H = (float)_maskedChecherboardImg.getHeight();

  int iV,n_edges,n_vertices;
  float x,x0,x1,y,y0,y1;
  // make sure that iVL1 can be reached from iVL0
  for(n_edges=0,iV=iVL0;iV>=0 && iV!=iVL1;iV=next[iV],n_edges++);
  if(iV!=iVL1) return -1;
  n_vertices = n_edges+1;

  // initialize the line to the one defined by iVL0 and iVL1

  // pixel coordinates of path segment ends 
  x0 = W*coord[2*iVL0  ];
  y0 = H*coord[2*iVL0+1];
  x1 = W*coord[2*iVL1  ];
  y1 = H*coord[2*iVL1+1];
  // unit length normal vector to the line
  // defined by the path segment ends (x0,y0)-(x1,y1)
  nx = y1-y0;
  ny = x0-x1;
  nd = sqrt(nx*nx+ny*ny);
  nx /= nd;
  ny /= nd;
  if(ny<0.0f) {
    nx = -nx; ny = -ny;
  } else if(ny==0.0f && nx<0.0f) {
    nx = -nx;
  }
  // centroid of path segment internal vertices
  x0 = y0 = 0.0f;
  for(iV=iVL0;;iV=next[iV]) {
    x = W*coord[2*iV  ]; x0 += x;
    y = H*coord[2*iV+1]; y0 += y;
    if(iV==iVL1) break;
  }
  x0 /= (float)n_vertices;
  y0 /= (float)n_vertices;
  nd = -(nx*x0+ny*y0);

  // implicit equation of line is
  // { (x,y) : nx*(x-x0)+ny*(y-y0)=0 }
  // signed distance, in pixel units, from point (x,y) to line is
  // sign_dist(x,y) = nx*(x-x0)+ny*(y-y0)

  return testLineFit(iVL0,iVL1, nx, ny, nd, max_dist, mean_dist);
}

//////////////////////////////////////////////////////////////////////
int LineDetector6::fitLineLeastSquares
(int iVL0, int iVL1,
 float& nx, float& ny, float& nd, float& max_dist, float& mean_dist) {

  int nV = _polylines.getNumberOfVertices();
  if(iVL0<0 || iVL0>=nV) return -1;
  if(iVL1<0 || iVL1>=nV) return -1;
  if(iVL0==iVL1) return -1;

  const VecFloat& coord = _polylines.getCoord();
  const VecInt&   next  = _polylines.getNext();
    const float W = (float)_maskedChecherboardImg.getWidth();
    const float H = (float)_maskedChecherboardImg.getHeight();

  int iV,n_edges,n_vertices;
  float dx,x,x0,dy,y,y0,Mxx,Mxy,Myy,a,b,c,t;
  // make sure that iVL1 can be reached from iVL0
  for(n_edges=0,iV=iVL0;iV>=0 && iV!=iVL1;iV=next[iV],n_edges++);
  if(iV!=iVL1) return -1;
  n_vertices = n_edges+1;

  // centroid of path segment internal vertices
  x0 = y0 = 0.0f;
  for(iV=iVL0;;iV=next[iV]) {
    x = W*coord[2*iV  ];
    y = H*coord[2*iV+1];
    x0 += x;
    y0 += y;
    if(iV==iVL1) break;
  }
  x0 /= (float)n_vertices;
  y0 /= (float)n_vertices;

  // build scatter matrix
  Mxx = Mxy = Myy = 0.0f;
  for(iV=iVL0;;iV=next[iV]) {
    dx = W*coord[2*iV  ]-x0;
    dy = H*coord[2*iV+1]-y0;
    Mxx += dx*dx;
    Mxy += dx*dy;
    Myy += dy*dy;
    if(iV==iVL1) break;
  }
  Mxx /= (float)n_vertices;
  Mxy /= (float)n_vertices;
  Myy /= (float)n_vertices;

  // compute the minimum (positive) eigenvalue of M
  a = (Mxx+Myy)/2.0f;
  b = (Mxx-Myy)/2.0f;
  c = sqrt(b*b+Mxy*Mxy);
  t = a-c;
  // second eigenvalue is t = a+c;
  // compute (nx,ny) as eigenvector associated with the minimum eigenvalue
  // notet that the two rows of M-t*I are linearly dependent
  // | Mxx-t Mxy   ||nx| = 0
  // | Mxy   Myy-t ||ny|
  a = Mxx-t;
  b = Myy-t;
  if(a*a>b*b) {
    nx = Mxy; ny = t-Mxx;
  } else {
    nx = t-Myy; ny = Mxy;
  }
  a = sqrt(nx*nx+ny*ny);
  nx /= a; ny /= a;
  if(ny<0.0f) {
    nx = -nx; ny = -ny;
  } else if(ny==0.0f && nx<0.0f) {
    nx = -nx;
  }
  // compute the constant term of the linear equation
  nd = -(nx*x0+ny*y0);

  // implicit equation of line is
  // { (x,y) : nx*x+ny*y+nd=0 }
  // signed distance, in pixel units, from point (x,y) to line is
  // sign_dist(x,y) = nx*x+ny*y+nd

  return testLineFit(iVL0,iVL1, nx, ny, nd, max_dist, mean_dist);
}

int LineDetector6::fitLineLeastSquares
(const VecInt& vertex,
 float& nx, float& ny, float& nd, float& max_dist, float& mean_dist) {

  const VecFloat& coord = _polylines.getCoord();
  const float W = (float)_maskedChecherboardImg.getWidth();
  const float H = (float)_maskedChecherboardImg.getHeight();

  int   i,iV,n_vertices;
  float dx,x,x0,dy,y,y0,Mxx,Mxy,Myy,a,b,c,t;
  n_vertices = vertex.size();

  if(n_vertices<2) return -1;

  // centroid of path segment internal vertices
  x0 = y0 = 0.0f;
  for(i=0;i<n_vertices;i++) {
    iV = vertex[i];
    x = W*coord[2*iV  ];
    y = H*coord[2*iV+1];
    x0 += x;
    y0 += y;
  }
  x0 /= (float)n_vertices;
  y0 /= (float)n_vertices;

  // build scatter matrix
  Mxx = Mxy = Myy = 0.0f;
  for(i=0;i<n_vertices;i++) {
    iV = vertex[i];
    dx = W*coord[2*iV  ]-x0;
    dy = H*coord[2*iV+1]-y0;
    Mxx += dx*dx;
    Mxy += dx*dy;
    Myy += dy*dy;
  }
  Mxx /= (float)n_vertices;
  Mxy /= (float)n_vertices;
  Myy /= (float)n_vertices;

  // compute the minimum (positive) eigenvalue of M
  a = (Mxx+Myy)/2.0f;
  b = (Mxx-Myy)/2.0f;
  c = sqrt(b*b+Mxy*Mxy);
  t = a-c;
  // second eigenvalue is t = a+c;
  // compute (nx,ny) as eigenvector associated with the minimum eigenvalue
  // notet that the two rows of M-t*I are linearly dependent
  // | Mxx-t Mxy   ||nx| = 0
  // | Mxy   Myy-t ||ny|
  a = Mxx-t;
  b = Myy-t;
  if(a*a>b*b) {
    nx = Mxy; ny = t-Mxx;
  } else {
    nx = t-Myy; ny = Mxy;
  }
  a = sqrt(nx*nx+ny*ny);
  nx /= a; ny /= a;
  if(ny<0.0f) {
    nx = -nx; ny = -ny;
  } else if(ny==0.0f && nx<0.0f) {
    nx = -nx;
  }
  // compute the constant term of the linear equation
  nd = -(nx*x0+ny*y0);

  // implicit equation of line is
  // { (x,y) : nx*x+ny*y+nd=0 }
  // signed distance, in pixel units, from point (x,y) to line is
  // sign_dist(x,y) = nx*x+ny*y+nd

  return testLineFit(vertex, nx, ny, nd, max_dist, mean_dist);
}

//////////////////////////////////////////////////////////////////////
void LineDetector6::extendSegmentToBorder
(float& xx0, float& yy0, float& xx1, float& yy1) {
  const float W = (float)_maskedChecherboardImg.getWidth();
  const float H = (float)_maskedChecherboardImg.getHeight();
  float xt = W*xx0;
  float yt = H*yy0;
  float dx = W*(xx1-xx0);
  float dy = H*(yy1-yy0);
  if(dy<0.0f) {
    dx = -dx; dy = -dy;
  } else if(dy==0.0f && dx<0.0f) {
    dx = -dx;
  }

  float tNeg,tPos,t0,t1,t2,t3,t;

  tNeg = 0.0f;
  tPos = 0.0f;
  if(dy!=0.0f) { // vertical line
    // t0 : yt+t0*dy == rowMin+0.5;
    tNeg = t0 = (((float)_rowMin)+0.5f-yt)/dy; // t0<0
    // t1 : yt+t1*dy == rowMax-0.5;
    tPos = t1 = (((float)_rowMax)-0.5f-yt)/dy; // t1>0
  }
  if(dx!=0.0f) { // horizontal line
    // t2 : xt+t2*dx == 0.5;
    t2 = (0.5f-xt)/dx;
    // t3 : xt+t3*dx == W-0.5;
    t3 = (W-0.5f-xt)/dx;
    // make sure that t2<0 and t3>0
    if(dx<0.0f) { t = t2; t2 = t3; t3 = t; }
    if(t2>tNeg) tNeg = t2;
    if(t3<tPos) tPos = t3;
  }

  xx0 = (xt+tNeg*dx)/W; yy0 = (yt+tNeg*dy)/H;
  xx1 = (xt+tPos*dx)/W; yy1 = (yt+tPos*dy)/H;
}
