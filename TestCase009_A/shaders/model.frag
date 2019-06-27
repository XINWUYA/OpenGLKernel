#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;
in vec3 v2f_Normal;
in vec3 v2f_FragPos;

struct SModelMaterial
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Normal;
	sampler2D Metallic;
	sampler2D Roughness;
	float Shininess;
};

struct SLightInfo
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

struct SBRDFParameterInfo
{
	vec3 ViewDir;
	vec3 LightDir;
	vec3 Normal;
	vec3 F0;
	vec3 AlbedoColor;
	float Roughness;
	float Metallic;
};

uniform SModelMaterial u_ModelMaterial;
uniform SLightInfo u_LightInfo;
uniform samplerCube u_IrradianceMap;
uniform vec3 u_CameraPos;
uniform float u_Roughness;
uniform float u_Metallic;

const float PI = 3.141592653f;

vec3 FresnelSchlickRoughness(float vCosTheta, vec3 vF0, float vRoughness)
{
    return vF0 + (max(vec3(1.0 - vRoughness), vF0) - vF0) * pow(1.0 - vCosTheta, 5.0);
}

vec3 FresnelSchlick(float vCosTheta, vec3 vF0)
{
	return vF0 + (1.0 - vF0) * pow(1.0 - vCosTheta, 5.0f);
}

float DistributionGGX(vec3 vNormal, vec3 vHalfVec, float vRoughness)
{
	float a = vRoughness * vRoughness;
	float a2 = a * a;
	float NormalDotHalfVec = max(dot(vNormal, vHalfVec), 0.0f);
	float NormalDotHalfVec2 = NormalDotHalfVec * NormalDotHalfVec;

	float Nom = a2;
	float Denom = NormalDotHalfVec2 * (a2 - 1.0f) + 1.0f;
	Denom = PI * Denom * Denom;
	return Nom / Denom;
}

float GeometrySchlickGGX(float vNormalDotViewDir, float vRoughness)
{
	float r = (vRoughness + 1.0f);
	float k = r * r / 8.0f;

	float Nom = vNormalDotViewDir;
	float Denom = vNormalDotViewDir * (1.0f - k) + k;
	return Nom / Denom;
}

float GeometrySmith(vec3 vNormal, vec3 vViewDir, vec3 vLightDir, float vRoughness)
{
	float NormalDotViewDir = max(dot(vNormal, vViewDir), 0.0f);
	float NormalDotLightDir = max(dot(vNormal, vLightDir), 0.0f);

	return GeometrySchlickGGX(NormalDotViewDir, vRoughness) * GeometrySchlickGGX(NormalDotLightDir, vRoughness);
}

vec3 CorrectNormal(vec3 vNormal, vec3 vViewDir)
{
	if(dot(vNormal, vViewDir) < 0.0f)
		vNormal = normalize(vNormal - 1.01 * vViewDir * dot(vNormal, vViewDir));
	return vNormal;
}

vec3 CalculateBRDF(SBRDFParameterInfo vBRDFParameterInfo)
{
	vec3 HalfVec = normalize(vBRDFParameterInfo.ViewDir + vBRDFParameterInfo.LightDir);
	vec3 F0 = mix(vBRDFParameterInfo.F0, vBRDFParameterInfo.AlbedoColor, vBRDFParameterInfo.Metallic);
	vec3 F = FresnelSchlick(max(dot(HalfVec, vBRDFParameterInfo.ViewDir), 0.0f), F0);
	float D = DistributionGGX(vBRDFParameterInfo.Normal, HalfVec, vBRDFParameterInfo.Roughness);
	float G = GeometrySmith(vBRDFParameterInfo.Normal, vBRDFParameterInfo.ViewDir, vBRDFParameterInfo.LightDir, vBRDFParameterInfo.Roughness);
	vec3 SpecularItem = (D * G * F) / (4.0f * max(dot(vBRDFParameterInfo.Normal, vBRDFParameterInfo.ViewDir), 0.0f) * max(dot(vBRDFParameterInfo.Normal, vBRDFParameterInfo.LightDir), 0.0f) + 0.001f);

	vec3 Ks = F;
	vec3 Kd = vec3(1.0f) - Ks;
	Kd *= 1.0f - vBRDFParameterInfo.Metallic;
	vec3 DiffuseItem = Kd * vBRDFParameterInfo.AlbedoColor / PI;

	return (DiffuseItem + SpecularItem) * max(dot(vBRDFParameterInfo.Normal, vBRDFParameterInfo.LightDir), 0.0f);
}

void main()
{
	vec3 OriginalColor = texture(u_ModelMaterial.Diffuse, v2f_TextureCoord).rgb;
	OriginalColor = pow(OriginalColor, vec3(2.2f));
	OriginalColor = vec3(1.0f, 0.0f, 0.0f);

//	float AmbientWeight = 0.03f;
//	vec3 AmbientColor = AmbientWeight * OriginalColor;

	vec3 Normal = normalize(v2f_Normal);

	SBRDFParameterInfo BRDFParameterInfo;
	BRDFParameterInfo.ViewDir = normalize(u_CameraPos - v2f_FragPos);
	BRDFParameterInfo.LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
	BRDFParameterInfo.Normal = CorrectNormal(Normal, BRDFParameterInfo.ViewDir);
	BRDFParameterInfo.F0 = vec3(0.04f);
	BRDFParameterInfo.AlbedoColor = OriginalColor;
	BRDFParameterInfo.Roughness = u_Roughness;
	BRDFParameterInfo.Metallic = u_Metallic;

	float Distance = length(u_LightInfo.Position - v2f_FragPos);
	vec3 ResultColor = u_LightInfo.Intensity * CalculateBRDF(BRDFParameterInfo) * u_LightInfo.Color / (Distance * Distance);

	vec3 Ks = FresnelSchlickRoughness(max(dot(BRDFParameterInfo.Normal, BRDFParameterInfo.ViewDir), 0.0f), BRDFParameterInfo.F0, BRDFParameterInfo.Roughness);
	vec3 Kd = vec3(1.0f) - Ks;
	//Kd *= 1.0f - BRDFParameterInfo.Metallic;
	vec3 AmbientColor = Kd * OriginalColor * texture(u_IrradianceMap, BRDFParameterInfo.Normal).rgb;
	ResultColor += AmbientColor;

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + 1.0f);
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));
	//ResultColor = vec3(1);
	gl_FragColor = vec4(ResultColor, 1.0f);
}