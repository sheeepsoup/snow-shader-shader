#include "LightDirectional.h"

LightDirectional::LightDirectional(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color) :
	position(_postiton),
	angle(_angles),
	color(_color)
{

}


void LightDirectional::UpdateDirection() {
	//初始化角度
	direction = glm::vec3(0, 0, 1.0f);
	//角度更新
	direction = glm::rotateZ(direction, angle.z);
	direction = glm::rotateX(direction, angle.x);
	direction = glm::rotateY(direction, angle.y);

	//翻一下,变成指向光
	direction = -direction;
}