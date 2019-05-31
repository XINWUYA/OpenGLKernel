#version 400 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;

struct SModelMaterial
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Normal;
	sampler2D Ambient;
	float Shininess;
};

uniform SModelMaterial u_ModelMaterial;
void main()
{
	vec4 ResultColor = texture(u_ModelMaterial.Diffuse, v2f_TextureCoord);
	gl_FragColor = ResultColor;
}