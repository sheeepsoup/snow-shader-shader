#define GLEW_STATIC

#include "Shader.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include<GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	std::ifstream vertexFile;
	std::ifstream fragmentFile;
	std::stringstream vertexSStream;
	std::stringstream fragmentSStream;



	vertexFile.open(vertexPath);
	fragmentFile.open(fragmentPath);
	// 检查顶点着色器文件是否打开成功
	if (!vertexFile.is_open()) {
		throw std::runtime_error("Failed to open vertex shader file: " + std::string(vertexPath));
		system("pause");
	}

	// 检查片段着色器文件是否打开成功
	if (!fragmentFile.is_open()) {
		throw std::runtime_error("Failed to open fragment shader file: " + std::string(fragmentPath));
		system("pause");
	}

	vertexSStream << vertexFile.rdbuf();
	fragmentSStream << fragmentFile.rdbuf();

	vertexString = vertexSStream.str();
	fragmentString = fragmentSStream.str();


	vertexSource = vertexString.c_str();
	fragmentSource = fragmentString.c_str();

	// 2. 新增几何着色器读取逻辑
	if (geometryPath != nullptr) {
		std::ifstream geometryFile;
		std::stringstream geometrySStream;
		geometryFile.open(geometryPath);
		if (!geometryFile.is_open()) {
			throw std::runtime_error("Failed to open geometry shader file: " + std::string(geometryPath));
			system("pause");
		}
		geometrySStream << geometryFile.rdbuf();
		geometryString = geometrySStream.str();
		geometrySource = geometryString.c_str();
	}


	// 3. 编译着色器
	unsigned int vertex, fragment, geometry = 0;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);
	// 编译几何着色器（如果存在）
	if (geometryPath != nullptr) {
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geometrySource, NULL);
		glCompileShader(geometry);

		// 检查几何着色器编译错误
		int success;
		char infoLog[512];
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			std::cout << "几何着色器编译错误: " << infoLog << std::endl;
			system("pause");
		}
	}


	// 4. 创建着色器程序并附加着色器
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr) {
		glAttachShader(ID, geometry);
	}
	glLinkProgram(ID);

	//错误检查
	int success;
	char infoLog[512];
	// 检查顶点着色器
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "顶点着色器:" << vertexPath << std::endl;
		std::cout << "顶点着色器编译错误: " << infoLog << std::endl;
		system("pause");
	}

	// 检查片段着色器
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "片段着色器:" << fragmentPath << std::endl;
		std::cout << "片段着色器编译错误: " << infoLog << std::endl;
		system("pause");
	}
	// 检查geo着色器
	if (geometryPath != nullptr) {
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			std::cout << "几何着色器:" << geometryPath << std::endl;
			std::cout << "几何着色器编译错误: " << infoLog << std::endl;
			system("pause");
		}
	}
	//链接错误检查
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "着色器程序链接错误: " << infoLog << std::endl;
		system("pause");
	}

	// 6. 删除着色器对象（包括几何着色器）
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometry != 0) {
		glDeleteShader(geometry);
	}
}

void Shader::use() {
	glUseProgram(ID);
}


void Shader::SetUniform3f(const char* name, glm::vec3 VEC3)
{
	glUniform3f(glGetUniformLocation(ID, name), VEC3.x, VEC3.y, VEC3.z);
}





















/*
10~11
#define GLEW_STATIC

#include "Shader.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include<GL/glew.h>
Shader::Shader(const char* vertexPath,const char* fragmentPath)
{
	std::ifstream vertexFile;
	std::ifstream fragmentFile;
	std::stringstream vertexSStream;
	std::stringstream fragmentSStream;



	vertexFile.open(vertexPath);
	fragmentFile.open(fragmentPath);
	// 检查顶点着色器文件是否打开成功
	if (!vertexFile.is_open()) {
		throw std::runtime_error("Failed to open vertex shader file: " + std::string(vertexPath));
	}

	// 检查片段着色器文件是否打开成功
	if (!fragmentFile.is_open()) {
		throw std::runtime_error("Failed to open fragment shader file: " + std::string(fragmentPath));
	}

	vertexSStream << vertexFile.rdbuf();
	fragmentSStream << fragmentFile.rdbuf();

	vertexString = vertexSStream.str();
	fragmentString = fragmentSStream.str();


	vertexSource = vertexString.c_str();
	fragmentSource = fragmentString.c_str();

	unsigned int vertex, fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);




	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);


	//错误检查
	int success;
	char infoLog[512];
	// 检查顶点着色器
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "顶点着色器编译错误: " << infoLog << std::endl;
	}

	// 检查片段着色器
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "片段着色器编译错误: " << infoLog << std::endl;
	}

	//链接错误检查
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "着色器程序链接错误: " << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() {
	glUseProgram(ID);
}





*/

/*
13
#define GLEW_STATIC

#include "Shader.h"
#include<fstream>
#include<iostream>
#include<sstream>
#include<GL/glew.h>
Shader::Shader(const char* vertexPath,const char* fragmentPath)
{
	std::ifstream vertexFile;
	std::ifstream fragmentFile;
	std::stringstream vertexSStream;
	std::stringstream fragmentSStream;



	vertexFile.open(vertexPath);
	fragmentFile.open(fragmentPath);
	// 检查顶点着色器文件是否打开成功
	if (!vertexFile.is_open()) {
		throw std::runtime_error("Failed to open vertex shader file: " + std::string(vertexPath));
	}

	// 检查片段着色器文件是否打开成功
	if (!fragmentFile.is_open()) {
		throw std::runtime_error("Failed to open fragment shader file: " + std::string(fragmentPath));
	}

	vertexSStream << vertexFile.rdbuf();
	fragmentSStream << fragmentFile.rdbuf();

	vertexString = vertexSStream.str();
	fragmentString = fragmentSStream.str();


	vertexSource = vertexString.c_str();
	fragmentSource = fragmentString.c_str();

	unsigned int vertex, fragment;

	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSource, NULL);
	glCompileShader(vertex);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSource, NULL);
	glCompileShader(fragment);




	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);


	//错误检查
	int success;
	char infoLog[512];
	// 检查顶点着色器
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "顶点着色器编译错误: " << infoLog << std::endl;
	}

	// 检查片段着色器
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "片段着色器编译错误: " << infoLog << std::endl;
	}

	//链接错误检查
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "着色器程序链接错误: " << infoLog << std::endl;
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() {
	glUseProgram(ID);
}





*/

