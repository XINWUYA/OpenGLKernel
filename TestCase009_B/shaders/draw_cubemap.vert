#version 430 core

layout(location = 0) in vec3 Pos;

uniform mat4 u_ProjectionMat;
uniform mat4 u_ViewMat;
uniform mat4 u_ModelMat;

out vec3 v2f_WorldPos;

void main()
{

	vec4 Position = u_ProjectionMat * mat4(mat3(u_ViewMat)) * u_ModelMat * vec4(Pos, 1.0f);
	gl_Position = Position.xyww;

	v2f_WorldPos = Pos;
}