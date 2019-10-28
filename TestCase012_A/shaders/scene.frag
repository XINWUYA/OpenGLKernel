#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;

uniform sampler2D u_Texture;

void main()
{
	float DepthValue = texture(u_Texture, v2f_TextureCoord).r;
	gl_FragColor = vec4(vec3(DepthValue), 1);
	//gl_FragColor = vec4(1, 0, 0, 1);
}