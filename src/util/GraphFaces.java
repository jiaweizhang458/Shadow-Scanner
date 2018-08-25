//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2014-06-07 19:47:20 taubin>
//------------------------------------------------------------------------
//
// GraphFaces.java
//
// Copyright (c) 2009-2012, Gabriel Taubin
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

package core;

public class GraphFaces extends Graph
{
  private int               _nF;
  private VecInt	    _edgeF;
  private VecInt	    _edgeI;

  public GraphFaces(VecInt coordIndex, int nV) {

    super(coordIndex,nV);

    if(coordIndex.size()>0) {

      int i,nfi,fi,j,k;
      int nE = getNumberOfEdges();
      _edgeF = new VecInt(nE+1,0);

      // count number of incident faces
      nfi = 0;
      fi  = -1;
      for(k=0;k<coordIndex.size();k++) {
        if(coordIndex.get(k)>=0) {
          if(k==0 || coordIndex.get(k-1)<0)
            { fi = k; nfi = 0; }
          nfi++;
        } else {
          _nF++;
          for(j=0;j<nfi;j++) {
            int h = (j+1)%nfi;
            int fij = coordIndex.get(fi+j);
            int fih = coordIndex.get(fi+h);
            GraphEdge e = getEdge(fij,fih);
            if(e!=null) {
              int ie = e.getIndex();
              int n = _edgeF.get(ie+1)+1;
              _edgeF.set(ie+1,n);
            }
          }
        }
      }
      
      for(i=0;i<nE;i++) {
        int n = _edgeF.get(i+1)+_edgeF.get(i);
        _edgeF.set(i+1,n);
      }
      int nEF = _edgeF.get(nE);
      _edgeI = new VecInt(nEF,-1);
      int[] nFe = new int[nE];
      for(i=0;i<nE;i++) nFe[i] = 0;
      nfi = 0;
      fi  = -1;
      for(i=k=0;k<coordIndex.size();k++)
        if(coordIndex.get(k)>=0) {
          if(k==0 || coordIndex.get(k-1)<0)
            { fi = k; nfi = 0; }
          nfi++;
        } else {  // end of face
          for(j=0;j<nfi;j++) {
            int h = (j+1)%nfi;
            int fij = coordIndex.get(fi+j);
            int fih = coordIndex.get(fi+h);
            GraphEdge e = getEdge(fij,fih);
            if(e!=null) {
              int  ie = e.getIndex();
              _edgeI.set(_edgeF.get(ie)+nFe[ie],i);
              nFe[ie]++;
            }
          }
          i++;
        }
    }
  }
  
  public int getNumberOfFaces()
  { return _nF; }

  public void insertFace(int nf, int[] f) {
    if(_isConst==false && nf>0) {
      for(int i=0;i<nf;i++) {
        int j = (i+1)%nf;
        insertEdge(f[i],f[j]);
      }
      // update _edgeF and _edgeI
    }
  }
  
  public void insertTriangle(int i, int j, int k) {
    if(_isConst==false) {
      insertEdge(i,j);
      insertEdge(j,k);
      insertEdge(k,i);
    }
    // update _edgeF and _edgeI
  }

  public void insertTriangle(int[] t) {
    if(t!=null && t.length>=3)
      insertTriangle(t[0],t[1],t[2]);
  }

  public void setEdgeFaces(VecInt edgeF, VecInt edgeI)
  { _edgeF = edgeF; _edgeI = edgeI; }

  public int getNumberOfEdgeFaces(int eIndx)
  { return _edgeF.get(eIndx+1)-_edgeF.get(eIndx); }

  public int getNumberOfEdgeFaces(GraphEdge e)
  { return getNumberOfEdgeFaces(e.getIndex()); }

  public int getEdgeFace(int eIndx, int i)
  { return _edgeI.get(_edgeF.get(eIndx)+i); }

  public int getEdgeFace(int eIndx) {
    int fi = -1;
    int nf = getNumberOfEdgeFaces(eIndx);
    if(nf==1) nf = 2;
    for(int i=0;i<nf;i++)
      if(_edgeI.get(_edgeF.get(eIndx)+i)>=0)
        { fi = _edgeI.get(_edgeF.get(eIndx)+i); break; }
    return fi;
  }

  public int getEdgeFace(GraphEdge e, int i)
  { return getEdgeFace(e.getIndex(),i); }

  public int getEdgeFace(GraphEdge e)
  { return getEdgeFace(e.getIndex()); }

  public int getOtherEdgeFace(int eIndx, int fi) {
    int fOther = -1;
    int nFe    = _edgeF.get(eIndx+1)-_edgeF.get(eIndx);
    if(nFe>1) {
      int j;
      for(j=0;j<nFe;j++)
        if(_edgeI.get(_edgeF.get(eIndx)+j)==fi) {
          fOther = (j>0)?_edgeI.get(_edgeF.get(eIndx)):
            _edgeI.get(_edgeF.get(eIndx)+1);
          break;
        }
    }
    return fOther;
  }
  
  public int getOtherEdgeFace(GraphEdge e, int fi)
  { return getOtherEdgeFace(e.getIndex(),fi); }

  public boolean isEdgeFace(int eIndx, int fi) {
    boolean is_edge_face = false;
    for(int j=_edgeF.get(eIndx);j<_edgeF.get(eIndx+1);j++)
      if(_edgeI.get(j)==fi)
        { is_edge_face=true; break; }
    return is_edge_face;
  }

  public boolean isEdgeFace(GraphEdge e, int fi)
  { return isEdgeFace(e.getIndex(),fi); }

  public boolean isUnknownEdge(int eIndx)
  { return (getNumberOfEdgeFaces(eIndx) <0)?true:false; }

  public boolean isUnknownEdge(GraphEdge e)
  { return isUnknownEdge(e.getIndex()); }

  public boolean isIsolatedEdge(int eIndx)
  { return (getNumberOfEdgeFaces(eIndx)==0)?true:false; }

  public boolean isIsolatedEdge(GraphEdge e)
  { return isIsolatedEdge(e.getIndex()); }

  public boolean isBoundaryEdge(int eIndx)
  { return (getNumberOfEdgeFaces(eIndx)==1)?true:false; }

  public boolean isBoundaryEdge(GraphEdge e)
  { return isBoundaryEdge(e.getIndex()); }

  public boolean isRegularEdge(int eIndx)
  { return (getNumberOfEdgeFaces(eIndx)==2)?true:false; }

  public boolean isRegularEdge(GraphEdge e)
  { return isRegularEdge(e.getIndex()); }

  public boolean isManifoldEdge(int eIndx)
  { return ((getNumberOfEdgeFaces(eIndx)==1)||
            (getNumberOfEdgeFaces(eIndx)==2))?true:false; }

  public boolean isManifoldEdge(GraphEdge e)
  { return isManifoldEdge(e.getIndex()); }

  public boolean isSingularEdge(int eIndx)
  { return (getNumberOfEdgeFaces(eIndx)>=3)?true:false; }

  public boolean isSingularEdge(GraphEdge e)
  { return isSingularEdge(e.getIndex()); }

  public Graph  makeDualGraph() {
    int nF = getNumberOfFaces();
    Graph dualGraph = new Graph(nF,false);
    int i;
    GraphEdge e;
    for(i=0;i<nF;i++)
      for(e=getFirstEdge(i);e!=null;e=getNextEdge(e))
        if(isRegularEdge(e)==true) {
          int f0 = getEdgeFace(e,0);
          int f1 = getEdgeFace(e,1);
          dualGraph.insertEdge(f0,f1);
        }
    return dualGraph;
  }
  
}

