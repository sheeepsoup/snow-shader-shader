#define GLEW_STATIC
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<SDL.h>
#include<SDL_image.h>
#include <algorithm> // for std::find_if
#include"Shader.h"
#include"Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"Material.h"
#include"LightDirectional.h"
#include"LightPoint.h"
#include"LightSpot.h"
#include"Mesh.h"
#include"Model.h"
#include"Music.h"
//是否显示光标
bool hindCurse = false;
#pragma region 定义
//摄像机
Camera camera(glm::vec3(0, 0, 3.0f), glm::radians(15.0f), glm::radians(180.0f), glm::vec3(0, 1.0f, 0));
glm::mat4 viewMat;
//事件
// 用于记录按键的前一状态
bool keyLeftPressedLastFrame = false;
bool keyRightPressedLastFrame = false;
bool keyNPressedLastFrame = false;
bool keyBPressedLastFrame = false;
bool keyESCPressedLastFrame = false;
SDL_Event event;


/* 加载纹理的函数
路径--------颜色模式-----------通道------
<路径><GL_RGB || GL_RGBA><GL_TEXTURE0~15>

*/
GLuint loadTexture(const char* path, GLint Mode, GLenum Num) {
	// 使用SDL_image加载图像
	SDL_Surface* surface = IMG_Load(path);
	if (!surface) {
		std::cerr << "Failed to load image: " << IMG_GetError() << std::endl;
		return 0;
	}

	// 生成OpenGL纹理对象
	GLuint textureID;
	glGenTextures(1, &textureID);
	//设置通道,以便多个
	glActiveTexture(Num);
	glBindTexture(GL_TEXTURE_2D, textureID);

	// 设置纹理参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 上传图像数据
	glTexImage2D(GL_TEXTURE_2D, 0, Mode, surface->w, surface->h, 0, Mode, GL_UNSIGNED_BYTE, surface->pixels);

	// 生成mipmap
	glGenerateMipmap(GL_TEXTURE_2D);
	// 释放SDL_Surface数据
	SDL_FreeSurface(surface);

	return textureID;
}
#undef main

float lastX;
float lastY;

//当鼠标移动的时候执行这个
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (!hindCurse) {
		float deltaX;
		float deltaY;
		deltaX = xpos - lastX;
		deltaY = ypos - lastY;


		lastX = xpos;
		lastY = ypos;

		camera.ProessMouseMovement(deltaX, deltaY);
	}
}

#pragma endregion
// 在main函数顶部添加
float deltaTimew = 0.0f;
float lastFrame = 0.0f;
#pragma region 事件
void Run_Event(GLFWwindow* win) {
	float moveSpeed = -10.0f;
	float objSpeed = 0.01f;





	if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS && !keyESCPressedLastFrame) {
		if (hindCurse) {
			hindCurse = false;
			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			hindCurse = true;

			glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
	if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) {
		camera.SpeedX = moveSpeed;
	}
	if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) {
		camera.SpeedX = -moveSpeed;
	}
	if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) {
		camera.SpeedZ = moveSpeed;
	}
	if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) {
		camera.SpeedZ = -moveSpeed;
	}
	if (glfwGetKey(win, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.SpeedY = -moveSpeed;
	}
	if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.SpeedY = moveSpeed;
	}




	// 更新按键的前一状态

	keyLeftPressedLastFrame = (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
	keyRightPressedLastFrame = (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
	keyESCPressedLastFrame = (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS);


}

#pragma endregion
#pragma region 帧率内容


//帧率-------------------

// 初始化帧率计数器
void initFPSCounter(Uint64& startTime, Uint64& lastTimeChecked, int& frameCount) {
	startTime = SDL_GetPerformanceCounter();
	lastTimeChecked = startTime;
	frameCount = 0;
}

// 更新帧率计数器并获取当前的FPS
float updateFPSCounter(Uint64& startTime, Uint64& lastTimeChecked, int& frameCount) {
	Uint64 currentTime = SDL_GetPerformanceCounter();
	frameCount++;

	// 检查是否已过一秒（或更长时间），以计算FPS
	if (currentTime - lastTimeChecked >= SDL_GetPerformanceFrequency()) {
		float fps = frameCount * (SDL_GetPerformanceFrequency() / (float)(currentTime - lastTimeChecked));
		// 重置计数器
		lastTimeChecked = currentTime;
		frameCount = 0;
		return fps; // 返回计算出的FPS
	}

	// 如果没有过一秒，则返回-1表示不需要更新FPS显示
	return -1.0f;
}
float deltaTime;
float lastFPS;

#pragma endregion
// 帧缓冲平面
float quadVertices[] = {
	// 位置 (NDC)      // 纹理坐标
	-1.0f,  1.0f, 0.0f,  0.0f, 1.0f,  // 左上
	-1.0f, -1.0f, 0.0f,  0.0f, 0.0f,  // 左下
	 1.0f,  1.0f, 0.0f,  1.0f, 1.0f,  // 右上
	 1.0f, -1.0f, 0.0f,  1.0f, 0.0f   // 右下
};

// 绕Y轴旋转矩阵
glm::mat3 RotateY(float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return glm::mat3(
		c, 0.0f, s,
		0.0f, 1.0f, 0.0f,
		-s, 0.0f, c
	);
}

// 绕X轴旋转矩阵
glm::mat3 RotateX(float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return glm::mat3(
		1.0f, 0.0f, 0.0f,
		0.0f, c, -s,
		0.0f, s, c
	);
}

// 绕Z轴旋转矩阵
glm::mat3 RotateZ(float angle) {
	float s = sin(angle);
	float c = cos(angle);
	return glm::mat3(
		c, -s, 0.0f,
		s, c, 0.0f,
		0.0f, 0.0f, 1.0f
	);
}

// 组合旋转矩阵（Y→X→Z顺序，与你的GLSL一致）
glm::mat3 CalculateRotationMatrix(float angle) {
	return RotateY(angle) * RotateX(angle) * RotateZ(angle);
}


int main(int argc, const char* argv[]) {//argv[0]指运行路径


#pragma region 初始化
#pragma region 正常不用改的
#pragma region VAO,VBO

	//glfw初始化
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* win = glfwCreateWindow(800, 600, "a", NULL, NULL);
	glfwMakeContextCurrent(win);
	//隐藏指针
	glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//当鼠标移动的时候执行第二个函数
	glfwSetCursorPosCallback(win, mouse_callback);

	glewExperimental = GL_TRUE;
	glewInit();
	glViewport(0, 0, 800, 600);


#pragma endregion



#pragma region 纹理初始化
	//纹理

	unsigned int snowText = loadTexture("bin/snowGround.png", GL_RGBA, GL_TEXTURE4);
	unsigned int fbmText = loadTexture("bin/snowGround.png", GL_RGBA, GL_TEXTURE1);

#pragma endregion

#pragma region 帧率初始化
	//帧率相关
	Uint64 startTime = 0;
	Uint64 lastTimeChecked = 0;
	int frameCount = 0;

	initFPSCounter(startTime, lastTimeChecked, frameCount);


#pragma endregion 

#pragma endregion








#pragma endregion
	//LoadSound();//初始化音乐


	//创建屏幕VAO,VBO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


	


	//创建球体的shader
	Shader* qiuShader = new Shader("snowMVert.vert", "snowMFrag.frag");

	float angle = 0.01 * 0.5f; // 旋转速度控制
	glm::mat3 rotation = CalculateRotationMatrix(angle);
	float waveTime = 0.0f;
	float waveSpeed = 0.5f; // 控制摇摆速度
	float maxWaveAngle = 0.1f; // 最大摇摆角度


	//播放音乐
	//useSound("rain");
	//useSound("sea");
	while (!glfwWindowShouldClose(win)) {
	


		// 计算时间差
		float currentFrame = glfwGetTime();
		deltaTimew = currentFrame - lastFrame;
		lastFrame = currentFrame;

#pragma region 帧率输出
		// 更新帧率计数器并尝试获取FPS
		float fps = updateFPSCounter(startTime, lastTimeChecked, frameCount);
		if (fps != -1.0f) {

			system("cls");

			std::cout << "FPS: " << fps << std::endl;
			std::cout << "x:" << camera.Position.x << " y:" << camera.Position.y << " z:" << camera.Position.z << std::endl;

			lastFPS = fps;
		}




#pragma endregion



		Run_Event(win);


#pragma region 绘画海洋
		glDisable(GL_DEPTH_TEST);
		//随着时间稍微变一下角度
		// 在渲染循环中更新waveTime
		waveTime += deltaTimew * waveSpeed;

		// 计算摇摆角度 - 使用sin函数实现周期性摆动
		float currentWaveAngle = maxWaveAngle * sin(waveTime);
		glm::mat3 rotation = CalculateRotationMatrix(currentWaveAngle);

		qiuShader->use();
		glBindVertexArray(quadVAO);

		glActiveTexture(GL_TEXTURE0);

		// 上传到Shader
		glUniformMatrix3fv(glGetUniformLocation(qiuShader->ID, "rot"), 1, GL_FALSE, &rotation[0][0]);

		glUniform3f(glGetUniformLocation(qiuShader->ID, "cameraPos"), camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1f(glGetUniformLocation(qiuShader->ID, "iTime"), static_cast<float>(glfwGetTime()));
		glUniform3f(glGetUniformLocation(qiuShader->ID, "Up"), camera.Up.x, camera.Up.y, camera.Up.z);
		glUniform3f(glGetUniformLocation(qiuShader->ID, "Right"), camera.Right.x, camera.Right.y, camera.Right.z);
		glUniform3f(glGetUniformLocation(qiuShader->ID, "Forward"), camera.Foword.x, camera.Foword.y, camera.Foword.z);

		
		glBindTexture(GL_TEXTURE_2D, snowText);
		glUniform1i(glGetUniformLocation(qiuShader->ID, "snowDentText"), 4);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	




#pragma endregion




		glfwPollEvents();
		glfwSwapBuffers(win);
		camera.UpdateCameraPos(deltaTimew);
	}

}