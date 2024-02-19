#pragma once
#include <vec3.hpp>
#include <vector>

using namespace glm;

class Camera
{
private:
	vec3 origin;
	vec3 forward;
	vec3 right;
	vec3 up;

	float fov, aspectRatio;
	vec3 orbitPivot;
public:
	Camera(float fov, float aspectRatio);
	void rotateX(float xRot);
	void rotateY(float yRot);
	void rotateZ(float zRot);
	void moveForward(float amount);
	void moveRight(float amount);
	void moveUp(float amount);
	void setFovAspectRatio(float fov, float aspectRatio);
	vec3 getOrigin();
	vec3 getForward();
	vec3 getRight();
	vec3 getUp();
};