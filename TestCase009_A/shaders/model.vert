#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;

uniform mat4 u_ProjectionMat;
uniform mat4 u_ViewMat;
uniform mat4 u_ModelMat;

out vec2 v2f_TextureCoord;
out vec3 v2f_Normal;
out vec3 v2f_FragPos;

void main()
{
	gl_Position = u_ProjectionMat * u_ViewMat * u_ModelMat * vec4(Pos, 1.0f);
	//gl_PointSize = 10;
	v2f_TextureCoord = TextureCoord;
	mat3 NormalMat = mat3(transpose(inverse(u_ModelMat)));
	v2f_Normal =  normalize(NormalMat * Normal);
	v2f_FragPos = vec3(u_ModelMat * vec4(Pos, 1.0f));
}