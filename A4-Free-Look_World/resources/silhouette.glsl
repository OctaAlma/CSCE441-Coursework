#version 120

uniform vec3 lightPos1;
uniform vec3 lightCol1;

uniform vec3 lightPos2;
uniform vec3 lightCol2;

uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

// Necessary variables
varying vec3 vNor;  // Contains the normalized normal of the position vector 
varying vec3 vPos; // Contains the position of fragment in camera space

void main()
{
    // Silhouette shaders colors all fragments white execept those
    // fragments whose normals form a right angle with the eye vector

    // eye vector: direction from FragPos to (0,0,0) / negative normalized FragPos
    vec3 eye = normalize(-1 * vPos);

    // A fragment has a right angle with the eye vector if the dot product of the two
    // vectors is 0. 
    // for the purpose of this assignment, color any fragments whose dot product between
    // the eye vector and the fragment normal vector is < 0.3

    if (dot (vNor, eye) < 0.3f){
        gl_FragColor = vec4(0, 0, 0, 1);
    }else{
        gl_FragColor = vec4(1, 1, 1, 1);
    }
}
