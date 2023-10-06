#pragma once
#ifndef HIT_H
#define HIT_H

#include <memory>
#include <vector>

#include "Shape.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
using namespace glm;
using std::vector;

class Hit{
	public: 
    Hit() {
        this->x = vec3(0,0,0); 
        this->n = vec3(0,0,0);
        this->t = INFINITY;
    }
    
	Hit(const vec3 &x, const vec3 &n, float t) { this->x = x; this->n = n; this->t = t; }
	vec3 x; // position
	vec3 n; // normal
	float t; // distance

	vec3 color;

	Shape* shapeHit = nullptr;
};

Hit getPlaneIntersection(vec3 p_world, vec3 v_world, vec3 c, vec3 n);

vector<Hit> getSphereIntersections(vec3 p_world, vec3 v_world, float radius, vec3 center);

vector<Hit> getEllipsoidIntersections(mat4 E, vec3 p_world, vec3 v_world);

int intersect_triangle3(double orig[3], double dir[3],
			double vert0[3], double vert1[3], double vert2[3],
			double *t, double *u, double *v);
#endif