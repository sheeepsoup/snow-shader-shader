#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<string>
#include<GL/glew.h>
#include<vector>
#include"Shader.h"
#include <vector>
#include<iostream>
//顶点组
struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
	glm::vec3 Color; // 添加颜色字段
};
#pragma pack(push, 1) // 禁用填充，确保结构体紧凑
struct Triangle {
	glm::vec3 v0; float pad0 = 0.0;
	glm::vec3 v1; float pad1 = 0.0;
	glm::vec3 v2; float pad2 = 0.0;
	glm::vec3 normal; float pad3 = 0.0;
	glm::vec3 Special; float pad4 = 0.0;
	glm::vec2 uv0; glm::vec2 pad5 = { 0.0, 0.0 };  // v0的纹理坐标
	glm::vec2 uv1; glm::vec2 pad6 = { 0.0, 0.0 };  // v1的纹理坐标
	glm::vec2 uv2; glm::vec2 pad7 = { 0.0, 0.0 };  // v2的纹理坐标
	int textureID = -1;  // 使用的纹理ID，-1表示无纹理
	float pad8 = 0.0, pad9 = 0.0, pad10 = 0.0;
};

#pragma pack(pop)

//纹理
struct Texture
{
	unsigned int id;
	std::string type;
	std::string path;
};
class Mesh
{
public:
	//网格数据
	std::vector<Vertex> vertices;
	std::vector<Texture> textures;
	std::vector<unsigned int> indices;
	//函数
	Mesh(float vertices[]);
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
	void Draw(Shader* shader);
	unsigned int VAO;//分离出来用于实例化
	// 新增方法：获取所有三角形数据（世界空间）
	std::vector<Triangle> GetTriangles(const glm::mat4& modelMatrix) const {
		std::vector<Triangle> triangles;

		for (size_t i = 0; i < indices.size(); i += 3) {
			Triangle tri;
			// 顶点位置
			tri.v0 = modelMatrix * glm::vec4(vertices[indices[i]].Position, 1.0f);
			tri.v1 = modelMatrix * glm::vec4(vertices[indices[i + 1]].Position, 1.0f);
			tri.v2 = modelMatrix * glm::vec4(vertices[indices[i + 2]].Position, 1.0f);

			// 纹理坐标
			tri.uv0 = vertices[indices[i]].TexCoords;
			tri.uv1 = vertices[indices[i + 1]].TexCoords;
			tri.uv2 = vertices[indices[i + 2]].TexCoords;

			// 计算面法线
			glm::vec3 edge1 = tri.v1 - tri.v0;
			glm::vec3 edge2 = tri.v2 - tri.v0;
			tri.normal = glm::normalize(glm::cross(edge1, edge2));

			// 如果有纹理，设置纹理ID
			if (!textures.empty()) {
				tri.textureID = textures[0].id; // 确保这里设置的是实际的纹理ID

			}

			triangles.push_back(tri);
		}
		return triangles;
	}
private:
	//渲染数据
	unsigned int VBO, EBO;
	//函数
	void setupMesh();
};

