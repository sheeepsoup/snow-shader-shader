#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include<glm/gtx/rotate_vector.hpp>

class LightDirectional
{
public:
	LightDirectional(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f));

	//µ„/æ€π‚”√
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 angle;
	glm::vec3 direction = glm::vec3(glm::radians(45.0f), glm::radians(45.0f), 1.0f);

	void UpdateDirection();

};

