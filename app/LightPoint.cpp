#include "LightPoint.h"

LightPoint::LightPoint(glm::vec3 _postiton, glm::vec3 _angles, glm::vec3 _color) :
	position(_postiton),
	angle(_angles),
	color(_color)
{
	constant = 1.0f;
	linear = 0.3f;//修改衰减量条这里,越高衰减越多
	quadratic = 0.032f;

}
