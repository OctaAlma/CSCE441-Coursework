#include "Hit.h"
#include <iostream>
using std::cout;
using std::endl;

Hit getPlaneIntersection(vec3 p_world, vec3 v_world, vec3 c, vec3 n){
	// The intersection point x, can be computed as:
	// Note: t is the distance of the intersection point from the camera
	float t = dot(n, (c - vec3(p_world))) / dot(n, v_world);
	vec3 x = vec3(p_world) + (t * v_world);

	// now we add a hit to the hits vector:
	Hit planeHit;
	planeHit.n = n;
	planeHit.x = x;
	planeHit.t = t;
	return planeHit;
}

// Given the parameters for a sphere and a ray's origin and direction, create a hit object
vector<Hit> getSphereIntersections(vec3 p_world, vec3 v_world, float radius, vec3 center){
	vector<Hit> hVec;

	// we compute the discriminant of the quadratic formula using the ray origin and direction (p_world and v_world):
	vec3 pc = p_world - center;
	
	float a = dot(v_world, v_world);
	float b = 2.0f * dot(v_world, pc);
	float c = dot(pc, pc) - pow(radius, 2);
	float d = (float)pow(b, 2) - 4.0f * a * c;

	if (d < 0.0f){ // there are no intersections:
		// Do nothing
	}else if (d == 0.0f){ // one intersection:
		// SUPER RARE DUE TO FLOATING POINT ERROR
		float t = -b / (2.0f * a);

		vec3 x = vec3(pc) + (t * v_world);
		vec3 n = normalize(x - center);
		Hit sphereHit(x, n, t);
		hVec.push_back(sphereHit);

	}else if (d > 0.0f){ // two intersections:
		float t_1 = (-b + sqrt(d)) / (2.0f * a);
		float t_2 = (-b - sqrt(d)) / (2.0f * a);

		vec3 x = p_world + (t_1 * v_world);
		vec3 n = normalize(x - center);
		Hit sphereHit1(x, n, t_1);
		hVec.push_back(sphereHit1);

		/*vec3*/ x = p_world + (t_2 * v_world);
		/*vec3*/ n = normalize(x - center);
		Hit sphereHit2(x, n, t_2);
		hVec.push_back(sphereHit2);
	}

	return hVec;
}

void printVec(vec3 vec){
    cout << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")\n";
}

void printVec(vec4 vec){
    cout << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", "<< vec.w << ")\n";
}

// Given the parameters for an ellipsoid and a ray's origin and direction, create a hit object
vector<Hit> getEllipsoidIntersections(mat4 E, vec3 p_world, vec3 v_world){
	vector<Hit> hVec;

	mat4 invE = inverse(E);
	vec3 p_prime = invE * vec4(p_world, 1);

	vec3 v_prime = normalize(invE * vec4(v_world, 0));

	// we compute the discriminant of the quadratic formula using the ray origin and direction (p_world and v_world):
	float a = dot(v_prime, v_prime);
	float b = 2.0f * dot(v_prime, p_prime);
	float c = dot(p_prime, p_prime) - 1.0f;
	float d = (float)pow(b, 2) - 4.0f * a * c;

	mat4 itE = transpose(invE);

	if (d < 0.0f){ // there are no intersections:
		// Do nothing
	}else if (d == 0.0f){ // one intersection:
		// SUPER RARE DUE TO FLOATING POINT ERROR
		float t_prime = -b / (2.0f * a);
		vec3 x_prime = p_prime + (t_prime * v_prime);

		vec3 x = E * vec4(x_prime, 1);
		vec3 n =  normalize(vec3(itE * vec4(x_prime, 0.0f)));

		float t = length(x - p_world);
		if (dot(v_world, (x - p_world)) < 0.0f){
			t = -t;
		}

		Hit sphereHit(x, n, t);
		hVec.push_back(sphereHit);
		
	}else if (d > 0.0f){ // two intersections:
		
		// Compute the first hit:
		float t1_prime = (-b + sqrt(d)) / (2.0f * a);
		vec3 x_prime = p_prime + (t1_prime * v_prime);

		vec3 x = E * vec4(x_prime, 1.0f);
		
		vec3 n = normalize(vec3(itE * vec4(x_prime, 0.0f)));

		float t1 = length(x - p_world);
		if (dot(v_world, (x - p_world)) < 0.0f){
			t1 = -t1;
		}

		Hit sphereHit1(x, n, t1);
		hVec.push_back(sphereHit1);

		// Compute the second hit:
		float t2_prime = (-b - sqrt(d))/ (2.0f * a);
		x_prime = p_prime + (t2_prime * v_prime);

		x = E * vec4(x_prime, 1);

		n = normalize(vec3(itE * vec4(x_prime, 0.0f)));

		float t2 = length(x - p_world);

		if (dot(v_world, (x - p_world)) < 0.0f){
			t2 = -t2;
		}

		Hit sphereHit2(x, n, t2);
		hVec.push_back(sphereHit2);		
	}

	return hVec;
}


// Code for scenes 4 and 5:

/* Ray-Triangle Intersection Test Routines          */
/* Different optimizations of my and Ben Trumbore's */
/* code from journals of graphics tools (JGT)       */
/* http://www.acm.org/jgt/                          */
/* by Tomas Moller, May 2000                        */

/*
Copyright 2020 Tomas Akenine-MÃ¶ller

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#define EPSILON 0.000001
#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2];

/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
/* and one CROSS has been moved out from the if-else if-else */
int intersect_triangle3(double orig[3], double dir[3],
			double vert0[3], double vert1[3], double vert2[3],
			double *t, double *u, double *v)
{
   double edge1[3], edge2[3], tvec[3], pvec[3], qvec[3];
   double det,inv_det;

   /* find vectors for two edges sharing vert0 */
   SUB(edge1, vert1, vert0);
   SUB(edge2, vert2, vert0);

   /* begin calculating determinant - also used to calculate U parameter */
   CROSS(pvec, dir, edge2);

   /* if determinant is near zero, ray lies in plane of triangle */
   det = DOT(edge1, pvec);

   /* calculate distance from vert0 to ray origin */
   SUB(tvec, orig, vert0);
   inv_det = 1.0 / det;

   CROSS(qvec, tvec, edge1);

   if (det > EPSILON)
   {
      *u = DOT(tvec, pvec);
      if (*u < 0.0 || *u > det)
	 return 0;

      /* calculate V parameter and test bounds */
      *v = DOT(dir, qvec);
      if (*v < 0.0 || *u + *v > det)
	 return 0;

   }
   else if(det < -EPSILON)
   {
      /* calculate U parameter and test bounds */
      *u = DOT(tvec, pvec);
      if (*u > 0.0 || *u < det)
	 return 0;

      /* calculate V parameter and test bounds */
      *v = DOT(dir, qvec) ;
      if (*v > 0.0 || *u + *v < det)
	 return 0;
   }
   else return 0;  /* ray is parallel to the plane of the triangle */

   *t = DOT(edge2, qvec) * inv_det;
   (*u) *= inv_det;
   (*v) *= inv_det;

   return 1;
}