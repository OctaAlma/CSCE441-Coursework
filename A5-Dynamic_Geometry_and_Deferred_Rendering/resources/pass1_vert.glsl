#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 itMV;

uniform float t;
uniform int isSor; // Is 1 when we are drawing a solid of revolution

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space
attribute vec2 aTex;

varying vec3 vPos;
varying vec3 vNor;
varying vec2 vTex;

void main()
{
	if (isSor == 1){
		float x = aPos.x;
		float theta = aPos.y;

		// Dependent variables, f(x), y, z:
		float f_x = cos(x + t * 3) + 2;
		float y = f_x * cos(theta);
		float z = f_x * sin(theta);

		// Make a new position variable:
		vec4 pos = vec4(x, y, z, 1);

		// To compute the normals, we take the derivatives with respect to x and theta, then find the cross product
		float f_prime_x = -sin(x + t * 3);
		vec3 dp_dx = vec3(1, f_prime_x * cos(theta), f_prime_x * sin(theta));
		vec3 dp_dtheta = vec3(0, f_x * -sin(theta), f_x * cos(theta));

		vec3 nor = -1 * normalize(cross(dp_dx, dp_dtheta));

		gl_Position = P * (MV * pos);

		vPos = vec3(MV * vec4(x, y, z, 1));
		vNor = normalize(vec3(itMV * vec4(nor, 0.0))); // Assuming MV contains only translations and rotations

	}else{
	
		gl_Position = P * (MV * aPos);

		vPos = vec3(MV * aPos); 
		vNor = normalize(vec3(itMV * vec4(aNor, 0.0)));
	}
    vTex = aTex;

}
