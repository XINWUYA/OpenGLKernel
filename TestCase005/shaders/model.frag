#version 400 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;

struct SModelMaterial
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Ambient;
	float Shininess;
};

uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AmbientTexture;
uniform SModelMaterial ModelMaterial;
void main()
{
	//vec2 uv = vec2(float(gl_FragCoord.x) / 800.0, float(gl_FragCoord.y)/600.0);
	//gl_FragColor = texture(u_Texture, v2f_TextureCoord);
	vec4 ResultColor = (texture(u_DiffuseTexture, v2f_TextureCoord) * texture(u_SpecularTexture, v2f_TextureCoord) * texture(u_NormalTexture, v2f_TextureCoord) * texture(u_AmbientTexture, v2f_TextureCoord));
	gl_FragColor = ResultColor * vec4(1, 0, 0, 1);
}