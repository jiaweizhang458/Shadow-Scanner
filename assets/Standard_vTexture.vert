// vim: filetype=glsl nospell

uniform highp mat4 uProjectionModelView;

attribute highp vec4 aVertex;
attribute highp vec2 aTexCoord;

varying highp vec2 vTexCoord;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = uProjectionModelView * aVertex;
}