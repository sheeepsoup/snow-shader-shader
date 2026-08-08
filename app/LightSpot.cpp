#include "LightSpot.h"



LightSpot::LightSpot(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color) :
	position(_postiton),
	angle(_angles),
	color(_color)
{
	//刚开始初始化一下角度
	UpdateDirction();
	//初始衰减度
	constant = 1.0f;
	linear = 0.3f;//修改衰减量条这里,越高衰减越多
	quadratic = 0.032f;
}

void LightSpot::UpdateDirction()
{
	//初始化角度
	direction = glm::vec3(0, 0, 1.0f);
	//角度更新
	direction = glm::rotateZ(direction, angle.z);
	direction = glm::rotateX(direction, angle.x);
	direction = glm::rotateY(direction, angle.y);

	//翻一下,变成指向光
	direction = -direction;


}
