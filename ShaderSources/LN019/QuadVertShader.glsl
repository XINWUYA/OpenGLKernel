#version 430 core

layout(location = 0) in vec2 Pos;
layout(location = 1) in vec2 Tex;

out vec2 oTexCoord;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
	gl_Position = uProjection * uView * uModel * vec4(Pos, 0.0f, 1.0f);
	oTexCoord   = Tex;
}