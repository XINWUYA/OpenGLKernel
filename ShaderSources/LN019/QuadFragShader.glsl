#version 430 core

out vec4 gl_FragColor;

in vec2 oTexCoord;

uniform sampler2D uTexture;

void main()
{
	vec4 FragColor = texture(uTexture, oTexCoord);

	gl_FragColor = FragColor;//ԭͼ

	//gl_FragColor = vec4(1,0,0, 1.0f);
	//gl_FragColor = texture(uTexture, oTexCoord);
}