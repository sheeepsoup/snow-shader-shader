#define GLM_ENABLE_EXPERIMENTAL
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>

class LightSpot
{
public:
	LightSpot(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color = glm::vec3(1.0f, 1.0f, 1.0f));
	//点/聚光用
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 angle;
	glm::vec3 direction = glm::vec3(0, 0, 1.0f);
	void UpdateDirction();

	//聚光灯半径   若要调整半径,下面这俩在[0,1]的范围内增大即可
	float CosPhyInner = 0.9f;
	//直直的内部[朝向的]  平滑用
	float CosPhyOutter = 0.85f;
	//衰减部分
	//常量
	float constant;
	//一次衰减值
	float linear;
	//二次衰减值
	float quadratic;
};

