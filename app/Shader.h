#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include<string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class Shader
{
public:
	unsigned int ID;//ShaderProgram的ID
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
	std::string vertexString;
	std::string fragmentString;
	std::string geometryString; // 新增几何着色器代码存储
	const char* vertexSource;
	const char* fragmentSource;
	const char* geometrySource; // 新增几何着色器指针
	void use();

	//发送vec3
		//发送vec3数据
	void SetUniform3f(const char* name, glm::vec3 VEC3);

private:

};

