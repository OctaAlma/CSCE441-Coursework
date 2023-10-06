#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Other libraries I required
#include <glm/gtc/type_ptr.hpp>
#include <queue>

#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

using namespace std;

GLFWwindow *window; // Main application window
string RES_DIR = ""; // Where data files live
shared_ptr<Program> prog;
shared_ptr<Program> progIM; // immediate mode

shared_ptr<Shape> shape;
shared_ptr<Shape> joint;

// Draws a 0.5x-scaled sphere.obj on the provided matrix stack.
void drawJoint(shared_ptr<MatrixStack> MV){
	MV->pushMatrix();
		MV->scale(0.5, 0.5, 0.5);
		glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
        joint->draw(prog);
	MV->popMatrix();
}

/** TASK 3: Create a class that represents a component **/
struct Component{

	// Represents the translation of the component’s joint with respect to the parent component’s joint.
	glm::vec3 translationPtoJ;

	// Represents the current joint angles about the X, Y, and Z axes of the component’s joint. 
	glm::vec3 jointAngles;

	// Represents the translation of the component’s mesh with respect to its joint.
	glm::vec3 translationJtoM;

	// Represents the X, Y, and Z scaling factors for the mesh.
	glm::vec3 scalingFactor;
	double scaleModifier = 1;

	// Rotation-in-place:
	glm::vec3 rotateInPlace;
	double radiansToRotate = 0;
	double radianIncrease = 0;

	// Used for breadth-first search:
	int currChild = -1;

	vector<shared_ptr<Component>> children;
	shared_ptr<Component> parent;

	string name;

	Component(){
		translationPtoJ = glm::vec3(0,0,0);
		translationJtoM = glm::vec3(0,0,0);

		jointAngles = glm::vec3(0,0,0);
		scalingFactor = glm::vec3(1,1,1);

		rotateInPlace = glm::vec3(0, 0, 0);
	}

	void draw(shared_ptr<MatrixStack> MV){
		MV->pushMatrix();

			MV->translate(translationPtoJ); 
			
			// Rotation about each axis:
			MV->rotate(jointAngles.x, 1, 0, 0); // Rotation amongst the x axis
			MV->rotate(jointAngles.y, 0, 1, 0); // Rotation amongst the y axis
			MV->rotate(jointAngles.z, 0, 0, 1); // Rotation amongst the z axis
			
			// Draw the specific component:
			MV->pushMatrix();
				// Task 6 Rotate in place:
				if (radianIncrease != 0){
					MV->rotate(radiansToRotate, rotateInPlace);
					radiansToRotate = (radiansToRotate + radianIncrease);
				}

				// Task 5: draw the joint:
				drawJoint(MV);

				MV->translate(translationJtoM); 

				MV->scale(scalingFactor.x * scaleModifier, scalingFactor.y * scaleModifier, scalingFactor.z * scaleModifier);

				glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, value_ptr(MV->topMatrix()));
				shape->draw(prog);

			MV->popMatrix();

			// Draw the component's children:
			for (int i = 0; i < children.size(); i++){
				children.at(i)->draw(MV);
			}

		MV->popMatrix();
	}

	void addChild(shared_ptr<Component> child){
		children.push_back(child);
	}
};

shared_ptr<Component> InitializeParts(){
	shared_ptr<Component> torso, head, upperLeftArm, lowerLeftArm, upperRightArm, lowerRightArm;
	shared_ptr<Component> upperLeftLeg, lowerLeftLeg, upperRightLeg, lowerRightLeg;

	// Make the torso:
	torso = make_shared<Component>();
	torso->translationPtoJ = glm::vec3(0,0,0); // The torso's joint is at 0,0,0
	torso->translationJtoM = glm::vec3(0,0,0); // The torso's mesh is at 0,0,0
	torso->scalingFactor = glm::vec3(1.5, 2, 1);
	torso->jointAngles = glm::vec3(0,0,0);

	torso->name = "torso";

	torso->parent = torso;


	// Make the head:
	head = make_shared<Component>();
	head->translationPtoJ = glm::vec3(0, 1, 0); // IN PARENT JOINT COORDS / ORIGIN
	head->translationJtoM = glm::vec3(0, 0.5*0.75, 0); // IN COMPONENT JOINT COORDS
	head->scalingFactor = glm::vec3(0.75, 0.75, 0.75);
	head->jointAngles = glm::vec3(0,0,0);
	
	head->name = "head";


	// Make the upperLeftArm:
    upperLeftArm = make_shared<Component>();
    upperLeftArm->translationPtoJ = glm::vec3(-0.75, 0.75, 0); // IN PARENT JOINT COORDS / ORIGIN
    upperLeftArm->translationJtoM = glm::vec3(-0.75, 0, 0); // IN COMPONENT JOINT COORDS
    upperLeftArm->scalingFactor = glm::vec3(1.5, 0.5, 0.5);
    upperLeftArm->jointAngles = glm::vec3(0, 0, 0);

	upperLeftArm->name = "upLA";

	// Task 6 Non-leaf node:
	upperLeftArm->rotateInPlace = glm::vec3(1,0,0);
	upperLeftArm->radianIncrease = 0.03;


	// Make the lowerLeftArm:
	lowerLeftArm = make_shared<Component>();
    lowerLeftArm->translationPtoJ = glm::vec3(-1.5, 0, 0); // IN PARENT JOINT COORDS / ORIGIN
    lowerLeftArm->translationJtoM = glm::vec3(-0.5, 0, 0); // IN COMPONENT JOINT COORDS
	lowerLeftArm->scalingFactor = glm::vec3(1, 0.25, 0.25);
    lowerLeftArm->jointAngles = glm::vec3(0, 0, 0);
	
	lowerLeftArm->parent = upperLeftArm;
	upperLeftArm->addChild(lowerLeftArm);
	lowerLeftArm->name = "loLA";


	// Make the upperRightArm:
    upperRightArm = make_shared<Component>();
    upperRightArm->translationPtoJ = glm::vec3(0.75, 0.75, 0); // IN PARENT JOINT COORDS / ORIGIN
    upperRightArm->translationJtoM = glm::vec3(0.75, 0, 0); // IN COMPONENT JOINT COORDS
    upperRightArm->scalingFactor = glm::vec3(1.5, 0.5, 0.5);
    upperRightArm->jointAngles = glm::vec3(0, 0, 0);
	
	upperRightArm->name = "upRA";
	

	// Make the lowerRightArm:
	lowerRightArm = make_shared<Component>();
    lowerRightArm->translationPtoJ = glm::vec3(1.5, 0, 0); // IN PARENT JOINT COORDS / ORIGIN
    lowerRightArm->translationJtoM = glm::vec3(0.5, 0, 0); // IN COMPONENT JOINT COORDS
	lowerRightArm->scalingFactor = glm::vec3(1, 0.25, 0.25);
    lowerRightArm->jointAngles = glm::vec3(0, 0, 0);

	lowerRightArm->parent = upperRightArm;
	upperRightArm->addChild(lowerRightArm);
	lowerRightArm->name = "loRA";

	// Task 6 leaf node:
	lowerRightArm->rotateInPlace = glm::vec3(1,0,0);
	lowerRightArm->radianIncrease = 0.05;


	// Make the UpperLeftLeg
    upperLeftLeg = make_shared<Component>();
    upperLeftLeg->translationPtoJ = glm::vec3(-0.4, -1, 0); // IN PARENT JOINT COORDS / ORIGIN
    upperLeftLeg->translationJtoM = glm::vec3(0, -0.75, 0); // IN COMPONENT JOINT COORDS
    upperLeftLeg->scalingFactor = glm::vec3(0.5, 1.5, 0.5);
    upperLeftLeg->jointAngles = glm::vec3(0, 0, 0);
	upperLeftLeg->parent = torso;

	upperLeftLeg->name = "upLL";


	// Make the lowerLeftLeg
	lowerLeftLeg = make_shared<Component>();
    lowerLeftLeg->translationPtoJ = glm::vec3(0, -1.5, 0); // IN PARENT JOINT COORDS / ORIGIN
    lowerLeftLeg->translationJtoM = glm::vec3(0, -0.5, 0); // IN COMPONENT JOINT COORDS
    lowerLeftLeg->scalingFactor = glm::vec3(0.25, 1, 0.25);
    lowerLeftLeg->jointAngles = glm::vec3(0, 0, 0);

	lowerLeftLeg->parent = upperLeftLeg;
	upperLeftLeg->addChild(lowerLeftLeg);
	lowerLeftLeg->name = "loLL";

	// Make the UpperRightLeg
    upperRightLeg = make_shared<Component>();
    upperRightLeg->translationPtoJ = glm::vec3(0.4, -1, 0); // IN PARENT JOINT COORDS / ORIGIN
    upperRightLeg->translationJtoM = glm::vec3(0, -0.75, 0); // IN COMPONENT JOINT COORDS
    upperRightLeg->scalingFactor = glm::vec3(0.5, 1.5, 0.5);
    upperRightLeg->jointAngles = glm::vec3(0, 0, 0);

	upperRightLeg->name = "upRL";


	// Make the lowerRightLeg
	lowerRightLeg = make_shared<Component>();
    lowerRightLeg->translationPtoJ = glm::vec3(0, -1.5, 0); // IN PARENT JOINT COORDS / ORIGIN
    lowerRightLeg->translationJtoM = glm::vec3(0, -0.5, 0); // IN COMPONENT JOINT COORDS
    lowerRightLeg->scalingFactor = glm::vec3(0.25, 1, 0.25);
    lowerRightLeg->jointAngles = glm::vec3(0, 0, 0);

	lowerRightLeg->parent = upperRightLeg;
	upperRightLeg->addChild(lowerRightLeg);
	lowerRightLeg->name = "loRL";

	// Make upper body parts children of the torso:
	torso->addChild(head);
	torso->addChild(upperLeftArm);
	torso->addChild(upperRightArm);
	torso->addChild(upperLeftLeg);
	torso->addChild(upperRightLeg);

	head->parent = torso;
	upperLeftArm->parent = torso;
	upperRightArm->parent = torso;
	upperLeftLeg->parent = torso;
	upperRightLeg->parent = torso;


	return torso;
}

/** TASK 4 **/
shared_ptr<Component> torso = nullptr;
shared_ptr<Component> currSelected = nullptr;

// Breadth-first search for the , and . keys requires a queue-type structure
vector<shared_ptr<Component>> q;
long qIndex = 0;

// We populate the queue array with the appropriate order for a breadth-first traversal:
void populateQueue(shared_ptr<Component> curr){
	if (curr == torso){
		q.push_back(torso);
	}

	for (int i = 0; i < curr->children.size(); i++){
		q.push_back(curr->children.at(i));
	}

	if (curr == torso){

		for (int i = 1; i < q.size(); i++){
			populateQueue(q.at(i));
		}
	}
}

// Print-statement code for debugging:
void printQueue(){
	for (int i = 0; i < q.size(); i++){
		cout << q.at(i)->name << ", ";
	}
	cout << endl;
}

/** NEW CODE FINISHED **/

static void error_callback(int error, const char *description)
{
	cerr << description << endl;
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

static void char_callback(GLFWwindow *window, unsigned int key)
{
    switch(key) {
        case 'x':
        {
            // Do something
			currSelected->jointAngles.x += 0.1;
            break;
        }

		case 'y':
        {
            // Do something
			currSelected->jointAngles.y += 0.1;
            break;
        }

		case 'z':
        {
            // Do something
			currSelected->jointAngles.z += 0.1;
            break;
        }

		case 'X':
        {
            // Do something
			currSelected->jointAngles.x -= 0.1;
            break;
        }

		case 'Y':
        {
            // Do something
			currSelected->jointAngles.y -= 0.1;
            break;
        }

		case 'Z':
        {
            // Do something
			currSelected->jointAngles.z -= 0.1;
            break;
        }

		case '.':
		{
			qIndex = (qIndex + 1) % q.size();;
			currSelected->scaleModifier = 1;
			currSelected = q.at(qIndex);
			break;
		}

		case ',':
		{
			if (qIndex != 0){
				qIndex--;
			}else{
				qIndex = q.size()-1;
			}
			currSelected->scaleModifier = 1;
			currSelected = q.at(qIndex);
			break;
		}

    }
}

static void init()
{
	GLSL::checkVersion();

	// Check how many texture units are supported in the vertex shader
	int tmp;
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tmp);
	cout << "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS = " << tmp << endl;
	// Check how many uniforms are supported in the vertex shader
	glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &tmp);
	cout << "GL_MAX_VERTEX_UNIFORM_COMPONENTS = " << tmp << endl;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &tmp);
	cout << "GL_MAX_VERTEX_ATTRIBS = " << tmp << endl;

	// Set background color.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Enable z-buffer test.
	glEnable(GL_DEPTH_TEST);

	/** NEW CODE **/

	// Initialize the robot structure:
	torso = InitializeParts();
	populateQueue(torso);
	currSelected = q.at(qIndex);
	currSelected = torso;

	/** END OF NEW CODE **/

	// Initialize mesh.
	shape = make_shared<Shape>();
	shape->loadMesh(RES_DIR + "cube.obj");
	shape->init();

	joint = make_shared<Shape>();
	joint->loadMesh(RES_DIR + "sphere.obj");
	joint->init();
	
	// Initialize the GLSL programs.
	prog = make_shared<Program>();
	prog->setVerbose(true);
	prog->setShaderNames(RES_DIR + "nor_vert.glsl", RES_DIR + "nor_frag.glsl");
	prog->init();
	prog->addUniform("P");
	prog->addUniform("MV");
	prog->addAttribute("aPos");
	prog->addAttribute("aNor");
	prog->setVerbose(false);
	
	progIM = make_shared<Program>();
	progIM->setVerbose(true);
	progIM->setShaderNames(RES_DIR + "simple_vert.glsl", RES_DIR + "simple_frag.glsl");
	progIM->init();
	progIM->addUniform("P");
	progIM->addUniform("MV");
	progIM->setVerbose(false);
	
	// If there were any OpenGL errors, this will print something.
	// You can intersperse this line in your code to find the exact location
	// of your OpenGL error.
	GLSL::checkError(GET_FILE_LINE);
}

static void render()
{
	// Get current frame buffer size.
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	float aspect = width/(float)height;
	glViewport(0, 0, width, height);

	// Clear framebuffer.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Create matrix stacks.
	auto P = make_shared<MatrixStack>();
	auto MV = make_shared<MatrixStack>();
	// Apply projection.
	P->pushMatrix();
	P->multMatrix(glm::perspective((float)(45.0*M_PI/180.0), aspect, 0.01f, 100.0f));

	double t = glfwGetTime();
	double a = 0.05;
	double f = 2;
	double s_t = 1 + a/2 + (a/2) * (sin(2 * 3.141592 * f * t));
	
	// Apply camera transform.
	MV->pushMatrix();
		MV->translate(glm::vec3(0, 0, -10));
		//MV->rotate(1.55, 0, 1, 0);

		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(P->topMatrix()));

		currSelected->scaleModifier = s_t;
		torso->draw(MV);
	
	// Pop matrix stacks.
	MV->popMatrix();
	P->popMatrix();
	
	GLSL::checkError(GET_FILE_LINE);
}

int main(int argc, char **argv)
{
	if(argc < 2) {
		cout << "Please specify the resource directory." << endl;
		return 0;
	}
	RES_DIR = argv[1] + string("/");

	// Set error callback.
	glfwSetErrorCallback(error_callback);
	// Initialize the library.
	if(!glfwInit()) {
		return -1;
	}
	// https://en.wikipedia.org/wiki/OpenGL
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	// glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// Create a windowed mode window and its OpenGL context.
	window = glfwCreateWindow(640, 480, "YOUR NAME", NULL, NULL);
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
	// Set vsync.
	glfwSwapInterval(1);
	// Set keyboard callback.
	glfwSetKeyCallback(window, key_callback);
	glfwSetCharCallback(window, char_callback); // ADDED THIS
	// Initialize scene.
	init();
	// Loop until the user closes the window.
	while(!glfwWindowShouldClose(window)) {
		if(!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
			// Render scene.
			render();

			// Swap front and back buffers.
			glfwSwapBuffers(window);
		}
		// Poll for and process events.
		glfwPollEvents();
	}
	// Quit program.
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
