//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 15:26:08 taubin>
//------------------------------------------------------------------------
//
// SaverWrl.h
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

#ifndef _IFS_WRL_SAVER_H_
#define _IFS_WRL_SAVER_H_

#include "Saver.h"
#include <wrl/Shape.h>
#include <wrl/Appearance.h>
#include <wrl/Material.h>
#include <wrl/IndexedFaceSet.h>
#include <wrl/IndexedLineSet.h>
#include <wrl/ImageTexture.h>
#include <wrl/Transform.h>
#include <wrl/SceneGraphTraversal.h>

class SaverWrl : public Saver {

private:

const static char* _ext;

public:

  SaverWrl()  {};
  ~SaverWrl() {};

  bool  save(const char* filename, SceneGraph& wrl) const;
  const char* ext() const { return _ext; }
  
private:
  
  void saveAppearance
  (FILE* fp, string indent, Appearance* appearance) const;
  void saveGroup
  (FILE* fp, string indent, Group* group) const;
  void saveImageTexture
  (FILE* fp, string indent, ImageTexture* imageTexture) const;
  void saveIndexedFaceSet
  (FILE* fp, string indent, IndexedFaceSet* indexedFaceSet) const;
  void saveIndexedLineSet
  (FILE* fp, string indent, IndexedLineSet* indexedLineSet) const;
  void saveMaterial
  (FILE* fp, string indent, Material* material) const;
  void saveShape
  (FILE* fp, string indent, Shape* shape) const;
  void saveTransform
  (FILE* fp, string indent, Transform* transform) const;
  
};

#endif /* _IFS_WRL_SAVER_H_ */
