#version 430 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TextureCoord;
layout(location = 3) in vec3 Tangent;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 u_LightProjectionMat;
uniform mat4 u_LightViewMat;

out vec2 v2f_TextureCoord;
out vec3 v2f_Normal;
out vec3 v2f_Tangent;
out vec3 v2f_FragPosInWorldSpace;
out vec4 v2f_FragPosInLightSpace;

void main()
{
	gl_Position = projection * view * model * vec4(Pos, 1.0f);
	v2f_FragPosInWorldSpace = vec3(model * vec4(Pos, 1.0f));
	mat3 InvModelMat = transpose(inverse(mat3(model)));
	v2f_Normal = normalize(InvModelMat * Normal);
	v2f_Tangent = normalize(InvModelMat * Tangent);
	v2f_TextureCoord = TextureCoord;
	v2f_FragPosInLightSpace = u_LightProjectionMat * u_LightViewMat * vec4(v2f_FragPosInWorldSpace, 1.0f);
}