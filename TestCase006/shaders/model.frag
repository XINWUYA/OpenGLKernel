#version 400 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;
in vec3 v2f_Normal;
in vec3 v2f_FragPos;

struct SModelMaterial
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Normal;
	sampler2D Ambient;
	sampler2D Roughness;
	float Shininess;
};

struct SLightInfo
{
	vec3 Position;
	vec3 Color;
};

uniform SModelMaterial u_ModelMaterial;
uniform SLightInfo u_LightInfo;
uniform vec3 u_CameraPos;

void main()
{
	vec3 OriginalColor = vec3(texture(u_ModelMaterial.Diffuse, v2f_TextureCoord));

	float AmbientWeight = 0.1f;
	vec3 AmbientColor = AmbientWeight * OriginalColor;

	vec3 Normal = normalize(v2f_Normal);
	vec3 LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
	vec3 DiffuseColor = max(dot(LightDir, Normal), 0.0f) * OriginalColor;

	vec3 ViewDir = normalize(u_CameraPos -v2f_FragPos);
	vec3 ReflectDir = reflect(-LightDir, Normal);
	vec3 SpecularColor = pow(max(dot(ViewDir, ReflectDir), 0.0f), 64) * u_LightInfo.Color * OriginalColor;

	vec3 ResultColor = AmbientColor + DiffuseColor + 0.9 * SpecularColor;
	gl_FragColor = vec4(ResultColor, 1.0f);
}