#pragma once
#include"Shader.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Material
{
public:
	Shader* shader;
	//纹理通道
	unsigned int diffuse;
	//环境光
	glm::vec3 ambient;
	//镜面反射纹理通道
	unsigned int specular;
	//照耀程度
	float shininess;



	Material(Shader* _shader, unsigned int _diffuse, glm::vec3 _ambient, unsigned int _specular, float _shininess);




};

