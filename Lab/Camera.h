#ifndef CAMERA_H
#define CAMERA_H

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

// simple up-right camera (no roll)
class Camera {
public:
	Camera();
	~Camera();

	void update(float moveForward, float moveRight, float moveUp = 0.0f);
	void updateRotation(float deltaYaw, float deltaPitch);
	void setViewMatrix(glm::vec3 position, glm::vec3 lookAt);
	void setProjMatrix(glm::mat4 projMatrix);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjMatrix();
	glm::vec3 getPosition();
	glm::vec3 getDirection();

private:
	float mYaw = 0.0f;
	float mPitch = 0.0f;
	glm::vec3 mPosition;
	glm::vec3 mLookAt;
	glm::vec3 mUp;
	glm::mat4 mViewMatrix;
	glm::mat4 mProjMatrix;
};

#endif