#version 430 core

layout(location = 0) in vec3 Pos;

uniform mat4 u_ProjectionMat;
uniform mat4 u_ViewMat;
uniform mat4 u_ModelMat;

//out vec3 v2f_VertexPos;

void main()
{
	gl_Position = u_ProjectionMat * u_ViewMat * u_ModelMat * vec4(Pos, 1.0f);
	//v2f_VertexPos = Pos;
}