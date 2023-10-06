#pragma once
#ifndef SCENE_H
#define SCENE_H

#define MAX_DEPTH 8

#include "Light.h"
#include "Shape.h"
#include "Hit.h"
#include "Camera.h"

#include "tiny_obj_loader.h"


#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;
using std::cerr;

#define BUNNY_SCENE_1 6
#define BUNNY_SCENE_2 7
#define BACKGROUND_COLOR vec3(0.0f, 0.0f, 0.0f)

class Scene{

    private:
    void printMatrix(mat4 m);
    mat4 makeScene7Matrix(int sceneNo);
    vec3 getReflectedRay(vec3 view, vec3 normal);
    bool samePos(vec3 pos1, vec3 pos2);
    Hit getNearestHit(vector<Hit> &allHits, vec3 rayOrigin);
    vec3 blinnPhong(Hit h, vec3 rayOrigin);
    void loadGeometry(string &meshName, vector<float> &posBuf, vector<float> &norBuf, vector<float> &texBuf);

    public:
    vector<Shape> shapes;
    vector<Light> lights;

    // For scenes 6 and 7:
    int sceneNo = -1;
    vec3 sphereCenter;
    float radius;
    mat4 E;

    void loadScene(int _sceneNo);

    vector<Hit> getAllHits(vec3 rayDir, vec3 rayOrigin, float maxDist);

    vec3 specularAndDiffuse(Hit hit, vec3 camPos, int lightIndex);
        
    vec3 computePixelColor(vec3 rayDir, vec3 rayOrigin, int depth);

};

vec3 normalizeColors(vec3 color);

#endif