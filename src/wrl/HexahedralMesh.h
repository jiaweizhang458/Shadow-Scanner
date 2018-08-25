//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-14 17:49:07 taubin>
//------------------------------------------------------------------------
//
// HexahedralMesh.h
//

// TODO Sat Nov 14 17:48:26 2015

#ifndef _HexahedralMesh_h_
#define _HexahedralMesh_h_

#include <string>
#include <algorithm>
#include <vector>
#include <unordered_map>

using namespace std;

class HexahedralMesh {

public:
  
  HexahedralMesh(unsigned nX, unsigned nY, unsigned nZ);
  virtual ~HexahedralMesh();

  unsigned getNumberOfVertices();
  unsigned getNumberOfCells();

  int      addCell(const int* cellId /*[3]*/);
  bool     getCell(const int* cellId /*[3]*/, int* cellVertex=(int*)0 /*[8]*/);
  bool     getVertex(const int vertexIndex, float* vertex=(float*)0 /*[3]*/);
  bool     getVertex(const int* vertexId /*[3]*/, float* vertex=(float*)0 /*[3]*/);

private:

  unsigned                         _nX;
  unsigned                         _nY;
  unsigned                         _nZ;
  vector<float>                    _coord;
  vector<int>                      _coordIndex;
  unordered_map<unsigned,unsigned> _mapVertices;
  unordered_map<unsigned,unsigned> _mapCells;

};

#endif /* _HexahedralMesh_h_ */
