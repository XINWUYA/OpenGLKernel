#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;

uniform sampler2D u_Texture;

void main()
{
	//vec2 uv = vec2(float(gl_FragCoord.x) / 800.0, float(gl_FragCoord.y)/600.0);
	gl_FragColor = vec4(texture(u_Texture, v2f_TextureCoord));
	//gl_FragColor = vec4(1, 0, 0, 1);
}