#version 120

uniform vec3 lightPos;
uniform vec3 lightCol;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

// Necessary variables
varying vec3 vNor;  // Contains the normalized normal of the position vector 
varying vec3 vPos; // Contains the position of fragment in camera space

// Texture variables:
uniform sampler2D textureGround;

// From vertex shader:
varying vec2 vTex0;


void main()
{
	vec3 n = normalize(vNor);

	// Ambient Component / ambient color:
	vec3 c_A = ka;

	if (vTex0.x + vTex0.y > 0.0001){
		c_A = texture2D(textureGround, vTex0).rgb;
	}

	// Diffuse Component:
	vec3 eye = normalize(-1 * vPos);

	vec3 lightVector = normalize( lightPos - vPos);
	vec3 c_D = kd * max(0, dot(lightVector, n) );

	// Specular Component
	vec3 h = normalize(lightVector + eye) ;
	vec3 c_S = ks * pow(max(0, dot(h, n)), s);


	// Actual output color:
	gl_FragColor = vec4(c_A + c_D + c_S, 1);
}

