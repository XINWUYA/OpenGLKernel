#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;

uniform mat4 u_ProjectionMat;
uniform mat4 u_ViewMat;
uniform mat4 u_ModelMat;

void main()
{
	gl_Position = u_ProjectionMat * u_ViewMat * u_ModelMat * vec4(Pos, 1.0f);
}