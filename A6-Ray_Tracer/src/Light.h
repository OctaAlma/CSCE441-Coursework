#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

using namespace glm;

class Light{
    public:
    glm::vec3 pos;
    glm::vec3 col;

    Light();
    // Pass position and intensity
    Light(float x, float y, float z, float i);

    // Pass position, color, and intensity
    Light(float x, float y, float z, float r, float g, float b, float i);

    // Pass position and color as vectors and intensity:
    Light(vec3 &_pos, vec3 &_col, float i);

    ~Light();
};

#endif