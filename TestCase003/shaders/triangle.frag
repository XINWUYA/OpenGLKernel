#version 400 core

out vec4 gl_FragColor;

uniform sampler2D u_Texture;

void main()
{
	vec2 uv = vec2(float(gl_FragCoord.x) / 800.0, float(gl_FragCoord.y)/600.0);
	gl_FragColor = texture(u_Texture, uv);
	//gl_FragColor = vec4(1, 0, 0, 1);
}