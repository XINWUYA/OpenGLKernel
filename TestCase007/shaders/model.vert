#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;
layout(location = 3) in vec3 Tangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 v2f_TextureCoord;
out vec3 v2f_Normal;
out vec3 v2f_FragPos;
out vec3 v2f_Tangent;

void main()
{
	gl_Position = projection * view * model * vec4(Pos, 1.0f);
	//gl_PointSize = 10;
	v2f_TextureCoord = TextureCoord;
	mat3 NormalMat = mat3(transpose(inverse(model)));
	v2f_Normal =  normalize(NormalMat * Normal);
	v2f_FragPos = vec3(model * vec4(Pos, 1.0f));
	v2f_Tangent = normalize(NormalMat * Tangent);
}