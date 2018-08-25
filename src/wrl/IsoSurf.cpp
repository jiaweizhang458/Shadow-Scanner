//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 09:50:20 taubin>
//------------------------------------------------------------------------
//
// IsoSurf.cpp
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

#include "IsoSurf.h"

const int IsoSurf::_faceTable[12][12] = {
  // i=0                       // i=1
  //   1-(01)-5                //   3-(03)-7
  // (08)    (10)              // (05)    (07)
  //   0-(00)-4                //   1-(01)-5
  // (04)    (06)              // (08)    (10)
  //   2-(02)-6                //   0-(00)-4
  { 0,8,1,1,5,10,4,6,6,2,2,4}, { 1,5,3,3,7,7,5,10,4,0,0,8},
  // i=2                       // i=3
  //   0-(00)-4                //   2-(02)-6
  // (04)    (06)              // (09)    (11)
  //   2-(03)-6                //   3-(04)-7
  // (09)    (11)              // (05)    (07)
  //   3-(03)-7                //   1-(01)-5
  { 2,4,0,0,4,6,6,11,7,3,3,9}, { 3,9,2,2,6,11,7,7,5,1,1,5},
  // i=4                       // i=5
  //   4-(06)-6                //   0-(04)-2
  // (00)    (02)              // (08)    (09)
  //   0-(04)-2                //   1-(05)-3
  // (08)    (09)              // (01)    (03)
  //   1-(05)-3                //   5-(07)-7
  { 0,0,4,6,6,2,2,9,3,5,1,8},  { 1,8,0,4,2,9,3,3,7,7,5,1},
  // i=6                       // i=7
  //   5-(07)-7                //   1-(05)-3
  // (10)    (11)              // (01)    (03)
  //   4-(06)-6                //   5-(07)-7
  // (00)    (02)              // (10)    (11)
  //   0-(04)-2                //   4-(06)-6
  { 4,10,5,7,7,11,6,2,2,4,0,0},{ 5,1,1,5,3,3,7,11,6,6,4,10},
  // i=8                       // i=9
  //   2-(09)-3                //   6-(11)-7
  // (04)    (05)              // (02)    (03)
  //   0-(08)-1                //   2-(09)-3
  // (00)    (01)              // (04)    (05)
  //   4-(10)-5                //   0-(08)-1
  { 0,4,2,9,3,5,1,1,5,10,4,0}, { 2,2,6,11,7,3,3,5,1,8,0,4},
  // i=10                      // i=11
  //   0-(08)-1                //   4-(10)-5
  // (00)    (01)              // (06)    (07)
  //   4-(10)-5                //   6-(11)-7
  // (06)    (07)              // (02)    (03)
  //   6-(11)-7                //   2-(09)-3
  { 4,0,0,8,1,1,5,7,7,11,6,6}, { 6,6,4,10,5,7,7,3,3,9,2,2}
};

//    vertices      //    edges                 //    faces
//      6-----7     //        [6]---11---[7]    //        1
//     /|    /|     //        /|         /|     //        | 3
//    4-----5 |     //       6 2        7 3     //        |/
//    | 2---|-3     //      /  |       /  |     //    4---+---5
//    |/    |/      //    [4]---10---[5]  |     //       /|
//    0-----1       //     |   |      |   |     //      2 |
//                  //     |  [2]--9--|--[3]    //        0
//     i            //     0  /       1  /      //
//     | j          //     | 4        | 5       //
//     |/           //     |/         |/        //
//     +---k        //    [0]---8----[1]        //

const int IsoSurf::_edgeTable[12][2] = {
  /* _edgeTable[ 0] = */ {0,4}, // +4 | [i,i+1],j  ,k
  /* _edgeTable[ 1] = */ {1,5}, // +4 | [i,i+1],j  ,k+1
  /* _edgeTable[ 2] = */ {2,6}, // +4 | [i,i+1],j+1,k
  /* _edgeTable[ 3] = */ {3,7}, // +4 | [i,i+1],j+1,k+1
  /* _edgeTable[ 4] = */ {0,2}, // +2 | i  ,[j,j+1]  ,k
  /* _edgeTable[ 5] = */ {1,3}, // +2 | i  ,[j,j+1]  ,k+1
  /* _edgeTable[ 6] = */ {4,6}, // +2 | i+1,[j,j+1]  ,k
  /* _edgeTable[ 7] = */ {5,7}, // +2 | i+1,[j,j+1]  ,k+1
  /* _edgeTable[ 8] = */ {0,1}, // +1 | i  ,j  ,[k,k+1]
  /* _edgeTable[ 9] = */ {2,3}, // +1 | i  ,j+1,[k,k+1]
  /* _edgeTable[10] = */ {4,5}, // +1 | i+1,j  ,[k,k+1]
  /* _edgeTable[11] = */ {6,7}  // +1 | i+1,j+1,[k,k+1]
};


const int (*IsoSurf::getEdgeTable())[2] {
  return _edgeTable;
}

int IsoSurf::makeCellFaces
(bool b[/*8*/], int iE[/*12*/], vector<int>& coordIndex) {

  // if(b==null || b.length<8) return;
  // if(iE==null || iE.length<12) return;
  // if(coordIndex==null) return;

  // link vertices
  int i,j,k,j0,j1;
  int next[12];
  for(i=0;i<12;i++)
    next[i] = -1;
  for(i=j=0;j<3;j++) { // j=0,1,2
    for(j0=0;j0<2;j0++) {
      for(j1=0;j1<2;j1++,i++) {
        if(iE[i]>=0) {
          k =
            (b[_faceTable[i][0]])?
            ((!b[_faceTable[i][2]])?1:(!b[_faceTable[i][4]])?3:5):
            ((!b[_faceTable[i][8]])?7:(!b[_faceTable[i][10]])?9:11);
          next[i] = _faceTable[i][k];
        }
      }
    }
  }

  // traverse linked list and output faces
  int j_prev,j_next,nCorners;
  int nFaces = 0;
  for(i=0;i<12;i++) {
    if(next[i]>=0) {
      nCorners = 0;
      j_prev = -1;
      j = i;
      do {
        j_next = next[j];
        // skip repeated vertex indices
        if(j_prev<0 || iE[j]!=iE[j_prev] || (j_next==i && iE[j]!=iE[i])) {
          coordIndex.push_back(iE[j]); nCorners++;
        }
        j_prev = j; j=j_next; next[j_prev] = -1;
      } while(j!=i);
      if(nCorners>=3) {
        // faces with 3 or more corners are OK
        coordIndex.push_back(-1);
        nFaces++;
      } else {
        // remove faces with less than 3 corners
        for(;nCorners>0;nCorners--)
          coordIndex.pop_back();
      }
    }
  }
  return nFaces;
}
