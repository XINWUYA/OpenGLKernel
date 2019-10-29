#version 430 core

out vec4 gl_FragColor;

uniform vec3 u_LightColor;
uniform float u_LightIntensity;

void main()
{
	gl_FragColor = vec4(u_LightIntensity * u_LightColor, 1.0f);
}