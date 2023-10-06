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
#include "Light.h"

using namespace std;

GLFWwindow *window; // Main application window
string RESOURCE_DIR = "./"; // Where the resources are loaded from
bool OFFLINE = false;

shared_ptr<Camera> camera;
shared_ptr<Program> prog;
shared_ptr<Shape> shape;

shared_ptr<Shape> teapot;

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
	sp->addUniform("lightPos1");
	sp->addUniform("lightCol1");

	sp->addUniform("lightPos2");
	sp->addUniform("lightCol2");

	sp->addUniform("ka");
	sp->addUniform("kd");
	sp->addUniform("ks");
	sp->addUniform("s");

	sp->setVerbose(false);

	shaderVec.push_back(sp);
}

void initShaders(){
	vector<string> attributeVec = {};
	vector<string> uniformVec = {"MV", "P"};

	addShader("normal_vert.glsl", "normal_frag.glsl");
	addShader("normal_vert.glsl", "blinn-phong.glsl");
	addShader("normal_vert.glsl", "silhouette.glsl");
	addShader("normal_vert.glsl", "cel-shader.glsl");
}

shared_ptr<Program> getCurrShader(){
	return shaderVec[shaderIndex];
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

	// Material 2: blue with green highlights
	Material m2;
	m2.ka = Vec3(0, 0, 1);
	m2.kd = Vec3(0.15, 0.15, 0.15); // 
	m2.ks = Vec3(0.0, 0.9, 0.0); // this causes the green highlight
	m2.s  = 200;
	materialVec.push_back(m2);

	// // Material 3: grayish with low shininess;
    Material m3;
    m3.ka = Vec3(0.01, 0.01, 0.1);
    m3.kd = Vec3(0.4, 0.4, 0.5);
    m3.ks = Vec3(0.3, 0.3, 0.3);
    m3.s  = 2;
    materialVec.push_back(m3);

	// Set the current material to be m1;
	currMaterial = materialVec.at(0);
}

/*------------------------ LIGHT SELECTION CODE ------------------------*/

vector<Light> lightVec;
int lightIndex = 0;

void initLights(){
	Light l1(1.0, 1.0, 1.0, 0.8, 0.8, 0.8);
	Light l2(-1.0, 1.0, 1.0, 0.2, 0.2, 0.0);

	lightVec.push_back(l1);
	lightVec.push_back(l2);
}

/*------------------------ NEW STUFF DONE ------------------------*/

static void char_callback(GLFWwindow *window, unsigned int key)
{
	keyToggles[key] = !keyToggles[key];

	// The s key will toggle through shaders.
	if (tolower(key) == 's'){
		if (key == 's'){
			shaderIndex = (shaderIndex + 1) % shaderVec.size();
		}else{
			if (!shaderIndex){
				shaderIndex = shaderVec.size() - 1;
			}else{
				shaderIndex--;
			}
		}

		prog = shaderVec[shaderIndex];
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

	// The l key will toggle through the different lights
	else if (tolower(key) == 'l'){
		if (key == 'l'){
			lightIndex = (lightIndex + 1) % lightVec.size();
		}else{
			if (!lightIndex){ 
				lightIndex = lightVec.size() - 1; 
			}else { 
				lightIndex--; 
			}
		}
	}

	else if (tolower(key) == 'x'){
		if (key == 'x'){
			lightVec.at(lightIndex).lightPos.x--;
		}else{
			lightVec.at(lightIndex).lightPos.x++;				
		}
	}

	else if (tolower(key) == 'y'){
		if (key == 'y'){
			lightVec.at(lightIndex).lightPos.y--;
		}else{
			lightVec.at(lightIndex).lightPos.y++;				
		}
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
	
	camera = make_shared<Camera>();
	camera->setInitDistance(2.0f); // Camera's initial Z translation
	
	shape = make_shared<Shape>();
	shape->loadMesh(RESOURCE_DIR + "bunny.obj");
	shape->init();

	teapot = make_shared<Shape>();
	teapot->loadMesh(RESOURCE_DIR + "teapot.obj");
	teapot->init();
	
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
	if(keyToggles[(unsigned)'z']) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	camera->setAspect((float)width/(float)height);
	
	double t = glfwGetTime();
	if(!keyToggles[(unsigned)' ']) {
		// Spacebar turns animation on/off
		t = 0.0f;
	}
	
	// Matrix stacks
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();

	// Apply camera transforms
	P->pushMatrix();
	camera->applyProjectionMatrix(P);
	MV->pushMatrix();
		camera->applyViewMatrix(MV);

		prog->bind();
	
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, glm::value_ptr(P->topMatrix()));	

		// Pass the material and light information:
		glUniform3f(prog->getUniform("lightPos1"), lightVec.at(0).x(), lightVec.at(0).y(), lightVec.at(0).z());
		glUniform3f(prog->getUniform("lightCol1"), lightVec.at(0).r(), lightVec.at(0).g(), lightVec.at(0).b());

		glUniform3f(prog->getUniform("lightPos2"), lightVec.at(1).x(), lightVec.at(1).y(), lightVec.at(1).z());
		glUniform3f(prog->getUniform("lightCol2"), lightVec.at(1).r(), lightVec.at(1).g(), lightVec.at(1).b());
		
		glUniform3f(prog->getUniform("ka"), currMaterial.ka.x, currMaterial.ka.y, currMaterial.ka.z);
		glUniform3f(prog->getUniform("kd"), currMaterial.kd.x, currMaterial.kd.y, currMaterial.kd.z);
		glUniform3f(prog->getUniform("ks"), currMaterial.ks.x, currMaterial.ks.y, currMaterial.ks.z);
		glUniform1f(prog->getUniform("s"), currMaterial.s);
		

		MV->pushMatrix();
			MV->translate(0.0f, -0.5f, 0.0f);
			MV->translate(-0.5, 0.0f, 0.0f);
			MV->scale(0.5f, 0.5f, 0.5f);
			MV->rotate(t, 0.0f, 1.0f, 0.0f);
			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			shape->draw(prog);

		MV->popMatrix();

		MV->pushMatrix();
			MV->translate(0.5f, 0.0f, 0.0f);
			MV->scale(0.5f, 0.5f, 0.5f);

			// Make the teapot shear
			glm::mat4 S(1.0f);
			S[0][1] = 0.5f * cos(t);
			MV->multMatrix(S);

			MV->rotate(M_PI, 0, -1, 0);
			glUniformMatrix4fv(prog->getUniform("itMV"), 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(MV->topMatrix()))));
			glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, glm::value_ptr(MV->topMatrix()));
			teapot->draw(prog);
		MV->popMatrix();


		prog->unbind();
	
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
