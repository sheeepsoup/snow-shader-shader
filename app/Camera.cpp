#include "Camera.h"
#include<iostream>

Camera::Camera(glm::vec3 position, glm::vec3 traget, glm::vec3 worldup) {
	Position = position;
	WorldUp = worldup;

	Foword = glm::normalize(traget - position);

	//垂直法向量
	Right = glm::normalize(glm::cross(Foword, WorldUp));
	Up = glm::normalize(glm::cross(Foword, Right));



}
Camera::Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup) {
	Position = position;
	WorldUp = worldup;
	Pitch = pitch;
	Yaw = yaw;

	Foword.x = glm::cos(pitch) * glm::sin(yaw);
	Foword.y = glm::sin(pitch);
	Foword.z = glm::cos(pitch) * glm::cos(yaw);
	Right = glm::normalize(glm::cross(Foword, WorldUp));
	Up = glm::normalize(glm::cross(Foword, Right));



}
glm::mat4 Camera::GetViewMatrix() {
	return glm::lookAt(Position, Position + Foword, WorldUp);
}
void Camera::UpdateCameraVectors() {
	Foword.x = glm::cos(Pitch) * glm::sin(Yaw);
	Foword.y = glm::sin(Pitch);
	Foword.z = glm::cos(Pitch) * glm::cos(Yaw);
	Right = glm::normalize(glm::cross(Foword, WorldUp));
	Up = glm::normalize(glm::cross(Foword, Right));

}

void Camera::ProessMouseMovement(float DealtX, float DealtY) {
	Pitch -= DealtY * 0.001f;
	Yaw -= DealtX * 0.001f;
	UpdateCameraVectors();
}
float speed = 1;
void Camera::UpdateCameraPos(float deltaTime) {

	Position += Foword * SpeedZ * deltaTime;
	Position += Right * SpeedX * deltaTime;
	Position += WorldUp * SpeedY * deltaTime;
	// 重置速度
	SpeedZ = SpeedX = SpeedY = 0.0f;
}