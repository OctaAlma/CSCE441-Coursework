#include <cassert>
#include <cstring>
#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include "Camera.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

// New includes
#include "FreeCam.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include <cstdlib>
#include <time.h>

using namespace std;

double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<FreeCam> camera;
shared_ptr<FreeCam> topDownCam;

shared_ptr<Program> prog;

shared_ptr<Shape> bunnyMesh;
shared_ptr<Shape> teapotMesh;
shared_ptr<Shape> sphere;
shared_ptr<Shape> frustum;

shared_ptr<Texture> textureG; // Texture for the ground
shared_ptr<Shape> ground;
glm::mat3 T(1.0f);

#define BUNNY 1
#define TEAPOT 0

bool enableTopDown = false;

int r = 1;

struct Object{
	glm::vec3 color;
	glm::vec3 rotation;
	float scaleMult;
	float shear;

	int objType;
	shared_ptr<Shape> obj;

	Object(int type){
		color = glm::vec3(0,0,0);
		rotation = glm::vec3(0,0,0);

		scaleMult = (float)((((rand()) + r)%121)+50) * 0.1f;
		r = rand();
		shear = 1;

		objType = type;


		if (type == BUNNY){
			obj = bunnyMesh;
		}else{
			obj = teapotMesh;
		}
	}
};


bool keyToggles[256] = {false}; // only for English keyboards!

// This function is called when a GLFW error occurs
static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

// This function is called when a key is pressed
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

// This function is called when the mouse is clicked
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	// Get the current mouse position.
	double xmouse, ymouse;
	glfwGetCursorPos(window, &xmouse, &ymouse);
	// Get current window size.
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if(action == GLFW_PRESS) {
		bool shift = (mods & GLFW_MOD_SHIFT) != 0;
		bool ctrl  = (mods & GLFW_MOD_CONTROL) != 0;
		bool alt   = (mods & GLFW_MOD_ALT) != 0;
		camera->mouseClicked((float)xmouse, (float)ymouse, shift, ctrl, alt);
	}
}

// This function is called when the mouse moves
static void cursor_position_callback(GLFWwindow* window, double xmouse, double ymouse)
{
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if(state == GLFW_PRESS) {
		camera->mouseMoved((float)xmouse, (float)ymouse);
	}
}

/*------------------------ SHADER SELECTION CODE ------------------------*/

vector<shared_ptr<Program>> shaderVec;
int shaderIndex = 0;

void addShader(string vectorShaderName, string fragShaderName){
	shared_ptr<Program> sp= make_shared<Program>();
	sp->setShaderNames(RESOURCE_DIR + vectorShaderName, RESOURCE_DIR + fragShaderName);
	sp->setVerbose(true);
	sp->init();
	sp->addAttribute("aPos");
	sp->addAttribute("aNor");
	sp->addUniform("MV");
	sp->addUniform("P");

	// New stuff:
	sp->addUniform("itMV"); // Inverse Transpose Model View Matrix;
	sp->addUniform("lightPos");
	sp->addUniform("lightCol");

	sp->addUniform("ka");
	sp->addUniform("kd");
	sp->addUniform("ks");
	sp->addUniform("s");

	sp->setVerbose(false);

	// Add the ground texture information
	sp->addAttribute("aTex");
	sp->addUniform("T");
	sp->addUniform("textureGround");

	// Initialize ground texture:
	textureG = make_shared<Texture>();
	textureG->setFilename(RESOURCE_DIR + "grass-1.jpeg");
	textureG->init();
	textureG->setUnit(0);
	textureG->setWrapModes(GL_REPEAT, GL_REPEAT); 

	shaderVec.push_back(sp);
}

void initShaders(){
	vector<string> attributeVec = {};
	vector<string> uniformVec = {"MV", "P"};

	addShader("normal_vert.glsl", "blinn-phong.glsl");

}

shared_ptr<Program> getCurrShader(){
	return shaderVec[shaderIndex];
}

/*------------------------ MATERIAL SELECTION CODE ------------------------*/

vector<Material> materialVec;
int materialIndex = 1;

Material currMaterial;

void initMaterials(){

	Material m;
	m.ka = Vec3(0.5, 0.5, 0.5);
	m.kd = Vec3(0.3, 0.3, 0.3); // 
	m.ks = Vec3(0.5, 0.5, 0.5); // this causes the green highlight
	m.s  = 100;
	materialVec.push_back(m);

	// Set the current material to be m1;
	currMaterial = materialVec.at(0);
}

/*------------------------ LIGHT SELECTION CODE ------------------------*/

vector<Light> lightVec;
int lightIndex = 0;

void initLights(){
	Light l1(7.0, 4, 5.5, 0.8, 0.8, 0.8);

	lightVec.push_back(l1);
}

/*------------------------ NEW STUFF DONE ------------------------*/

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];

	// The s key will toggle through shaders.
	if (tolower(key) == 'w' || tolower(key) == 'a' || tolower(key) == 's' || tolower(key) == 'd'){
		camera->move(tolower(key));
	}

	else if (key == 'z'){
		// Increase FOV. Note: the cap is 114
		camera->fovy+=0.05;

		if (camera->fovy > 1.98968){
			camera->fovy = 1.98968;
		}
	}

	else if (key == 'Z'){
		// Decrease DOV. Note: the min is 4 degrees 
		camera->fovy-=0.05;

		if (camera->fovy < 0.0698132){
			camera->fovy = 0.0698132;
		}
	}


	// The m keys will Toggle through the different materials
	else if (tolower(key) == 'm'){
		if (key == 'm'){
			materialIndex = (materialIndex + 1) % materialVec.size();
		}else{
			if (!materialIndex){ 
				materialIndex = materialVec.size() - 1; 
			}else { 
				materialIndex--; 
			}
		}

		currMaterial = materialVec[materialIndex];
	}

	else if (tolower(key) == 't'){
		enableTopDown = !enableTopDown;
	}

}

// If the window is resized, capture the new size and reset the viewport
static void resize_callback(GLFWwindow *window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// https://lencerf.github.io/post/2019-09-21-save-the-opengl-rendering-to-image-file/
static void saveImage(const char *filepath, GLFWwindow *w)
{
	int width, height;
	glfwGetFramebufferSize(w, &width, &height);
	GLsizei nrChannels = 3;
	GLsizei stride = nrChannels * width;
	stride += (stride % 4) ? (4 - stride % 4) : 0;
	GLsizei bufferSize = stride * height;
	std::vector<char> buffer(bufferSize);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());
	stbi_flip_vertically_on_write(true);
	int rc = stbi_write_png(filepath, width, height, nrChannels, buffer.data(), stride);
	if(rc) {
		cout << "Wrote to " << filepath << endl;
	} else {
		cout << "Couldn't write to " << filepath << endl;
	}
}

vector<shared_ptr<Object>> objects;
vector<shared_ptr<Object>> hudObjects;

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// NEW CODE:
	initShaders();
	initMaterials();
	initLights();

	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	prog = getCurrShader(); // Gets the currently selected shader from shaderVec()
	
	camera = make_shared<FreeCam>();
	camera->setInitDistance(5.0f); // Camera's initial Z translation

	topDownCam = make_shared<FreeCam>();
	topDownCam->setInitDistance(5.0f);
	
	string model;

	int colorIndex = 0;

	// Load the bunny and teapot models:
	bunnyMesh = make_shared<Shape>();
	bunnyMesh->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunnyMesh->init();

	teapotMesh = make_shared<Shape>();
	teapotMesh->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapotMesh->init();

	// Load the sun/sphere model:
	sphere = make_shared<Shape>();
	sphere->loadMesh(RESOURCE_DIR + "sphere.obj");
	sphere->init();

	// Load the frustum model:
	frustum = make_shared<Shape>();
	frustum->loadMesh(RESOURCE_DIR + "Frustum.obj");
	frustum->init();


	for (int i = 0; i < 121; i++){
		shared_ptr<Object> o = nullptr;

		if ((i) % 2 == 0){
			o = make_shared<Object>(BUNNY);
		}else{
			o = make_shared<Object>(TEAPOT);
		}

		o->color = glm::vec3(RANDOM_COLORS[colorIndex][0],RANDOM_COLORS[colorIndex][1],RANDOM_COLORS[colorIndex][2] );


		colorIndex = (colorIndex + 1) % 7;
		objects.push_back(o);
	}

	ground = make_shared<Shape>();
	ground->loadMesh(RESOURCE_DIR + "square.obj");
	ground->init();

	T[0][0] = 25.0f; 
	T[1][1] = 25.0f; 

	// Create the HUD bunny and teapot:
	shared_ptr<Object> o = nullptr;
	o = make_shared<Object>(TEAPOT);
	o->color = glm::vec3(0.3, 0.3, 0.3); // Make the color grayish
	hudObjects.push_back(o);

	o = make_shared<Object>(BUNNY);
	o->color = glm::vec3(0.3, 0.3, 0.3); // Make the color grayish
	hudObjects.push_back(o);
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'f']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	glViewport(0, 0, width, height);
	
	double t = glfwGetTime();
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	prog->bind();

	// Pass light and color information for HUD components to GPU:
	glUniform3f(prog->getUniform("ka"), 0.2, 0.2, 0.2);
	glUniform3f(prog->getUniform("kd"), 0.9, 0.9, 0.9);
	glUniform3f(prog->getUniform("ks"), 0.5, 0.5, 0.5);
	glUniform3f(prog->getUniform("lightPos"), 0,0,0);
	glUniform3f(prog->getUniform("lightCol"), 1,1,1);
	glUniform1f(prog->getUniform("s"), 200);

	P->pushMatrix();
		float wN = (float)width/(float)height;
        float hN = (float)height/(float)width;

		// Draw the HUD teapot:
		MV->pushMatrix();

			MV->translate(-0.75f, 0.5f, 0);
			//MV->translate(0.75f * wN, 0.5f, 0);			
			MV->scale(-0.25 * hN, 0.25,-0.25);
			MV->rotate(t, 0, -1, 0);
			MV->translate(0, 0.333099, 0); 


			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

			hudObjects.at(0)->obj->draw(prog);

		MV->popMatrix();
		
		// Draw the HUD bunny:
		MV->pushMatrix();
			
			MV->translate(0.75f, 0.5f, 0);
			//MV->translate(0.75f * wN, 0.5f, 0);			
			MV->scale(-0.25 * hN, 0.25, 0.25);
			MV->rotate(t, 0, 1, 0);

			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

			hudObjects.at(1)->obj->draw(prog);

		MV->popMatrix();
	P->popMatrix();

	// Apply camera transforms
	P->pushMatrix();
	
		camera->applyProjectionMatrix(P);

	MV->pushMatrix();

		camera->applyViewMatrix(MV);
	
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));	

		// Pass the material and light information:
		glm::vec3 lPos = MV->topMatrix() * glm::vec4(lightVec.at(0).x(), lightVec.at(0).y(), lightVec.at(0).z(), 1);

		glUniform3f(prog->getUniform("lightPos"), lPos.x, lPos.y, lPos.z);
		glUniform3f(prog->getUniform("lightCol"), lightVec.at(0).r(), lightVec.at(0).g(), lightVec.at(0).b());

		// Pass texture information
		textureG->bind(prog->getUniform("textureGround"));
		glUniformMatrix3fv(prog->getUniform("T"), 1, GL_FALSE, glm::value_ptr(T));
		
		// Draw the floor:
		MV->pushMatrix();
			MV->rotate(-1.0 * M_PI_2, 1, 0, 0); // Rotate -pi/2 radians on x axis
			MV->scale(50, 50, 1); // Scale the x and y axis so it can cover the whole plane

			glUniform3f(prog->getUniform("ka"), 0.2, 0.2, 0.2);
			glUniform3f(prog->getUniform("kd"), 0.2, 0.2, 0.2);
			glUniform3f(prog->getUniform("ks"), 0.2, 0.2, 0.2);
			glUniform1f(prog->getUniform("s"), currMaterial.s);


			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			ground->draw(prog);
		MV->popMatrix();

		// Draw the sun:
		MV->pushMatrix();
			glUniform1f(prog->getUniform("s"), currMaterial.s);

			//MV->translate(lPos.x, lPos.y, lPos.z);
			MV->translate(lightVec.at(0).x(), lightVec.at(0).y(), lightVec.at(0).z());
			glUniform3f(prog->getUniform("ka"), 1, 1, 0);
			glUniform3f(prog->getUniform("kd"), 0, 0, 0);
			glUniform3f(prog->getUniform("ks"), 0, 0, 0);

			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			sphere->draw(prog);
		MV->popMatrix();

		int i = 0; 
		glUniform3f(prog->getUniform("ka"), currMaterial.ka.x, currMaterial.ka.y, currMaterial.ka.z);
		glUniform3f(prog->getUniform("kd"), currMaterial.kd.x, currMaterial.kd.y, currMaterial.kd.z);
		glUniform3f(prog->getUniform("ks"), currMaterial.ks.x, currMaterial.ks.y, currMaterial.ks.z);
		glUniform1f(prog->getUniform("s"), currMaterial.s);

		for (int x = -5; x < 6; x++){
			
			for (int z = -5; z < 6; z++){

					float scale = sin(objects.at(i)->scaleMult * t * 0.1) / 6;

					glUniform3f(prog->getUniform("ka"), objects.at(i)->color.x * 0.1, objects.at(i)->color.y * 0.1, objects.at(i)->color.z * 0.1);
					glUniform3f(prog->getUniform("kd"), objects.at(i)->color.x, objects.at(i)->color.y, objects.at(i)->color.z);
					MV->pushMatrix();
						
						MV->translate(0.0f + x, 0.0f, 0.0f + z);
						
						// If what we are about to draw is a bunny, we must translate down in y:
						// The min y of the bunny is: 0.333099
						if (objects.at(i)->objType == BUNNY){
							MV->translate(0.0f, -0.333099 * (scale + 0.25f), 0.0f);
						}

						
						MV->scale(0.25f + scale, 0.25f + scale, 0.25f + scale);

						MV->rotate(0.785398 + abs(z), 0, 1, 0);
						glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
						glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

						objects.at(i)->obj->draw(prog);
					MV->popMatrix();

				i++;
			}
		}
	
	MV->popMatrix();

	P->popMatrix();

	prog->unbind();

	// Task 5
	if (enableTopDown == true){

		double s = 0.5;
		glViewport(0, 0, s*width, s*height);
		glEnable(GL_SCISSOR_TEST);
		glScissor(0, 0, s*width, s*height);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_SCISSOR_TEST);

		topDownCam->setAspect((float)width/(float)height);
		prog->bind();

		P->pushMatrix();
			MV->pushMatrix();

				// Apply projection matrix for Top-Down viewport:
				topDownCam->applyOrthoMatrix(P);

				// Apply view matrix for Top-down viewport:
				MV->rotate(M_PI_2, 1, 0, 0);
				MV->translate(0, -15, 0);
				topDownCam->applyViewMatrix(MV);

				// Draw the frustum:
				MV->pushMatrix();
					float a = (float)width/(float)height; // aspect ratio
					float xS = a * tan(camera->fovy / 2.0f);
					float yS = tan(camera->fovy / 2.0f);

					glm::vec3 forwardDir = glm::vec3(sin(camera->yaw), sin(camera->pitch), cos(camera->yaw));
					glm::mat4 camMatrix = glm::inverse(glm::lookAt(camera->position, camera->position + forwardDir, glm::vec3(0, 1, 0)));

					MV->multMatrix(camMatrix);

					MV->scale(xS, yS, 1);
					
					glUniform3f(prog->getUniform("ka"), 0.0, 0.0, 0.0);
					glUniform3f(prog->getUniform("kd"), 0.0, 0.0, 0.0);
					glUniform3f(prog->getUniform("ks"), 0.0, 0.0, 0.0);

					glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));
					glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

					frustum->draw(prog);

				MV->popMatrix();

				glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));

				// Pass the material and light information:
				glm::vec3 lPos = MV->topMatrix() * glm::vec4(lightVec.at(0).x(), lightVec.at(0).y(), lightVec.at(0).z(), 1);

				glUniform3f(prog->getUniform("lightPos"), lPos.x, lPos.y, lPos.z);
				glUniform3f(prog->getUniform("lightCol"), lightVec.at(0).r(), lightVec.at(0).g(), lightVec.at(0).b());

				// Pass texture information
				textureG->bind(prog->getUniform("textureGround"));
				glUniformMatrix3fv(prog->getUniform("T"), 1, GL_FALSE, glm::value_ptr(T));
				
				// Draw the floor:
				MV->pushMatrix();
					MV->rotate(-1.0 * M_PI_2, 1, 0, 0); // Rotate -pi/2 radians on x axis
					MV->scale(50, 50, 1); // Scale the x and y axis so it can cover the whole plane

					glUniform3f(prog->getUniform("ka"), 0.2, 0.2, 0.2);
					glUniform3f(prog->getUniform("kd"), 0.2, 0.2, 0.2);
					glUniform3f(prog->getUniform("ks"), 0.2, 0.2, 0.2);
					glUniform1f(prog->getUniform("s"), currMaterial.s);


					glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					ground->draw(prog);
				MV->popMatrix();

				// Draw the sun:
				MV->pushMatrix();
					glUniform1f(prog->getUniform("s"), currMaterial.s);

					//MV->translate(lPos.x, lPos.y, lPos.z);
					MV->translate(lightVec.at(0).x(), lightVec.at(0).y(), lightVec.at(0).z());
					glUniform3f(prog->getUniform("ka"), 1, 1, 0);
					glUniform3f(prog->getUniform("kd"), 0, 0, 0);
					glUniform3f(prog->getUniform("ks"), 0, 0, 0);

					glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
					glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
					sphere->draw(prog);
				MV->popMatrix();

				int i = 0; 
				glUniform3f(prog->getUniform("ka"), currMaterial.ka.x, currMaterial.ka.y, currMaterial.ka.z);
				glUniform3f(prog->getUniform("kd"), currMaterial.kd.x, currMaterial.kd.y, currMaterial.kd.z);
				glUniform3f(prog->getUniform("ks"), currMaterial.ks.x, currMaterial.ks.y, currMaterial.ks.z);
				glUniform1f(prog->getUniform("s"), currMaterial.s);

				for (int x = -5; x < 6; x++){
					
					for (int z = -5; z < 6; z++){
							float scale = sin(objects.at(i)->scaleMult * t * 0.1) / 6;

							glUniform3f(prog->getUniform("ka"), objects.at(i)->color.x * 0.1, objects.at(i)->color.y * 0.1, objects.at(i)->color.z * 0.1);
							glUniform3f(prog->getUniform("kd"), objects.at(i)->color.x, objects.at(i)->color.y, objects.at(i)->color.z);
							MV->pushMatrix();
								
								MV->translate(0.0f + x, 0.0f, 0.0f + z);
								
								// If what we are about to draw is a bunny, we must translate down in y:
								// The min y of the bunny is: 0.333099
								if (objects.at(i)->objType == BUNNY){
									MV->translate(0.0f, -0.333099 * (scale + 0.25f), 0.0f);
								}

								
								MV->scale(0.25f + scale, 0.25f + scale, 0.25f + scale);

								MV->rotate(0.785398 + abs(z), 0, 1, 0);
								glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
								glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));

								objects.at(i)->obj->draw(prog);
							MV->popMatrix();

						i++;
					}
				}
			MV->popMatrix();
		P->popMatrix();

		prog->unbind();
	}

	
	GLSL::checkError(GET_FILE_LINE);
	
	if(OFFLINE) {
		saveImage("output.png", window);
		GLSL::checkError(GET_FILE_LINE);
		glfwSetWindowShouldClose(window, true);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Usage: A4 RESOURCE_DIR" << endl;
		return 0;
	}
	RESOURCE_DIR = argv[1] + string("/");
	
	// Optional argument
	if(argc >= 3) {
		OFFLINE = atoi(argv[2]) != 0;
	}

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "OCTAVIO ALMANZA", NULL, NULL);
	if(!window) {
		glfwTerminate();
		return -1;
	}
	// Make the window's context current.
	glfwMakeContextCurrent(window);
	// Initialize GLEW.
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		cerr << "Failed to initialize GLEW" << endl;
		return -1;
	}
	glGetError(); // A bug in glewInit() causes an error that we can safely ignore.
	cout << "OpenGL version: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
	GLSL::checkVersion();

	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	// Set char callback.
	glfwSetCharCallback(window, char_callback);
	// Set cursor position callback.
	glfwSetCursorPosCallback(window, cursor_position_callback);
	// Set mouse button callback.
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	// Set the window resize call back.
	glfwSetFramebufferSizeCallback(window, resize_callback);
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		// Render scene.
		render();
		// Swap front and back buffers.
		glfwSwapBuffers(window);
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
