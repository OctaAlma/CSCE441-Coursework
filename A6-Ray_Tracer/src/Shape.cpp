#include "Shape.h"

Shape::Shape(){
    type = -1;    
}

Shape::Shape(glm::vec3 point, glm::vec3 normal){
    type = PLANE;

    p_point = point;
    p_normal = normal;
}

Shape::Shape(glm::vec3 _center, float _radius){
    type = SPHERE;
    this->center = _center;
    this->radius = _radius; 
}

Shape::Shape(glm::vec3 _center, float _radius, bool _isReflective){
    type = SPHERE;
    this->center = _center;
    this->radius = _radius; 
    this->isReflective = _isReflective;

    if (_isReflective){
    	this->diffuse = glm::vec3(0.0f, 0.0f, 0.0f);
        this->specular = glm::vec3(0.0f, 0.0f, 0.0f);
        this->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
    }
}

Shape::Shape(glm::mat4 &E, glm::vec3 center, float radius){
    type = ELLIPSOID;
    this->E = E;
    this->center = center;
    this->radius = radius; 
}

Shape::Shape(std::vector<float> &posBuf, std::vector<float> &norBuf, int posBufIndex){
    type = TRIANGLE;

    vert0[0] = (double)posBuf[posBufIndex];
    vert0[1] = (double)posBuf[posBufIndex+1];
    vert0[2] = (double)posBuf[posBufIndex+2];

    vert1[0] = (double)posBuf[posBufIndex+3];
    vert1[1] = (double)posBuf[posBufIndex+4];
    vert1[2] = (double)posBuf[posBufIndex+5];

    vert2[0] = (double)posBuf[posBufIndex+6];
    vert2[1] = (double)posBuf[posBufIndex+7];
    vert2[2] = (double)posBuf[posBufIndex+8];

    nor0 = glm::vec3(norBuf[posBufIndex], norBuf[posBufIndex+1], norBuf[posBufIndex+2]);
    nor1 = glm::vec3(norBuf[posBufIndex+3], norBuf[posBufIndex+4], norBuf[posBufIndex+5]);
    nor2 = glm::vec3(norBuf[posBufIndex+6], norBuf[posBufIndex+7], norBuf[posBufIndex+8]);

}

Shape::~Shape(){}