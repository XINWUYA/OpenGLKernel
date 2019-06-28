#version 430 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

out vec3 g2f_WorldPos;

uniform mat4 u_ProjectionMat;
uniform mat4 u_ViewMat[6];

void main()
{
	for(int i = 0; i < 6; ++i)
	{
		gl_Layer = i;
		for(int k = 0; k < 3; ++k)
		{
			g2f_WorldPos = gl_in[k].gl_Position.xyz;
			gl_Position = u_ProjectionMat * u_ViewMat[i] * gl_in[k].gl_Position;
			EmitVertex();
		}
		EndPrimitive();
	}
}