#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 v2f_TextureCoord;
out vec3 v2f_Normal;
out vec3 v2f_FragPos;

void main()
{
	gl_Position = projection * view * model * vec4(Pos, 1.0f);
	//gl_PointSize = 10;
	v2f_TextureCoord = TextureCoord;
	v2f_Normal = mat3(transpose(inverse(model))) * Normal;
	v2f_FragPos = vec3(model * vec4(Pos, 1.0f));
}