#include "Camera.h"

// rays argument gets populated with imgSize^2 vectors representing ray directions. d is the distance from the grid to the camera
void Camera::generateRays(vector<vec3> &rays, int imgSize, float d){

    // The challenge here to generate the rays knowing the position of the camera and the number of squares

    float pixel_height = (d * tan(this->fov / 2.0f) * 2.0f) / (float)imgSize;

    for (int x = 0; x < imgSize; x++){

        float theta = atan((pixel_height * (float)((imgSize / 2) - x)) / d);
        
        for (int y = 0; y < imgSize; y++){

            float phi = atan((pixel_height * (float)((imgSize / 2) - y)) / d);

            vec3 ray = normalize(vec3(d * tan(phi), d * tan(theta), -d));
            rays.push_back(ray);
        }
    }
}