#pragma once
#ifndef MATERIAL
#define MATERIAL

#include "Vec3.h"

class Material{
    public:
        Vec3 ka, kd, ks;
        float s;

        Material(){
            ka = Vec3();
            kd = Vec3();
            ks = Vec3();
            s = 0;
        }
};

#endif