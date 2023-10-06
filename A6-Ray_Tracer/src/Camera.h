#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <vector>
#include "MatrixStack.h"
#include <iostream>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace glm;
using std::vector;

class Camera{
    public:
    vec3 pos;

    mat4 rotationMat;

    float fov;
    float aspect;

    Camera(){}

    Camera(vec3 _pos, vec3 _rot, float _fov, float _aspect){
        pos = _pos;
        fov = _fov;

        // the _rot is a vector containing the expected rotations on each axis
        // _rot.x is the rotation on the x axis
        // _rot.y is the rotation on the y axis, etc.
        auto M = std::make_shared<MatrixStack>();
        M->loadIdentity();
        //M->translate(_pos.x, _pos.y, _pos.z);
        M->rotate(_rot.x, 1, 0, 0);
        M->rotate(_rot.y, 0, 1, 0);
        M->rotate(_rot.z, 0, 0, 1);
        rotationMat = M->topMatrix();

        aspect = _aspect;
    }

    void generateRays(vector<vec3> &rays, int imgSize, float d);

    void updateRotation(vec3 _rot){
        auto M = std::shared_ptr<MatrixStack>();
        M->rotate(_rot.x, 1, 0, 0);
        M->rotate(_rot.y, 0, 1, 0);
        M->rotate(_rot.z, 0, 0, 1);
        rotationMat = M->topMatrix();
    }

};

#endif