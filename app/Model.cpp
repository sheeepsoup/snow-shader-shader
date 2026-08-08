#include "Model.h"
#include<iostream>
#include<SDL.h>
#include<SDL_image.h>

Model::Model(std::string path, GLint LoadModew)
{
	loadModle(path);
	LoadMode = LoadModew;

}

void Model::Draw(Shader* shader)
{

	for (unsigned int i = 0; i < meshes.size(); i++) {
		meshes[i].Draw(shader);
	}
}

void Model::loadModle(std::string path)
{

	//模型数据
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	//检测内容
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}
	else {
		//	std::cout << "success" << std::endl;
	}

	//返回上一级文件夹
//	directory = path.substr(0, path.find_last_of('\\'));

	// 修正目录路径获取方式
	size_t lastSlash = path.find_last_of("/\\");
	directory = (lastSlash != std::string::npos) ?
		path.substr(0, lastSlash) :
		"";
	// 统一替换为Unix风格路径分隔符
	std::replace(directory.begin(), directory.end(), '\\', '/');
	//SDL_Delay(88888);
	std::cout << directory;
	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	//std::cout << node->mName.data << std::endl;
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* curMesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(curMesh, scene));
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}
// 修复后的纹理加载函数
GLuint loadTexturea(const char* path, GLint Mode, GLenum Num) {
	SDL_Surface* surface = IMG_Load(path);
	if (!surface) {
		std::cerr << "IMG_Load failed: " << IMG_GetError() << std::endl;
		return 0;
	}

	// 转换为RGBA格式
	SDL_Surface* converted = SDL_ConvertSurfaceFormat(
		surface,
		SDL_PIXELFORMAT_RGBA32,
		0
	);
	SDL_FreeSurface(surface);

	// 创建OpenGL纹理
	GLuint textureID;
	glGenTextures(1, &textureID);
	glActiveTexture(Num);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 设置内存对齐
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// 上传纹理
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RGBA,  // 内部格式
		converted->w,
		converted->h,
		0,
		GL_RGBA,  // 数据格式
		GL_UNSIGNED_BYTE,
		converted->pixels
	);

	glGenerateMipmap(GL_TEXTURE_2D);
	SDL_FreeSurface(converted);

	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}
// 修改后的loadMaterialTextures
std::vector<Texture> Model::loadMaterialTextures(
	aiMaterial* mat,
	aiTextureType type,
	std::string typeName
) {
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);

		// 统一路径格式
		std::string texturePath = str.C_Str();
		std::replace(texturePath.begin(), texturePath.end(), '\\', '/');

		bool skip = false;
		for (auto& loaded : textures_loaded) {
			if (loaded.path == texturePath) { // 比较统一后的路径
				textures.push_back(loaded);
				skip = true;
				break;
			}
		}
		if (!skip) {
			std::string fullPath = directory + "/" + texturePath;
			Texture texture;
			texture.id = loadTexturea(fullPath.c_str(), LoadMode, GL_TEXTURE0);
			texture.type = typeName;
			texture.path = texturePath; // 保存统一后的路径

			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}


Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// data to fill
	std::vector<Vertex> tempVertices;
	std::vector<unsigned int> tempIndices;
	std::vector<Texture> tempTextures;

	glm::vec3 tempVec;
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex tempvertex;
		tempvertex.Position.x = mesh->mVertices[i].x;
		tempvertex.Position.y = mesh->mVertices[i].y;
		tempvertex.Position.z = mesh->mVertices[i].z;

		tempvertex.Normal.x = mesh->mNormals[i].x;
		tempvertex.Normal.y = mesh->mNormals[i].y;
		tempvertex.Normal.z = mesh->mNormals[i].z;

		// 纹理坐标（带Y轴翻转）
		if (mesh->mTextureCoords[0]) {
			tempvertex.TexCoords = glm::vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			);
		}
		else {
			tempvertex.TexCoords = glm::vec2(0.0f);
		}

		tempVertices.push_back(tempvertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {

		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
		{
			tempIndices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	// 处理材质
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

	// 1. 漫反射贴图
	std::vector<Texture> diffuseMaps = loadMaterialTextures(
		material,
		aiTextureType_DIFFUSE,
		"texture_diffuse"
	);
	tempTextures.insert(tempTextures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 在processMesh函数中添加：
// 2. 镜面反射贴图
	std::vector<Texture> specularMaps = loadMaterialTextures(
		material,
		aiTextureType_SPECULAR,
		"texture_specular"
	);
	tempTextures.insert(tempTextures.end(), specularMaps.begin(), specularMaps.end());

	// 返回包含纹理的Mesh
	return Mesh(tempVertices, tempIndices, tempTextures); // 传递纹理数据

}
