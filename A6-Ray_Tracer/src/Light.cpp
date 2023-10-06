#include "Light.h"

Light::Light(){
    pos = vec3(0,0,0);
    col = vec3(1,1,1);
}

Light::Light(float x, float y, float z, float i){
    pos = vec3 (x, y, z);

    col = vec3(1.0f, 1.0f, 1.0f) * i;
}

Light::Light(float x, float y, float z, float r, float g, float b, float i){
    pos = vec3 (x, y, z);
    col = vec3 (r, g, b);

}

Light::Light(vec3 &_pos, vec3 &_col, float i){
    pos = _pos;
    col = _col;

}

Light::~Light(){}