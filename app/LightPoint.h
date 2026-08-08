#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class LightPoint
{

public:
	LightPoint(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f));

	//点/聚光用
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 angle;
	glm::vec3 direction = glm::vec3(0, 0, 1.0f);



	//衰减部分
	//常量
	float constant;
	//一次衰减值
	float linear;
	//二次衰减值
	float quadratic;
};

