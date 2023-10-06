#pragma once
#ifndef LIGHT
#define LIGHT

#include "Vec3.h"

class Light{
    public:
        Vec3 lightPos;
        Vec3 lightCol;

        Light(){
            lightPos = Vec3(0,0,0);
            lightCol = Vec3(1,1,1);
        }

        Light(float xPos, float yPos, float zPos, float r, float g, float b){
            lightPos = Vec3(xPos, yPos, zPos);
            lightCol = Vec3(r, g, b);
        }

        float x(){
            return lightPos.x;
        }

        float y(){
            return lightPos.y;
        }

        float z(){
            return lightPos.z;
        }

        float r(){
            return lightCol.x;
        }

        float g(){
            return lightCol.y;
        }

        float b(){
            return lightCol.z;
        }


};

#endif