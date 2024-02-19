#define GLM_ENABLE_EXPERIMENTAL
#include "Camera.h"
#include <vec3.hpp>
#include <glm.hpp>
#include <gtx/rotate_vector.hpp>
#include <gtx/quaternion.hpp>

using namespace glm;

Camera::Camera(float fov, float aspectRatio)
{
	this->fov = fov;
	this->aspectRatio = aspectRatio;

	origin = vec3(0.0, 0.0, -10.0);
	forward = vec3(0.0, 0.0, 1.0);

	float upLength = tan(fov / 2);
	up = vec3(0.0, upLength, 0.0);

	right = vec3(aspectRatio * upLength, 0.0, 0.0);

	orbitPivot = vec3(0.0, 0.0, 0.0);
}

void Camera::rotateX(float xRot)
{
	mat4 rotationMatrix = rotate(mat4(1.0), xRot, normalize(right));
	forward = vec3(rotationMatrix * vec4(forward, 1.0));
	up = vec3(rotationMatrix * vec4(up, 1.0));
}

void Camera::rotateY(float yRot)
{
	mat4 rotationMatrix = rotate(mat4(1.0), yRot, normalize(up));
	forward = vec3(rotationMatrix * vec4(forward, 1.0));
	right = vec3(rotationMatrix * vec4(right, 1.0));
}

void Camera::rotateZ(float zRot)
{
	mat4 rotationMatrix = rotate(mat4(1.0), zRot, normalize(forward));
	forward = vec3(rotationMatrix * vec4(forward, 1.0));
	up = vec3(rotationMatrix * vec4(up, 1.0));
	right = vec3(rotationMatrix * vec4(right, 1.0));
}

void Camera::moveForward(float amount)
{
	origin = origin + forward * amount;
}

void Camera::moveRight(float amount)
{
	origin = origin + right * amount;
}

void Camera::moveUp(float amount)
{
	origin = origin + up * amount;
}

void Camera::setFovAspectRatio(float fov, float aspectRatio)
{
	this -> fov = fov;
	this -> aspectRatio = aspectRatio;

	float upLength = tan(fov / 2);
	up = normalize(up) * upLength;

	float rightLength = aspectRatio * upLength;
	right = normalize(right) * rightLength;
}

vec3 Camera::getOrigin()
{
	return origin;
}
vec3 Camera::getForward()
{
	return forward;
}
vec3 Camera::getRight()
{
	return right;
}
vec3 Camera::getUp()
{
	return up;
}

