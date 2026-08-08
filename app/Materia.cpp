#include "Material.h"

#include<GL/glew.h>

Material::Material(Shader* _shader, unsigned int _diffuse, glm::vec3 _ambient, unsigned int _specular, float _shininess) :
	shader(_shader),
	diffuse(_diffuse),
	ambient(_ambient),
	specular(_specular),
	shininess(_shininess)//这里表示初始值 和 shiness = _shiness一样
{

}


