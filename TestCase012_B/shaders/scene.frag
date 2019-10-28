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

float ShadowCalculation(vec4 vFragPosInLightSpace, float vBias)
{
	vec3 ProjectionCoords = vFragPosInLightSpace.xyz / vFragPosInLightSpace.w; //Í¸ÊÓ³ý·¨
	ProjectionCoords = ProjectionCoords * 0.5 + 0.5;
	float ClosestDepth = texture(u_ShadowMapTex, ProjectionCoords.xy).r;
	float CurrentDepth = ProjectionCoords.z;
	float Shadow = CurrentDepth - ClosestDepth > 0.00001f ? 1.0f : 0.0f;
	if(CurrentDepth > 1.0f)
		Shadow = 0.0f;
	return Shadow;
	//return ClosestDepth;
}

void main()
{
	vec3 AlbedoColor = texture(u_ModelMaterial.Diffuse, v2f_TextureCoord).rgb;
	vec3 Normal = normalize(v2f_Normal);
	vec3 LightDir = normalize(u_LightPos - v2f_FragPosInWorldSpace);
	vec3 DiffuseColor = u_LightColor * max(dot(LightDir, Normal), 0.0f);

	vec3 ViewDir = normalize(u_CameraPos - v2f_FragPosInWorldSpace);
	vec3 HalfVec = normalize(LightDir + ViewDir);
	vec3 SpecularColor = u_LightColor * pow(max(dot(HalfVec, Normal), 0.0f), 64.0f);

	float Bias = max(0.05f * (1.0f - dot(Normal, LightDir)), 0.005f);
	float Shadow = ShadowCalculation(v2f_FragPosInLightSpace, 0.005);
	vec3 ResultColor = (1.0f - Shadow) * (DiffuseColor + SpecularColor) * AlbedoColor;
	//if(Shadow > 0.0f)
	//	ResultColor = vec3(1,0,0);
	//else
	//	ResultColor = vec3(1);
	gl_FragColor = vec4(ResultColor, 1.0f);
	//gl_FragColor = vec4(Shadow, Shadow, Shadow, 1.0f);
	//gl_FragColor = vec4(1, 1, 0, 1);
}