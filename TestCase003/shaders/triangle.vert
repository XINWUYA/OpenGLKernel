#version 400 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 TextureCoord;

out vec2 v2f_TextureCoord;

void main()
{
	gl_Position = vec4(Pos, 1.0f);
	v2f_TextureCoord = TextureCoord;
}