//------------------------------------------------------------------------
// Time-stamp: <2014-06-07 22:22:49 taubin>
// Copyright (c) 2013, Gabriel Taubin, Brown University
// All rights reserved.
//
// PartitionLists.java
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
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL GABRIEL
// TAUBIN OR BROWN UNIVERSITY BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.

package core;

public class PartitionLists extends Partition {

  // private int[]  _beg = null;
  // private int[]  _end = null;
  // private int[] _next = null;
  private VecInt _beg = null;
  private VecInt _end = null;
  private VecInt _next = null;

  public PartitionLists() {
  }

  public PartitionLists(int nElements) {
    reset(nElements);
  }

  public void addElement() {
    int i = _father.size();
    super.addElement();
    _beg.pushBack(i);
    _end.pushBack(i);
    _next.pushBack(-1);
  }

  public void reset(int nElements) {
    super.reset(nElements);
    if(_nElements>0) {
      _beg  = new VecInt(_nElements,-1);
      _end  = new VecInt(_nElements,-1);
      _next = new VecInt(_nElements,-1);
      for(int i=0;i<_nElements;i++) { _beg.set(i,i); _end.set(i,i); }
    } else {
      _beg = _end = _next = null;
    }
  }

  public int join(int i, int j) {
    int Ci = super.find(i);
    int Cj = super.find(j);
    int Ck = Ci;
    if(Ci!=Cj) {
      if(_size.get(Ci)>=_size.get(Cj)) {
        _next.set(_end.get(Ci),_beg.get(Cj));
        _end.set(Ci,_end.get(Cj));
        _beg.set(Cj,-1);
        _end.set(Cj,-1);
      } else {
        _next.set(_end.get(Cj),_beg.get(Ci));
        _end.set(Cj,_end.get(Ci));
        _beg.set(Ci,-1);
        _end.set(Ci,-1);
      }
      Ck = super.join(Ci,Cj);
    }
    return Ck;
  }

  // split a component into singletons
  public void split(int i) {
    int j,k;
    i = find(i); // find the root of the component
    _nParts += (_size.get(i)-1); // fix the number of parts
    // split the tree into singletons
    for(j=_beg.get(i);j>=0;k=_next.get(j),_next.set(j,-1),j=k) {
      _size.set(j,1); _father.set(j,j);
      _beg.set(j,j); _end.set(j,j);
    }
  }

  public int beg(int i)  { return  _beg.get(i); }
  public int end(int i)  { return  _end.get(i); }
  public int next(int i) { return _next.get(i); }

  public int length(int i) {
    int len = 0;
    for(int j=_beg.get(i);j>=0;j=_next.get(j)) len++;
    return len;
  }

}
