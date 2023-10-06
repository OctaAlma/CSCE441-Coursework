#version 120

uniform vec3 ke;
uniform vec3 kd;

varying vec3 vPos; // in camera space
varying vec3 vNor; // in camera space
varying vec2 vTex;
    
void main()
{
    gl_FragData[0].xyz = vPos;
    gl_FragData[1].xyz = vNor;
    gl_FragData[2].xyz = ke;
    gl_FragData[3].xyz = kd;
}