#version 430 core

out vec4 gl_FragColor;

in vec3 v2f_WorldPos;

uniform samplerCube u_CubeMapTexture;

void main()
{
	vec3 ResultColor = texture(u_CubeMapTexture, v2f_WorldPos).rgb;
	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + 1.0f);
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));

	gl_FragColor = vec4(ResultColor, 1.0f);
}