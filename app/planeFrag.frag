#version 330 core

out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

// 添加下雨效果


void main() {
	

vec4 color = texture(screenTexture,TexCoord);
	 FragColor = vec4(color);

//	 FragColor = vec4(0,0,0,1);
//	FragColor = vec4(color.a);
	// if((FragColor.r == 0) && (FragColor.g == 0) && (FragColor.b == 0))FragColor.a = 0.0f;
}


