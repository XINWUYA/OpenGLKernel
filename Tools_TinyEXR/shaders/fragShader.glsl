#version 400 core

out vec4 gl_FragColor;

in vec2 oTexCoord;
in vec3 oPos;
uniform sampler2D u_Texture;

void main()
{
	vec2 UV = vec2(1.0f) - oTexCoord;
	vec3 ResultColor = texture(u_Texture, UV).rgb;
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));
	gl_FragColor = vec4(ResultColor, 1.0f);

	//gl_FragColor = vec4(1, 0, 0, 1);
}