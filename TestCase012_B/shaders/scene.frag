#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;
in vec3 v2f_Normal;
in vec3 v2f_FragPosInWorldSpace;
in vec4 v2f_FragPosInLightSpace;

struct SModelMaterial
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Normal;
	sampler2D Ambient;
	float Shininess;
};

uniform sampler2D u_ShadowMapTex;
uniform SModelMaterial u_ModelMaterial;
uniform vec3 u_LightPos;
uniform vec3 u_LightColor;
uniform vec3 u_CameraPos;
uniform float u_LightIntensity;

float ShadowCalculation(vec4 vFragPosInLightSpace)
{
	vec3 ProjectionCoords = vFragPosInLightSpace.xyz / vFragPosInLightSpace.w; //Í¸ÊÓ³ý·¨
	ProjectionCoords = ProjectionCoords * 0.5 + 0.5;
	float ClosestDepth = texture(u_ShadowMapTex, ProjectionCoords.xy).r;
	float CurrentDepth = ProjectionCoords.z;
	float Shadow = CurrentDepth - 0.00001f > ClosestDepth ? 1.0f : 0.0f;
	if(CurrentDepth > 1.0f)
		Shadow = 0.0f;
	return Shadow;
}

void main()
{
	vec3 AlbedoColor = vec3(1.0f);//texture(u_ModelMaterial.Diffuse, v2f_TextureCoord).rgb;
	vec3 Normal = normalize(v2f_Normal);
	vec3 LightDir = normalize(u_LightPos - v2f_FragPosInWorldSpace);
	vec3 DiffuseColor = u_LightColor * max(dot(LightDir, Normal), 0.0f);

	vec3 ViewDir = normalize(u_CameraPos - v2f_FragPosInWorldSpace);
	vec3 HalfVec = normalize(LightDir + ViewDir);
	vec3 SpecularColor = u_LightColor * pow(max(dot(HalfVec, Normal), 0.0f), 64.0f);

	float Shadow = ShadowCalculation(v2f_FragPosInLightSpace);
	vec3 ResultColor = (1.0f - Shadow) * (DiffuseColor + SpecularColor) * AlbedoColor * u_LightIntensity / (dot(u_LightPos - v2f_FragPosInWorldSpace, u_LightPos - v2f_FragPosInWorldSpace) + 0.0001f);

	gl_FragColor = vec4(ResultColor, 1.0f);
	//gl_FragColor = vec4(Shadow, Shadow, Shadow, 1.0f);
	//gl_FragColor = vec4(1, 1, 0, 1);
}