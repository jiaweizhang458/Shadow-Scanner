//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2015-11-12 12:19:56 taubin>
//------------------------------------------------------------------------
//
// Material.cpp
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
#include "Material.h"
  
Material::Material():
  _ambientIntensity(0.2f),
  _diffuseColor(0.8f,0.8f,0.8f),
  _emissiveColor(0.0f,0.0f,0.0f),
  _shininess(0.2f),
  _specularColor(0.0f,0.0f,0.0f),
  _transparency(0.0f) {
}

Material::~Material() {
}


float  Material::getAmbientIntensity()            {  return _ambientIntensity; }
Color& Material::getDiffuseColor()                {  return     _diffuseColor; }
Color& Material::getEmissiveColor()               {  return    _emissiveColor; }
float  Material::getShininess()                   {  return        _shininess; }
Color  Material::getSpecularColor()               {  return    _specularColor; }
float  Material::getTransparency()                {  return     _transparency; }

void   Material::setAmbientIntensity(float value) { _ambientIntensity = value; }
void   Material::setDiffuseColor(Color& value)    {     _diffuseColor = value; }
void   Material::setEmissiveColor(Color&value)    {    _emissiveColor = value; }
void   Material::setShininess(float value)        {        _shininess = value; }
void   Material::setSpecularColor(Color& value)   {    _specularColor = value; }
void   Material::setTransparency(float value)     {     _transparency = value; }

void Material::printInfo(string indent) {
  std::cout << indent;
  if(_name!="") std::cout << "DEF " << _name << " ";
  std::cout << "Material {\n";
  std::cout << indent << "}\n";
}
