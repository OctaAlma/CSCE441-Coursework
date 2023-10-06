#pragma once
#ifndef SHAPE_H
#define SHAPE_H

#include <memory>
#include <vector>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#define PLANE 0
#define SPHERE 1
#define ELLIPSOID 2
#define TRIANGLE 3

class Shape{
    public: 
    int type;

    // For plane:
    glm::vec3 p_point;
    glm::vec3 p_normal;

    // For sphere:
    glm::vec3 center;
    float radius;

    bool isReflective = false;

    // For ellipsoid:
    glm::mat4 E;

    // For triangles:
    double vert0[3];
    double vert1[3];
    double vert2[3];
    glm::vec3 nor0;
    glm::vec3 nor1;
    glm::vec3 nor2;

    // Lighting Characteristics:
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 ambient;
    float s = 1;

    Shape();
    // Construct a plane
    Shape(glm::vec3 point, glm::vec3 normal);

    // Construct a sphere
    Shape(glm::vec3 _center, float _radius);

    // Construct a reflective sphere:
    Shape(glm::vec3 _center, float _radius, bool _isReflective);

    // Construct an ellipsoid
    Shape(glm::mat4 &E, glm::vec3 center, float radius);

    // Construct a triangle
    Shape(std::vector<float> &posBuf, std::vector<float> &norBuf, int posBufIndex);

    ~Shape();
};

#endif