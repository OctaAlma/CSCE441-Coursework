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

// New inclue
#include "Material.h"
#include "Texture.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Program> progPass1;
shared_ptr<Program> progPass2;

shared_ptr<Shape> teapotMesh;
shared_ptr<Shape> bunnyMesh;
shared_ptr<Shape> sphereMesh;
shared_ptr<Shape> squareMesh;

map<string,GLuint> ballBufIDs;
vector<float> ballPosBuf;
vector<float> ballNorBuf;

map<string, GLuint> sorBufIDs;
vector<float> sorPosBuf;
vector<float> sorNorBuf;
vector<unsigned int>sorIndBuf;

vector<unsigned int> indBuf;
vector<float> texBuf;

int textureWidth = 640;
int textureHeight = 480;
GLuint framebufferID;
GLuint textureA;
GLuint textureB;
GLuint textureC;
GLuint textureD;

#define BUNNY 0
#define TEAPOT 1
#define BALL 2
#define SOR 3

double RANDOM_COLORS[7][3] = {
	{0.0000,    0.4470,    0.7410},
	{0.8500,    0.3250,    0.0980},
	{0.9290,    0.6940,    0.1250},
	{0.4940,    0.1840,    0.5560},
	{0.4660,    0.6740,    0.1880},
	{0.3010,    0.7450,    0.9330},
	{0.6350,    0.0780,    0.1840},
};


struct Object{
	glm::vec3 ke;
	glm::vec3 kd;
	glm::vec3 ks;

	float rotation;
	float scaleMult;
	float shear;
	float timeMult;

	int objType;
	shared_ptr<Shape> obj;

	Object(int type){
		ke = glm::vec3(0.0, 0.0, 0.0);
		kd = glm::vec3(((float)(rand()%100))/100.0f, ((float)(rand()%100))/100.0f, ((float)(rand()%100))/100.0f);
		ks = glm::vec3(1.0, 0.9, 0.8);

		scaleMult = (float)(((rand())%121)+50.0f) * 0.5f;
		shear = 1;

		objType = type;

		if (type == BUNNY){
			obj = bunnyMesh;
			rotation = (float)(rand()%300);

		}else if (type == TEAPOT){
			obj = teapotMesh;
			shear = (float)(rand()%300);

		}else if (type == BALL){
			obj = nullptr;
			scaleMult = (float)(rand()%300) / 225.0f + 0.5f;
			timeMult = 2.0f / (rand()%10) + 0.5;

		}else if (type == SOR){
			obj = nullptr;
			timeMult = (float)(rand()%300) / 150.0f + 0.5f;
		}
	}
};


/**************************************************************************************/

void initModel(map<string, GLuint>& bufIDs, vector<float>& posBuf, vector<float>& norBuf, vector<float>& texBuf)
{
    unsigned posBufID = bufIDs["bPos"];
	unsigned norBufID = bufIDs["bNor"];
	unsigned texBufID = bufIDs["bTex"];
	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(!norBuf.empty()) {
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(!texBuf.empty()) {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	GLSL::checkError(GET_FILE_LINE);
	
}

void drawModel(shared_ptr<Program> prog, map<string, GLuint>& bufIDs, vector<float>& posBuf, vector<float>& norBuf, vector<float>& texBuf, vector<unsigned int>& indBuf){
	glEnableVertexAttribArray(prog->getAttribute("aPos"));
	GLSL::checkError(GET_FILE_LINE);
	glEnableVertexAttribArray(prog->getAttribute("aNor"));
	GLSL::checkError(GET_FILE_LINE);
	glEnableVertexAttribArray(prog->getAttribute("aTex"));
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bPos"]);
	glVertexAttribPointer(prog->getAttribute("aPos"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bNor"]);
	glVertexAttribPointer(prog->getAttribute("aNor"), 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, bufIDs["bTex"]);
	glVertexAttribPointer(prog->getAttribute("aTex"), 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufIDs["bInd"]);

	int indCount = indBuf.size();

	glDrawElements(GL_TRIANGLES, indCount, GL_UNSIGNED_INT, (void *)0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableVertexAttribArray(prog->getAttribute("aTex"));
	glDisableVertexAttribArray(prog->getAttribute("aNor"));
	glDisableVertexAttribArray(prog->getAttribute("aPos"));
	GLSL::checkError();
}

/**************************************************************************************/

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

/*------------------------ CREATE OBJECTS ------------------------*/

vector<shared_ptr<Object>> objects;

void initObjects(){
	int colorIndex = 0;

	for (int i = 0; i < 121; i++){
		shared_ptr<Object> o = nullptr;

		int objType = (i%4);

		if (objType == BUNNY){
			o = make_shared<Object>(BUNNY);

		}else if (objType == TEAPOT){
			o = make_shared<Object>(TEAPOT);

		}else if (objType == BALL){
			o = make_shared<Object>(BALL);

		}else{
			o = make_shared<Object>(SOR);
		}
		
		colorIndex = (colorIndex + 1) % 7;
		objects.push_back(o);
	}
}

/*------------------------ CREATE LIGHTS ------------------------*/

// vector<glm::vec3> lights;
vector<glm::vec3> lightPos;
vector<glm::vec3> lightCol;
vector<float> lPosAdd;


void initLights(){
	int colorIndex = 0;

	for (int i = 0; i < 15; i++){
		// Lights must have a y value that is AT MOST HALF the height of the objects
		//glm::vec3 l(3.0f * ((float)(rand() % 64)/8.0f - 4.0f), (float)(rand()%500)/1000.0f, 3.0f * ((float)(rand() % 64)/8.0f - 4.0f));
		glm::vec3 l(3.0f * ((float)(rand() % 64)/8.0f - 4.0f), 0.75f, 3.0f * ((float)(rand() % 64)/8.0f - 4.0f));
		rand();

		lPosAdd.push_back((float)(rand()%3200)/100.0 - 8.0f);

		lightPos.push_back(l);
		lightCol.push_back(glm::vec3(RANDOM_COLORS[colorIndex][0],RANDOM_COLORS[colorIndex][1],RANDOM_COLORS[colorIndex][2]));
		colorIndex = (colorIndex + 1) % 7;
	}
}

/*------------------------ MATERIAL SELECTION CODE ------------------------*/

vector<Material> materialVec;
int materialIndex = 0;

Material currMaterial;

void initMaterials(){

	// // Material 1: pinkish with strong highlights
	Material m1;
	m1.ka = Vec3(0.2, 0.2, 0.2);
	m1.kd = Vec3(0.8, 0.7, 0.7);
	m1.ks = Vec3(1.0, 0.9, 0.8);
	m1.s  = 200;
	materialVec.push_back(m1);

	// Set the current material to be m1;
	currMaterial = materialVec.at(0);
}

/*------------------------ NEW STUFF DONE ------------------------*/

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];
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
		std::cout << "Wrote to " << filepath << endl;
	} else {
		std::cout << "Couldn't write to " << filepath << endl;
	}
}

void generateTextures(){
	// Create textureA as the output of the offscreen framebuffer
	glGenTextures(1, &textureA);
	glBindTexture(GL_TEXTURE_2D, textureA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureA, 0);

	// Create textureB as another output of the offscreen framebuffer. Note that we use GL_COLOR_ATTACHMENT1 instead of 0 at the last line
	glGenTextures(1, &textureB);
	glBindTexture(GL_TEXTURE_2D, textureB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textureB, 0);

	// Create textureC as the output of the offscreen framebuffer
	glGenTextures(1, &textureC);
	glBindTexture(GL_TEXTURE_2D, textureC);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, textureC, 0);

	// Create textureD as another output of the offscreen framebuffer.
	glGenTextures(1, &textureD);
	glBindTexture(GL_TEXTURE_2D, textureD);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, textureD, 0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, textureWidth, textureHeight, 0, GL_RGB, GL_FLOAT, NULL);

	// Enable depth tests on the offscreen buffers during pass 1 by binding the frame buffer:
	GLuint depthrenderbuffer;
	glGenRenderbuffers(1, &depthrenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, textureWidth, textureHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

	// Now we tell OpenGL that we want FOUR textures as the output of this framebuffer:
	GLenum attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
	glDrawBuffers(4, attachments);

	// check everything is ok:
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    	cerr << "Framebuffer is not ok" << endl;
	}
}

// This function is called once to initialize the scene and OpenGL
static void init()
{
	// NEW CODE:
	initMaterials();

	// Initialize time.
	glfwSetTime(0.0);
	
	// Set background color.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);
	
	camera = make_shared<Camera>();
	camera->setInitDistance(20.0f); // Camera's initial Z translation

	progPass1 = make_shared<Program>();
	progPass1->setShaderNames(RESOURCE_DIR + "pass1_vert.glsl", RESOURCE_DIR + "pass1_frag.glsl");
	progPass1->setVerbose(false);
	progPass1->init();
	progPass1->addAttribute("aPos");
	progPass1->addAttribute("aNor");
	progPass1->addAttribute("aTex");
	progPass1->addUniform("P");
	progPass1->addUniform("MV");
	progPass1->addUniform("itMV");
	progPass1->addUniform("isSor");
	progPass1->addUniform("t");
	progPass1->addUniform("ke");
	progPass1->addUniform("kd");

	progPass2 = make_shared<Program>();
	progPass2->setShaderNames(RESOURCE_DIR + "pass2_vert.glsl", RESOURCE_DIR + "pass2_frag.glsl");
	progPass2->setVerbose(false);
	progPass2->init();
	progPass2->addAttribute("aPos");
	progPass2->addUniform("P");
	progPass2->addUniform("MV");
	progPass2->addUniform("windowSize");
	progPass2->addUniform("posTexture");
	progPass2->addUniform("norTexture");
	progPass2->addUniform("keTexture");
	progPass2->addUniform("kdTexture");
	progPass2->addUniform("ks");
	progPass2->addUniform("s");
	progPass2->addUniform("lPosBuf");
	progPass2->addUniform("lColBuf");

	progPass2->bind();
		glUniform1i(progPass2->getUniform("posTexture"), 0);
		glUniform1i(progPass2->getUniform("norTexture"), 1);
		glUniform1i(progPass2->getUniform("keTexture"), 2);
		glUniform1i(progPass2->getUniform("kdTexture"), 3);
	progPass2->unbind();
	
	bunnyMesh = make_shared<Shape>();
	bunnyMesh->loadMesh(RESOURCE_DIR + "bunny.obj");
	bunnyMesh->init();

	teapotMesh = make_shared<Shape>();
	teapotMesh->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapotMesh->init();

	sphereMesh = make_shared<Shape>();
	sphereMesh->loadMesh(RESOURCE_DIR + "sphere.obj");
	sphereMesh->init();

	squareMesh = make_shared<Shape>();
	squareMesh->loadMesh(RESOURCE_DIR + "square.obj");
	squareMesh->init();

	int rowParam(50), colParam(50);

	// Task 2: Sphere
	for (int row = 0; row < rowParam; row++){
		for (int col = 0; col < colParam; col++){
			
			// Bouncing ball stuff:
			float theta = M_PI * (float)row/(float)(rowParam-1);
			float phi = 2 * M_PI * (float)col/(float)(colParam-1);

			float x = sin(theta) * sin(phi);
			float y = cos(theta);
			float z = sin(theta) * cos(phi);

			ballPosBuf.push_back(x * 0.5);
			ballPosBuf.push_back(y * 0.5);
			ballPosBuf.push_back(z * 0.5);

			// Note: for a unit sphere at the origin, the normals are the same as a position
			glm::vec3 nor(x, y ,z);
			ballNorBuf.push_back(nor.x);
			ballNorBuf.push_back(nor.y);
			ballNorBuf.push_back(nor.z);

			texBuf.push_back((float)col/(float)(colParam-1));
			texBuf.push_back(-(float)row/(float)(rowParam-1));
			
			// Solid of revolution (SOR) stuff:
			x = 10 * (float)col / (float)(colParam-1);
			theta = 2 * M_PI * (float)row / (float)(rowParam-1);

			sorPosBuf.push_back(x);
			sorPosBuf.push_back(theta);
			sorPosBuf.push_back(0.0f);

			sorNorBuf.push_back(0.0f);
			sorNorBuf.push_back(0.0f);
			sorNorBuf.push_back(0.0f); 
		}
	}

	// Make the triangles for the sphere and SOR:
	for (int row = 0; row < rowParam - 1; row++){
		for (int col = 0; col < colParam - 1; col++){
			
			int k = rowParam * row + col;

			indBuf.push_back(k);
			indBuf.push_back(k + colParam + 1);
			indBuf.push_back(k + 1);

			indBuf.push_back(k + colParam + 1);
			indBuf.push_back(k);
			indBuf.push_back(k + colParam);
		}
	}
		
	// Generate buffer IDs and put them in the bufIDs map.
	GLuint tmp[4];
	glGenBuffers(4, tmp);
	ballBufIDs["bPos"] = tmp[0];
	ballBufIDs["bNor"] = tmp[1];
	ballBufIDs["bTex"] = tmp[2];
	ballBufIDs["bInd"] = tmp[3];
	glBindBuffer(GL_ARRAY_BUFFER, ballBufIDs["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, ballPosBuf.size()*sizeof(float), &ballPosBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ballBufIDs["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, ballNorBuf.size()*sizeof(float), &ballNorBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, ballBufIDs["bTex"]);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballBufIDs["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(ballNorBuf.size() == ballPosBuf.size());


	// Generate buffer IDs and put them in the bufIDs map.
	GLuint tmp2[4];
	glGenBuffers(4, tmp2);
	sorBufIDs["bPos"] = tmp2[0];
	sorBufIDs["bNor"] = tmp2[1];
	sorBufIDs["bTex"] = tmp2[2];
	sorBufIDs["bInd"] = tmp2[3];
	glBindBuffer(GL_ARRAY_BUFFER, sorBufIDs["bPos"]);
	glBufferData(GL_ARRAY_BUFFER, sorPosBuf.size()*sizeof(float), &sorPosBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, sorBufIDs["bNor"]);
	glBufferData(GL_ARRAY_BUFFER, sorNorBuf.size()*sizeof(float), &sorNorBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, sorBufIDs["bTex"]);
	glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sorBufIDs["bInd"]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBuf.size()*sizeof(unsigned int), &indBuf[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(sorNorBuf.size() == sorPosBuf.size());
	
	GLSL::checkError(GET_FILE_LINE);

	initObjects();
	initLights();

	// Set up the offscreen framebuffer for the first pass by generating and binding the framebuffer
	// From this point on, until we call glBindFramebuffer(GL_FRAMEBUFFER, 0), OpenGL will do stuff on this offscreen buffer
	glGenFramebuffers(1, &framebufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	generateTextures();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	GLSL::checkError(GET_FILE_LINE);
}

// This function is called every frame to draw the scene.
static void render()
{
	double t = glfwGetTime();
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	glActiveTexture(GL_TEXTURE0);

	if(keyToggles[(unsigned)'c']) {
		glEnable(GL_CULL_FACE);
	} else {
		glDisable(GL_CULL_FACE);
	}
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);

	if (textureWidth != width || textureHeight != height){
		textureWidth = width;
		textureHeight = height;

		generateTextures();
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureA);		
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureB);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureC);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureD);
	
	glViewport(0, 0, textureWidth, textureHeight);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	progPass1->bind(); // pass 1 starts
	//camera->setAspect(1.0f);

	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
		camera->applyViewMatrix(MV);
		progPass1->bind();

		// Pass the light positions:
		// Make a new vector with lights positions in world space:
		vector<glm::vec3> lWorld;
		float radius = 2.0f;
		for (int i = 0; i < lightPos.size(); i++){
			// // Spinning lights:
			lightPos.at(i).x = lightPos.at(i).z = radius;
			lightPos.at(i).y = sin(t + lPosAdd.at(i))/2.0f + 0.75f;
			lightPos.at(i).z *= cos((2.0*t/radius + lPosAdd.at(i)));
			lightPos.at(i).x *= sin((2.0*t/radius + lPosAdd.at(i)));

			// Regular:
			lWorld.push_back(MV->topMatrix() * glm::vec4(lightPos.at(i), 1));
			radius += 1.0f;
		}

		// Draw the lights as spheres:
		glUniform3f(progPass1->getUniform("kd"), 0.0f, 0.0f, 0.0f);
		glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));	

		for (int i = 0; i < lightPos.size(); i++){
			glUniform3f(progPass1->getUniform("ke"), lightCol.at(i).r, lightCol.at(i).g, lightCol.at(i).b);
			MV->pushMatrix();
				MV->translate(lightPos.at(i).x, lightPos.at(i).y, lightPos.at(i).z);
				MV->scale(0.1, 0.1, 0.1);
				glUniformMatrix4fv(progPass1->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
				glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
				sphereMesh->draw(progPass1);
			MV->popMatrix();
		}

		// Draw the floor:
		glUniform3f(progPass1->getUniform("ke"), 0.0f, 0.0f, 0.0f);
		MV->pushMatrix();
			MV->rotate(-1.0 * M_PI_2, 1, 0, 0); // Rotate -pi/2 radians on x axis
			MV->scale(100, 100, 1); // Scale the x and y axis so it can cover the whole plane

			glUniform3f(progPass1->getUniform("kd"), 1.0f, 1.0f, 1.0f);

			glUniformMatrix4fv(progPass1->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			squareMesh->draw(progPass1);
		MV->popMatrix();

		// Draw the objects on top of the ground:
		int i = 0;
		glUniformMatrix4fv(progPass1->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		for (int x = -5; x < 6; x++){
			for (int z = -5; z < 6; z++){
					MV->pushMatrix();
						
						MV->translate(0.0f + 3.0f * x, 0.0f, 0.0f + 3.0f * z);
						glUniform3f(progPass1->getUniform("kd"), objects.at(i)->kd.r, objects.at(i)->kd.g, objects.at(i)->kd.b);
						
						int objType = objects.at(i)->objType;

						if (objType == BUNNY){
							MV->rotate(t + objects.at(i)->rotation, 0, 1, 0);
							// If what we are about to draw is a bunny, we must translate down in y:
							// The min y of the bunny is: 0.333099
							MV->translate(0.0f, -0.333099f, 0.0f);
							glUniform1i(progPass1->getUniform("isSor"), 0);
						}
						else if (objType == TEAPOT){
							// Make the teapot shear
							glm::mat4 S(1.0f);
							S[1][2] = cos(1.5 * t + objects.at(i)->shear);;
							MV->multMatrix(S);
							glUniform1i(progPass1->getUniform("isSor"), 0);
						}else if (objType == BALL){

							float scale = objects.at(i)->scaleMult;
							MV->scale(scale, scale, scale);

							MV->translate(0.0f, 0.5f, 0.0f);
							float A_y = 1.3; // Amplitude for the Y translation
							float A_s = 0.5; // Amplitude for the XZ scale
							float p = 1.7; // the period
							float t_0 = 0.9; // the phase

							float timeMult = objects.at(i)->timeMult;
							t_0 = timeMult;

							float y = A_y * (0.5f * sin(2 * M_PI * (t * 0.5 + t_0) / p) + 0.5f);
							float s = -1.0 * A_s * (0.5f * cos(4 * M_PI * (t * 0.5 + t_0) / p) + 0.5f) + 1.0f;
							
							MV->translate(0, y, 0);
							MV->scale(s, 1, s);

							glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("itMV"), 1, GL_FALSE, value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
							glUniform1i(progPass1->getUniform("isSor"), 0);
							drawModel(progPass1, ballBufIDs, ballPosBuf, ballNorBuf, texBuf, indBuf);
						}else{

							MV->scale(0.25f, 0.25f, 0.25f);
							MV->rotate(M_PI_2, 0, 0, 1);

							glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
							glUniformMatrix4fv(progPass1->getUniform("itMV"), 1, GL_FALSE, value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
							glUniform1i(progPass1->getUniform("isSor"), 1);
							glUniform1f(progPass1->getUniform("t"), t * objects.at(i)->timeMult);
							drawModel(progPass1, sorBufIDs, sorPosBuf, sorNorBuf, texBuf, indBuf);
						}

						if (objType < BALL){
							glUniformMatrix4fv(progPass1->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
							glUniformMatrix4fv(progPass1->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
							objects.at(i)->obj->draw(progPass1);
						}
					MV->popMatrix();
				i++;
			}
		}

		progPass1->unbind();
	
	MV->popMatrix();

	P->popMatrix();

	GLSL::checkError(GET_FILE_LINE);

	// Render to the screen:

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	progPass2->bind();
	
	P->pushMatrix();
		MV->pushMatrix();
			glUniform2f(progPass2->getUniform("windowSize"), width, height);

			MV->scale(width, height, 1);

			glUniformMatrix4fv(progPass2->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));
			glUniformMatrix4fv(progPass2->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));

			glUniform3fv(progPass2->getUniform("lPosBuf"), lightPos.size(), value_ptr(lWorld[0]));
			glUniform3fv(progPass2->getUniform("lColBuf"), lightCol.size(), value_ptr(lightCol[0]));
			glUniform3f(progPass2->getUniform("ks"), 1.0f, 1.0f, 1.0f);
			glUniform1f(progPass2->getUniform("s"), 10);

			squareMesh->draw(progPass2);

			glActiveTexture(GL_TEXTURE0);
			progPass2->unbind();
		MV->popMatrix();
	P->popMatrix();
	
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
		cout << "Usage: A3 RESOURCE_DIR" << endl;
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
