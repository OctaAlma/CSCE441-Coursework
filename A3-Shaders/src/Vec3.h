#pragma once
#ifndef VEC3
#define VEC3

struct Vec3{
    float x, y, z;

    Vec3(float x, float y, float z){
        this->x = x;
        this->y = y;
        this->z = z;
    }

    Vec3(){
        x = y = z = 0;
    }
};

#endif