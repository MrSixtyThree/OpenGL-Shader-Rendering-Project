#include "Camera.h"

Camera::Camera()
{
	// initialise member variables with default values
	mPosition = glm::vec3(0.0f, 0.0f, 1.0f);
	mLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	mUp = glm::vec3(0.0f, 1.0f, 0.0f);
	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
	mProjMatrix = glm::mat4(1.0f);
}

Camera::~Camera()
{}

void Camera::update(float moveForward, float moveRight, float moveUp)
{
	// rotate the respective unit vectors about the y-axis
	glm::vec3 rotatedForwardVec = glm::vec3(glm::rotate(mYaw, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
	glm::vec3 rotatedRightVec = glm::vec3(glm::rotate(mYaw, glm::vec3(0.0f, 1.0f, 0.0f))
		* glm::vec4(1.0f, 0.0f, 0.0f, 0.0f));

	// rotate the forward vector about the right vector by the pitch
	rotatedForwardVec = glm::vec3(glm::rotate(mPitch, rotatedRightVec) 
		* glm::vec4(rotatedForwardVec, 0.0f));

	// update camera position, look-at position and up vectors
	mPosition += rotatedForwardVec * moveForward + rotatedRightVec * moveRight;
	mLookAt = mPosition + rotatedForwardVec;
	mUp = glm::cross(rotatedRightVec, rotatedForwardVec);

	// move in direction of camera's up vector
	mPosition += moveUp * mUp;
	mLookAt += moveUp * mUp;

	// compute the new view matrix
	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);
}

void Camera::updateRotation(float deltaYaw, float deltaPitch)
{
	// set pitch limit to avoid Gimbal lock
	static const float pitchLimit = glm::radians(89.5f);

	// update yaw and pitch
	mYaw += deltaYaw;
	mPitch += deltaPitch;

	// keep pitch within limits
	if (mPitch > pitchLimit)
		mPitch = pitchLimit;
	else if (mPitch < -pitchLimit)
		mPitch = -pitchLimit;
}

void Camera::setViewMatrix(glm::vec3 position, glm::vec3 lookAt)
{
	// set member variables
	mPosition = position;
	mLookAt = lookAt;

	// calculate up vector
	glm::vec3 forwardVec = glm::normalize(lookAt - position);
	glm::vec3 rightVec = glm::cross(forwardVec, glm::vec3(0.0f, 1.0f, 0.0f));
	mUp = glm::normalize(glm::cross(rightVec, forwardVec));

	// create view matrix
	mViewMatrix = glm::lookAt(mPosition, mLookAt, mUp);

	// calculate yaw and pitch
	glm::vec3 hLookAtVec = glm::vec3(forwardVec.x, 0.0f, forwardVec.z);
	glm::vec3 vLookAtVec = glm::vec3(0.0f, forwardVec.y, forwardVec.z);
	glm::vec3 negativeZ(0.0f, 0.0f, -1.0f);

	// yaw angle
	mYaw = acos(glm::dot(negativeZ, glm::normalize(hLookAtVec)));
	if (hLookAtVec.x > 0.0f)
		mYaw = -mYaw;

	// pitch angle
	mPitch = acos(glm::dot(negativeZ, glm::normalize(vLookAtVec)));
	if (vLookAtVec.y < 0.0f)
		mPitch = -mPitch;
}

void Camera::setProjMatrix(glm::mat4 projMatrix)
{
	mProjMatrix = projMatrix;
}

glm::mat4 Camera::getViewMatrix()
{
	return mViewMatrix;
}

glm::mat4 Camera::getProjMatrix()
{
	return mProjMatrix;
}

glm::vec3 Camera::getPosition()
{
	return mPosition;
}

glm::vec3 Camera::getDirection()
{
	return glm::normalize(mLookAt - mPosition);
}
