#pragma once
//#ifndef CAMERA_H
//#define CAMERA_H
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
class Camera
{
public:
	Camera(glm::vec3 position, glm::vec3 traget, glm::vec3 worldup);
	Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup);
	glm::vec3 Position;
	glm::vec3 Foword;
	glm::vec3 Right;
	glm::vec3 Up;
	glm::vec3 WorldUp;

	glm::mat4 GetViewMatrix();

	float Pitch;
	float Yaw;
	void ProessMouseMovement(float DealtX, float DealtY);
	void UpdateCameraPos(float deltaTime);
	float SpeedZ = 0;

	float SpeedX = 0;
	float SpeedY = 0;
private:

	void UpdateCameraVectors();
};

//#endif CAMERA_H
