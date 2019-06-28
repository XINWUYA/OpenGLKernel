#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TextureCoord;

out vec2 v2f_TextureCoord;

void main()
{
	gl_Position =  vec4(Pos, 1.0f);
	//gl_PointSize = 10;
	v2f_TextureCoord = TextureCoord;
}