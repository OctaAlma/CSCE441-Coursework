#define _USE_MATH_DEFINES
#include <cmath> 
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include "FreeCam.h"
#include "MatrixStack.h"

FreeCam::FreeCam() :
	yaw(0),
	pitch(0),
	fovy((float)(45.0*M_PI/180.0)),
	aspect(1.0f),
	znear(0.1f),
	zfar(1000.0f),
	rotations(0.0, 0.0),
	translations(0.0f, 0.0f, -5.0f),
	rfactor(0.01f),
	tfactor(0.001f),
	sfactor(0.005f)
{
	position = glm::vec3(0.5,0.5,0.5);
}

FreeCam::~FreeCam()
{
}

void FreeCam::mouseClicked(float x, float y, bool shift, bool ctrl, bool alt)
{
	mousePrev.x = x;
	mousePrev.y = y;
	if(shift) {
		state = FreeCam::TRANSLATE;
	} else if(ctrl) {
		state = FreeCam::SCALE;
	} else {
		state = FreeCam::ROTATE;
	}
}

#define DEGREES60 1.04719755

void FreeCam::mouseMoved(float x, float y)
{
	glm::vec2 mouseCurr(x, y);
	glm::vec2 dv = mouseCurr - mousePrev;

	yaw -= dv.x * 0.005;

	pitch -= dv.y * 0.005;
	if (pitch > DEGREES60){
		pitch = DEGREES60;
	}else if (pitch < -DEGREES60){
		pitch = -DEGREES60;
	}

	switch(state) {
		case FreeCam::ROTATE:
			rotations += rfactor * dv;
			break;
		case FreeCam::TRANSLATE:
			translations.x -= translations.z * tfactor * dv.x;
			translations.y += translations.z * tfactor * dv.y;
			break;
		case FreeCam::SCALE:
			translations.z *= (1.0f - sfactor * dv.y);
			break;
	}
	mousePrev = mouseCurr;
}

void FreeCam::applyProjectionMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::perspective(fovy, aspect, znear, zfar));
}

void FreeCam::applyOrthoMatrix(std::shared_ptr<MatrixStack> P) const
{
	// Modify provided MatrixStack
	P->multMatrix(glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, znear, zfar));
}

void FreeCam::applyViewMatrix(std::shared_ptr<MatrixStack> MV) const
{
	// Add yaw to the freelook camera (right and left)
    glm::vec3 forwardDir = glm::vec3(sin(yaw), sin(pitch), cos(yaw));

	glm::mat4 View = glm::lookAt(position, position + forwardDir, glm::vec3(0, 1, 0));

    //MV->translate(position.x, position.y, position.z);
	MV->multMatrix(View);
}

void FreeCam::move(char key){
	glm::vec3 forwardDir = glm::vec3(sin(yaw), 0, cos(yaw));

	// The "right" direction is forward X up directions
	glm::vec3 rightDir = glm::cross(forwardDir, glm::vec3(0,1,0));

	if (key == 'w'){
		position = position + (forwardDir * 0.1f);
	}else if (key == 's'){
		position = position - (forwardDir * 0.1f);		
	}else if (key == 'a'){
		position = position - (rightDir * 0.1f);
	}else if (key == 'd'){
		position = position + (rightDir * 0.1f);
	}

}
