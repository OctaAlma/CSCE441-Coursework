#include <iostream>
#include <string>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Image.h"

#include "Camera.h"
#include "Shape.h"
#include "Light.h"
#include "Hit.h"
#include "Scene.h"
#include "MatrixStack.h"

#include <algorithm>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_CTOR_INIT

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// This allows you to skip the `std::` in front of C++ standard library
// functions. You can also say `using std::cout` to be more selective.
// You should never do this in a header file.
using namespace std;

void generateRays(vector<vec3> &rays, Camera &cam, int imgSize, float d){
	// The challenge is to generate the rays knowing the position of the camera and the number of squares

	float pixel_height = (d * tan(cam.fov / 2.0f) * 2.0f) / (float)imgSize;

	for (int x = 0; x < imgSize; x++){

		float theta = atan((pixel_height * (float)((imgSize / 2) - x)) / d);
		
		for (int y = 0; y < imgSize; y++){

			float phi = atan((pixel_height * (float)((imgSize / 2) - y)) / d);

			vec3 ray = normalize(vec3(d * tan(phi), d * tan(theta), -d));
			ray = inverse(cam.rotationMat) * vec4(ray, 1.0f);

			rays.push_back(ray);
		}
	}
}

vec3 convertToRGB(vec3 color){
	return vec3(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f);
}

void printMatrix(mat4 E){
	for (int i = 0; i < 4; i++){
		for (int j = 0; j < 4; j++){
			cout << E[i][j] << " ";
		}
		cout << endl;
	}
}

int main(int argc, char **argv)
{
	if(argc != 4) {
		cout << "Usage: ./A6 <SCENE> <IMAGE SIZE> <IMAGE FILENAME>" << endl;
		return 0;
	}

	int sceneNo = stoi(argv[1]);
	int imgSize = stoi(argv[2]);
	string outputName(argv[3]);

	shared_ptr<Image> img = make_shared<Image>(imgSize, imgSize);

	Camera cam;
	vector<vec3> rays;

	if (sceneNo == 8){
		cam = Camera(vec3(-3, 0, 0), vec3(0,radians(90.0f),0), radians(60.0f), 1.0f);
		generateRays(rays, cam, imgSize, 1.0f);

	}else{
		cam = Camera(vec3(0, 0, 5), vec3(0,0,0), radians(45.0f), 1.0f);
		generateRays(rays, cam, imgSize, 1.0f);
	}

	int startDepth = 0;

	// In scene 4, we only want one reflection:
	if (sceneNo == 4) { startDepth = MAX_DEPTH - 1;} 

	Scene s;
	s.loadScene(sceneNo);

	if (sceneNo == 9){
		// Bonus: motion blur. Render a specific scene multiple times
		// the ball will move in the +x and +z direction
		int FRAMES = 45;
		vector<vector<vec3>> colArray;
		for (int i = 0; i < imgSize; i++){
			vector<vec3> newVec;
			for (int j = 0; j < imgSize; j++){
				newVec.push_back(vec3(0.0f,0.0f,0.0f));
			}
			colArray.push_back(newVec);
		}

		float currTranslation = 0.03f;
		for (int i = 0; i < FRAMES; i++){
			img = make_shared<Image>(imgSize, imgSize);
			for (int x = 0; x < imgSize; x++){
				for (int y = 0; y < imgSize; y++){
					vec3 col = convertToRGB(s.computePixelColor(rays.at(x * imgSize + y), cam.pos, startDepth));
					colArray[imgSize - y - 1][imgSize - x - 1] += col;
					img->setPixel(imgSize - y - 1, imgSize - x - 1, col.r, col.g, col.b);
				}
			}

			// TO PREVENT THE FRAMES FROM BEING OUTPUT, UNCOMMENT THIS LINE OUT:
			//img->writeToFile("../frames/frame" + to_string(i + 1) + ".png");

			// Only move the ball at around half the frames
			// for scene 4 and 5, shapes.at(0) is the blue sphere while shapes.at(1) is the red sphere
			s.shapes.at(0).center += vec3(-currTranslation, 0, currTranslation * 1.2f);
			s.shapes.at(1).center += vec3(0, 0, currTranslation * 1.5f);
		}

		for (int x = 0; x < imgSize; x++){
			for (int y = 0; y < imgSize; y++){
				colArray.at(x).at(y) = (colArray.at(x).at(y)) * (1.0f / (float)FRAMES);
				vec3 currColor = colArray.at(x).at(y);
				img->setPixel(x, y, currColor.r, currColor.g, currColor.b);
			}
		}
	}
	else{

		for (int x = 0; x < imgSize; x++){
			for (int y = 0; y < imgSize; y++){

				vec3 col = convertToRGB(s.computePixelColor(rays.at(x * imgSize + y), cam.pos, startDepth));
				img->setPixel(imgSize - y - 1, imgSize - x - 1, col.r, col.g, col.b);
			}
		}
	}

	img->writeToFile(outputName);
	return 0;
}
