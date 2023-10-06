#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 itMV;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 color; // Pass to fragment shader

// For Blinn-Phong shader:
varying vec3 normal;
varying vec3 FragPos;

varying vec3 vPos;
varying vec3 vNor;

// texture input variables:
attribute vec2 aTex;

// Texture output variables:
varying vec2 vTex0;
uniform mat3 T;


void main()
{
	gl_Position = P * (MV * aPos);
	color = 0.5 * aNor + vec3(0.5, 0.5, 0.5);

	// For Blinn-Phong shader:
	normal = normalize( vec3(itMV * vec4(aNor,0)) );
	FragPos = vec3 (MV * aPos);

	// From notes:
	vec4 tmp = MV * aPos;
	vPos = vec3(tmp); 

	tmp = itMV * vec4(aNor, 0.0);
	vNor = normalize(vec3(tmp));

	// Texture stuff:
	vTex0 = vec2(T * vec3(aTex,1));
}
