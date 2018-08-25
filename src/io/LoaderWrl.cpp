//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2016-05-03 18:28:23 taubin>
//------------------------------------------------------------------------
//
// LoaderWrl.cpp
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

#include <stdio.h>
#include "Tokenizer.h"
#include "LoaderWrl.h"
#include "StrException.h"

#define VRML_HEADER "#VRML V2.0 utf8"

const char* LoaderWrl::_ext = "wrl";

bool LoaderWrl::loadSceneGraph(Tokenizer& tkn, SceneGraph& wrl) {

  string name    = "";
  bool   success = false;
  while(tkn.get()) {
    if(tkn.equals("DEF")) {
      // if(name!="") throw StrException("DEF name DEF");
      tkn.get("missing token after DEF");
      name = tkn;
    } else if(tkn.equals("Group")) {
      Group* g = new Group();
      wrl.addChild(g);
      loadGroup(tkn,*g);
      g->setName(name);
      name = "";
    } else if(tkn.equals("Transform")) {
      Transform* t = new Transform();
      wrl.addChild(t);
      loadTransform(tkn,*t);
      t->setName(name);
      name = "";
    } else if(tkn.equals("Shape")) {
      Shape* s = new Shape();
      wrl.addChild(s);
      loadShape(tkn,*s);
      s->setName(name);
      name = "";
    } else if(tkn.equals("")) {
      break;
    } else {
      fprintf(stderr,"tkn=\"%s\"\n",tkn.c_str());
      throw new StrException("unexpected token while parsing Group");
    }
  }
  success = true;
  return success;
}

bool LoaderWrl::loadGroup(Tokenizer& tkn, Group& group) {

  // Group {
  //   MFNode children    []
  //   SFVec3f bboxCenter  0 0 0
  //   SFVec3f bboxSize   -1 -1 -1
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("children")) {
      loadChildren(tkn,group);
    } else if(tkn.equals("bboxCenter")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      group.setBBoxCenter(v);
    } else if(tkn.equals("bboxSize")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      group.setBBoxSize(v);
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("unexpected token while parsing Group");
    }
  }
  return success;
}

bool LoaderWrl::loadTransform(Tokenizer& tkn, Transform& transform) {

  // Transform {
  //   MFNode     children          []
  //   SFVec3f    bboxCenter        0 0 0
  //   SFVec3f    bboxSize          -1 -1 -1
  //   SFVec3f    center            0 0 0
  //   SFRotation rotation          0 0 1 0
  //   SFVec3f    scale             1 1 1
  //   SFRotation scaleOrientation  0 0 1 0
  //   SFVec3f    translation       0 0 0
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("children")) {
      loadChildren(tkn,transform);
    } else if(tkn.equals("bboxCenter")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      transform.setBBoxCenter(v);
    } else if(tkn.equals("bboxSize")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      transform.setBBoxCenter(v);
    } else if(tkn.equals("center")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      transform.setCenter(v);
    } else if(tkn.equals("rotation")) {
      Vec4f	v;
      if(tkn.getVec4f(v)==false)
        throw new StrException("expecting Vec4f");
      transform.setRotation(v);
    } else if(tkn.equals("scale")) {
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      transform.setScale(v);
    } else if(tkn.equals("scaleOrientation")) {
      // expecting 4 floats
      Vec4f v;
      if(tkn.getVec4f(v)==false)
        throw new StrException("expecting Vec4f");
      transform.setScaleOrientation(v);
    } else if(tkn.equals("translation")) {
      // expecting 3 floats
      Vec3f v;
      if(tkn.getVec3f(v)==false)
        throw new StrException("expecting Vec3f");
      transform.setTranslation(v);
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("unexpected token while parsing Group");
    }
  }
  return success;
}

bool LoaderWrl::loadChildren(Tokenizer& tkn, Group& group) {
  string name    = "";
  bool   success = false;
  if(tkn.expecting("[")==false) throw new StrException("expecting \"[\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("DEF")) {
      tkn.get("missing token after DEF");
      name = tkn;
    } else if(tkn.equals("Group")) {
      Group* g = new Group();
      group.addChild(g);
      loadGroup(tkn,*g);
      g->setName(name);
      name = "";
    } else if(tkn.equals("Transform")) {
      Transform* t = new Transform();
      group.addChild(t);
      loadTransform(tkn,*t); 
      t->setName(name);
      name = "";
   } else if(tkn.equals("Shape")) {
      Shape* s = new Shape();
      group.addChild(s);
      loadShape(tkn,*s);
      s->setName(name);
      name = "";
    } else if(tkn.equals("]")) {
      success = true;
    } else {
      throw new StrException("unexpected token while parsing Group");
    }
  }
  return success;
}

bool LoaderWrl::loadShape(Tokenizer& tkn, Shape& shape) {

  // Shape {
  //   SFNode appearance NULL
  //   SFNode geometry   NULL
  // }

  // TODO Mon Nov 12 10:22:28 2012
  // implement DEF name for appearance and geometry nodes

  string name    = "";
  bool   success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("appearance")) {
      tkn.get("expecting appearance node");
      if(tkn.equals("DEF")) {
        tkn.get("missing token after DEF");
        name = tkn;
        tkn.get("missing Appearance token");
      }
      if(tkn.equals("Appearance")==false)
        throw new StrException("expecting Appearance");
      Appearance* a = new Appearance();
      a->setName(name);
      name = "";
      shape.setAppearance(a);
      loadAppearance(tkn,*a);
    } else if(tkn.equals("geometry")) {
      tkn.get("expecting geometry node");
      if(tkn.equals("DEF")) {
        tkn.get("missing token after DEF");
        name = tkn;
        tkn.get("missing Appearance token");
      }
      if(tkn.equals("IndexedFaceSet")) {
        IndexedFaceSet* ifs = new IndexedFaceSet();
        ifs->setName(name);
        name = "";
        shape.setGeometry(ifs);
        loadIndexedFaceSet(tkn,*ifs);
      } else if(tkn.equals("IndexedLineSet")) {
        IndexedLineSet* ils = new IndexedLineSet();
        ils->setName(name);
        name = "";
        shape.setGeometry(ils);
        loadIndexedLineSet(tkn,*ils);
      } else {
        throw new StrException("found unexpected geometry node");
      }
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found Appearance field");
    }
  }
  return success;
}

bool LoaderWrl::loadAppearance(Tokenizer& tkn, Appearance& appearance) {

  // Appearance {
  //   SFNode material NULL
  //   SFNode texture NULL
  //   // SFNode textureTransform NULL
  // }

  // TODO Mon Nov 12 10:22:28 2012
  // implement DEF name for matrial and texture nodes

  string name    = "";
  bool   success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"[\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("material")) {
      tkn.get("expecting material node");
      if(tkn.equals("DEF")) {
        tkn.get("missing token after DEF");
        name = tkn;
        tkn.get("missing Appearance token");
      }
      if(tkn.equals("Material")==false)
        throw new StrException("expecting Material");
      Material* m = new Material();
      m->setName(name);
      name = "";
      appearance.setMaterial(m);
      loadMaterial(tkn,*m);
    } else if(tkn.equals("texture")) {
      tkn.get("expecting Texture node");
      if(tkn.equals("DEF")) {
        tkn.get("missing token after DEF");
        name = tkn;
        tkn.get("missing Appearance token");
      }
      if(tkn.equals("ImageTexture")) {
        ImageTexture* it = new ImageTexture();
        it->setName(name);
        name = "";
        appearance.setTexture(it);
        loadImageTexture(tkn,*it);
   // } else if(tkn.equals("PixelTexture")) {
   //   ...
   // } else if(tkn.equals("MovieTexture")) {
   //   ...
      } else {
        throw new StrException("found unexpected Texture node");
      }
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found unexpected Group field");
    }
  }
  return success;
}

bool LoaderWrl::loadMaterial(Tokenizer& tkn, Material& material) {

  // Material {
  //   SFFloat ambientIntensity 0.2
  //   SFColor diffuseColor     0.8 0.8 0.8
  //   SFColor emissiveColor    0 0 0
  //   SFFloat shininess        0.2
  //   SFColor specularColor    0 0 0
  //   SFFloat transparency     0
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("ambientIntensity")) {
      float f;
      if(tkn.getFloat(f)==false)
        throw new StrException("expecting float");
      material.setAmbientIntensity(f);
    } else if(tkn.equals("diffuseColor")) {
      Color c;
      if(tkn.getColor(c)==false)
        throw new StrException("expecting Color");
      material.setDiffuseColor(c);
    } else if(tkn.equals("emissiveColor")) {
      Color c;
      if(tkn.getColor(c)==false)
        throw new StrException("expecting Color");
      material.setEmissiveColor(c);
    } else if(tkn.equals("shininess")) {
      float f;
      if(tkn.getFloat(f)==false)
        throw new StrException("expecting float");
      material.setShininess(f);
    } else if(tkn.equals("specularColor")) {
      Color c;
      if(tkn.getColor(c)==false)
        throw new StrException("expecting Color");
      material.setSpecularColor(c);
    } else if(tkn.equals("transparency")) {
      float f;
      if(tkn.getFloat(f)==false)
        throw new StrException("expecting float");
      material.setTransparency(f);
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found unexpected Appearance field");
    }
  }
  return success;

}

bool LoaderWrl::loadImageTexture(Tokenizer& tkn, ImageTexture& imageTexture) {

  // ImageTexture {
  //   MFString url []
  //   SFBool repeatS TRUE
  //   SFBool repeatT TRUE
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("url")) {
      vector<string>& _url = imageTexture.getUrl();
      if(loadVecString(tkn,_url)==false)
        throw new StrException("loading vector<string>");
    } else if(tkn.equals("repeatS")) {
      bool b;
      if(tkn.getBool(b)==false)
        throw new StrException("expecting boolean value");
      imageTexture.setRepeatS(b);
    } else if(tkn.equals("repeatT")) {
      bool b;
      if(tkn.getBool(b)==false)
        throw new StrException("expecting boolean value");
      imageTexture.setRepeatT(b);
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found unexpected Appearance field");
    }
  }
  return success;
}

bool LoaderWrl::loadIndexedFaceSet(Tokenizer& tkn, IndexedFaceSet& ifs) {

  // IndexedFaceSet {
  //   SFNode  color             NULL
  //   SFNode  coord             NULL
  //   SFNode  normal            NULL
  //   SFNode  texCoord          NULL
  //   SFBool  ccw               TRUE
  //   MFInt32 colorIndex        []        # [-1,)
  //   SFBool  colorPerVertex    TRUE
  //   SFBool  convex            TRUE
  //   MFInt32 coordIndex        []        # [-1,)
  //   SFFloat creaseAngle       0         # [ 0,)
  //   MFInt32 normalIndex       []        # [-1,)
  //   SFBool  normalPerVertex   TRUE
  //   SFBool  solid             TRUE
  //   MFInt32 texCoordIndex     []        # [-1,)
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("color")) {
      //   SFNode  
      vector<float>& _color = ifs.getColor();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("Color")==false)
        throw new StrException("expecting Color");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("color")==false)
        throw new StrException("expecting color");
      if(loadVecFloat(tkn,_color)==false)
        throw new StrException("loading IndexedFaceSet color field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("coord")) {
      //   SFNode  
      vector<float>& _coord = ifs.getCoord();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("Coordinate")==false)
        throw new StrException("expecting Coordinate");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("point")==false)
        throw new StrException("expecting point");
      if(loadVecFloat(tkn,_coord)==false)
        throw new StrException("loading IndexedFaceSet coord field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("normal")) {
      //   SFNode  
      vector<float>& _normal = ifs.getNormal();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("Normal")==false)
        throw new StrException("expecting Normal");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("vector")==false)
        throw new StrException("expecting vector");
      if(loadVecFloat(tkn,_normal)==false)
        throw new StrException("loading IndexedFaceSet normal field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("texCoord")) {
      //   SFNode  
      vector<float>& _texCoord = ifs.getTexCoord();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("TextureCoordinate")==false)
        throw new StrException("expecting TextureCoordinate");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("point")==false)
        throw new StrException("expecting point");
      if(loadVecFloat(tkn,_texCoord)==false)
        throw new StrException("loading IndexedFaceSet texCoord field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("ccw")) {
      //   SFBool
      bool& _ccw = ifs.getCcw();
      if(tkn.getBool(_ccw)==false)
        throw new StrException("loading IndexedFaceSet ccw field");  
    } else if(tkn.equals("colorIndex")) {
      //   MFInt32 
      vector<int>& _colorIndex = ifs.getColorIndex();
      if(loadVecInt(tkn,_colorIndex)==false)
        throw new StrException("loading IndexedFaceSet colorIndex field");
    } else if(tkn.equals("colorPerVertex")) {
      //   SFBool
      bool& _colorPerVertex = ifs.getColorPerVertex();
        if(tkn.getBool(_colorPerVertex)==false)
        throw new StrException("loading IndexedFaceSet colorPerVertex field");
    } else if(tkn.equals("convex")) {
      //   SFBool
      bool& _convex = ifs.getConvex();  
      if(tkn.getBool(_convex)==false)
        throw new StrException("loading IndexedFaceSet convex field");
    } else if(tkn.equals("coordIndex")) {
      //   MFInt32 
      vector<int>& _coordIndex = ifs.getCoordIndex();
      if(loadVecInt(tkn,_coordIndex)==false)
        throw new StrException("loading IndexedFaceSet coordIndex field");
    } else if(tkn.equals("creaseAngle")) {
      //   SFFloat
      float& _creaseAngle = ifs.getCreaseangle();
      if(tkn.getFloat(_creaseAngle)==false)
        throw new StrException("loading IndexedFaceSet creaseAngle value");
    } else if(tkn.equals("normalIndex")) {
      //   MFInt32 
      vector<int>& _normalIndex = ifs.getNormalIndex();
      if(loadVecInt(tkn,_normalIndex)==false)
        throw new StrException("loading IndexedFaceSet normalIndex field");
    } else if(tkn.equals("normalPerVertex")) {
      //   SFBool
      bool& _normalPerVertex = ifs.getNormalPerVertex();
      if(tkn.getBool(_normalPerVertex)==false)
        throw new StrException("loading IndexedFaceSet normalPerVertex field");
    } else if(tkn.equals("solid")) {
      //   SFBool
      bool& _solid = ifs.getSolid();  
      if(tkn.getBool(_solid)==false)
        throw new StrException("loading IndexedFaceSet solid field");
    } else if(tkn.equals("texCoordIndex")) {
      //   MFInt32 
      vector<int>& _texCoordIndex = ifs.getTexCoordIndex();
      if(loadVecInt(tkn,_texCoordIndex)==false)
        throw new StrException("loading IndexedFaceSet texCoordIndex field");
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found unexpected IndexedFaceSet field");
    }
  }
  return success;
}

bool LoaderWrl::loadIndexedLineSet(Tokenizer& tkn, IndexedLineSet& ifs) {

  // IndexedFaceSet {
  //   SFNode  coord             NULL
  //   MFInt32 coordIndex        []        # [-1,)
  //   SFNode  color             NULL
  //   MFInt32 colorIndex        []        # [-1,)
  //   SFBool  colorPerVertex    TRUE
  // }

  bool success = false;
  if(tkn.expecting("{")==false) throw new StrException("expecting \"{\"");
  while(success==false && tkn.get()) {
    if(tkn.equals("color")) {
      //   SFNode  
      vector<float>& _color = ifs.getColor();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("Color")==false)
        throw new StrException("expecting Color");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("color")==false)
        throw new StrException("expecting color");
      if(loadVecFloat(tkn,_color)==false)
        throw new StrException("loading IndexedLineSet color field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("coord")) {
      //   SFNode  
      vector<float>& _coord = ifs.getCoord();

      // if DEF name found, skip and ignore ??

      if(tkn.expecting("Coordinate")==false)
        throw new StrException("expecting Coordinate");
      if(tkn.expecting("{")==false)
        throw new StrException("expecting \"{\"");
      if(tkn.expecting("point")==false)
        throw new StrException("expecting point");
      if(loadVecFloat(tkn,_coord)==false)
        throw new StrException("loading IndexedLineSet coord field");
      if(tkn.expecting("}")==false)
        throw new StrException("expecting \"}\"");

    } else if(tkn.equals("colorIndex")) {
      //   MFInt32 
      vector<int>& _colorIndex = ifs.getColorIndex();
      if(loadVecInt(tkn,_colorIndex)==false)
        throw new StrException("loading IndexedLineSet colorIndex field");
    } else if(tkn.equals("colorPerVertex")) {
      //   SFBool
      bool& _colorPerVertex = ifs.getColorPerVertex();
        if(tkn.getBool(_colorPerVertex)==false)
        throw new StrException("loading IndexedLineSet colorPerVertex field");
    } else if(tkn.equals("coordIndex")) {
      //   MFInt32 
      vector<int>& _coordIndex = ifs.getCoordIndex();
      if(loadVecInt(tkn,_coordIndex)==false)
        throw new StrException("loading IndexedLineSet coordIndex field");
    } else if(tkn.equals("}")) {
      success = true;
    } else {
      throw new StrException("found unexpected IndexedLineSet field");
    }
  }
  return success;
}

bool LoaderWrl::loadVecFloat(Tokenizer&tkn,vector<float>& vec) {
  bool success = false;
  if(tkn.expecting("[")==false) throw new StrException("expecting \"[\"");
  float value;
  while(success==false && tkn.get()) {
    if(tkn.equals("]")) {
      success = true; // done
    } else if(sscanf(tkn.c_str(),"%f",&value)==1) {
      vec.push_back(value);
    } else {
      throw new StrException("expecting int value");
    }
  }

  return success;
}

bool LoaderWrl::loadVecInt(Tokenizer&tkn,vector<int>& vec) {
  bool success = false;
  if(tkn.expecting("[")==false) throw new StrException("expecting \"[\"");
  int value;
  while(success==false && tkn.get()) {
    if(tkn.equals("]")) {
      success = true; // done
    } else if(sscanf(tkn.c_str(),"%d",&value)==1) {
      vec.push_back(value);
    } else {
      throw new StrException("expecting int value");
    }
  }
  return success;
}

bool LoaderWrl::loadVecString(Tokenizer&tkn,vector<string>& vec) {
  bool success = false;
  tkn.get("expecting a token");
  if(tkn.equals("[")) {
    // expecting 0 or more strings followed by "]"
    while(tkn.get()) {
      if(tkn.equals("]"))
        break;
      else
        vec.push_back(tkn);
    }
    success = true;
  } else {
    // expecting a single string
    tkn.get("expecting a token");
    vec.push_back(tkn);
    success = true;
  }
  return success;
}

bool LoaderWrl::load(const char* filename, SceneGraph& wrl) {
  bool success = false;

  FILE* fp = (FILE*)0;
  try {

    // open the file
    if(filename==(char*)0) throw new StrException("filename==null");
    fp = fopen(filename,"r");
    if(fp==(FILE*)0) throw new StrException("fp==(FILE*)0");

    // clear the container
    wrl.clear();
    wrl.setUrl(filename);

    // read and check header line
    char header[16];
    // memset(header,'\0',16);
    for(int i=0;i<16;i++) header[i] = '\0';
    fscanf(fp,"%15c",header);
    if(string(header)!=VRML_HEADER) throw new StrException("header!=VRM_HEADER");

    // create a Tokenizer and start parsing
    Tokenizer tkn(fp);
    loadSceneGraph(tkn,wrl);

    // will be done later
    // wrl.updateBBox();
    
    // if we have reached this point we have succeeded
    fclose(fp);
    success = true;

  } catch(StrException* e) { 

    if(fp!=(FILE*)0) fclose(fp);
    fprintf(stderr,"ERROR | %s\n",e->what());
    delete e;
    wrl.clear();
	wrl.setUrl("");

  }

  return success;
}

