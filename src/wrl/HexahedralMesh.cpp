//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-14 17:48:31 taubin>
//------------------------------------------------------------------------
//
// HexahedralMesh.cpp
//

// TODO Sat Nov 14 17:48:26 2015

#include "HexahedralMesh.h"
  
HexahedralMesh::HexahedralMesh(unsigned nX, unsigned nY, unsigned nZ):
  _nX(nX),_nY(nY),_nZ(nZ) {

}

HexahedralMesh::~HexahedralMesh() {

}

// vector<float> _coord;
// vector<int>   _coordIndex;
// unordered_map<unsigned,unsigned> _vertexIndex;
// unordered_map<unsigned,unsigned> _cellIndex;

unsigned HexahedralMesh::getNumberOfVertices() {
  return _mapVertices.size();
}

unsigned HexahedralMesh::getNumberOfCells() {
  return _mapCells.size();
}

// return -1 if the cell is already in the map,
// and the cell index otherwise
int HexahedralMesh::addCell(const int cellId[/*3*/]) {
  int cellIndx = -1;

  if(0<=cellId[0] && cellId[0]<(int)_nX &&
     0<=cellId[1] && cellId[1]<(int)_nY &&
     0<=cellId[2] && cellId[2]<(int)_nZ) {

    unsigned cellKey   = cellId[0]*_nX*(cellId[1]+_nY*cellId[2]);

    unordered_map<unsigned,unsigned>::const_iterator foundCell =
      _mapCells.find(cellKey);

    if(foundCell!=_mapCells.end()) {

      unsigned cellValue = _coordIndex.size()/9;
      pair<unsigned,unsigned> cPair(cellKey,cellValue);
      _mapCells.insert(cPair);

      unsigned i,i0,i1,i2;
      unsigned vKey[8], iV[8];
      for(i=0;i<8;i++) {
        i2 = ((i>>0)&0x1); i1 = ((i>>1)&0x1); i0 = ((i>>2)&0x1);
        vKey[i] = (cellId[0]+i0)*(_nX+1)*((cellId[1]+i1)+(_nY+1)*(cellId[2]+i2));
      }
      unordered_map<unsigned,unsigned>::const_iterator foundVertex;
      for(i=0;i<8;i++) {
        foundVertex = _mapVertices.find(vKey[i]);
        if(foundVertex==_mapVertices.end()) {
          iV[i] = _coord.size();

          // TODO

          _coord.push_back(0.0f); _coord.push_back(0.0f); _coord.push_back(0.0f);
          pair<unsigned,unsigned> vPair(vKey[i],iV[i]);
          _mapVertices.insert(vPair);
        } else {
          iV[i] = _mapVertices[vKey[i]];
        }
        _coordIndex.push_back((int)(iV[i]));
      }
      _coordIndex.push_back(-1);

    }

  }
  return cellIndx;
}

bool HexahedralMesh::getCell(const int cellId[/*3*/], int cellVertex[/*8*/]) {
  bool found = false;
  if(0<=cellId[0] && cellId[0]<(int)_nX &&
     0<=cellId[1] && cellId[1]<(int)_nY &&
     0<=cellId[2] && cellId[2]<(int)_nZ) {
    unsigned cellKey = cellId[0]*_nX*(cellId[1]+_nY*cellId[2]);
    unordered_map<unsigned,unsigned>::const_iterator foundCell =
      _mapCells.find(cellKey);
    if(foundCell!=_mapCells.end()) {
      int cellValue = (int)(foundCell->second);
      if(cellVertex!=(int*)0) {
        for(unsigned i=0;i<8;i++)
          cellVertex[i] = _coordIndex[9*cellValue+i];
      }
    }
    found = true;
  }
  return found;
}

bool HexahedralMesh::getVertex
(const int vertexIndex, float* vertex) {
  bool found = false;
  if(0<=vertexIndex && vertexIndex<(int)getNumberOfVertices()) {
    if(vertex!=(float*)0)
      for(unsigned i=0;i<3;i++)
        vertex[i] = _coord[3*vertexIndex+i];
    found = true;
  }
  return found;
}

bool HexahedralMesh::getVertex
(const int* vertexId /*[3]*/, float* vertex /*[3]*/) {
  bool found = false;
  if(0<=vertexId[0] && vertexId[0]<(int)_nX &&
     0<=vertexId[1] && vertexId[1]<(int)_nY &&
     0<=vertexId[2] && vertexId[2]<(int)_nZ) {
    unsigned vertexKey = vertexId[0]*(_nX+1)*(vertexId[1]+(_nY+1)*vertexId[2]);
    unordered_map<unsigned,unsigned>::const_iterator foundVertex =
      _mapVertices.find(vertexKey);
    if(foundVertex!=_mapVertices.end()) {
      unsigned vertexIndex = foundVertex->second;
      found = getVertex(vertexIndex,vertex);
    }
  }
  return found;
}

