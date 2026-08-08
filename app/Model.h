#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#pragma once
#include"Mesh.h"
#include<string>
#include<GL/glew.h>
#include<vector>
#include<assimp/Importer.hpp>
#include<assimp/scene.h>

#include<assimp/postprocess.h>
//路径

class Model
{
public:
	Model(std::string path, GLint LoadModew);

	std::vector<Mesh> meshes;

	std::vector<Texture> loadMaterialTextures(
		aiMaterial* mat,
		aiTextureType type,
		std::string typeName
	);
	void Draw(Shader* shader);
	//加载纹理模式[GL_RGBA]
	GLint LoadMode;
private:
	void loadModle(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	std::string  directory;
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> textures_loaded;
};

