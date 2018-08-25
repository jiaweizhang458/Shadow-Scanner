// vim: filetype=glsl nospell

uniform sampler2D uTexture;

varying highp vec2 vTexCoord;

void main() 
{
  gl_FragColor = texture2D(uTexture, vTexCoord).rgba;
}
