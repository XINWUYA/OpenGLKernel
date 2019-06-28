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
uniform samplerCube u_EnvironmentMap;
uniform sampler2D u_BRDFTexture;
uniform vec3 u_CameraPos;
uniform float u_Roughness;
uniform float u_Metallic;
uniform float u_ClearCoatRoughness;
uniform float u_ClearCoatThickness;
uniform float u_Reflectance = 0.5f;

const float PI = 3.141592653f;
const float MAX_CLEAR_COAT_ROUGHNESS = 0.6f;
const float MIN_ROUGHNESS = 0.045f;

//***********************************************************************************************
//Function: 
vec3 ConvertF0ClearCoat2Surface(const vec3 vF0)
{
  // Approximation of iorToF0(f0ToIor(f0), 1.5). Assumes IOR of 1.5 (4% reflectance)
  return clamp(vF0 * (vF0 * (0.941892f - 0.263008f * vF0) + 0.346479f) - 0.0285998f, 0.0f, 1.0f);
}

//***********************************************************************************************
//Function: 
float RemapClearCoatRoughness(float vInputRoughness)
{
	float ResultRoughness = mix(MIN_ROUGHNESS, MAX_CLEAR_COAT_ROUGHNESS, vInputRoughness);//Remapped to 0.0..0.6 matches the fact that clear coat layers are almost always glossy.
	return ResultRoughness;
}

//***********************************************************************************************
//Function:
vec3 CalculateF0WithClearCoat(vec3 vF0, float vClearCoatThickness, float vReflectance, vec3 vAlbedoColor, float vMetallic)
{
	vec3 F0 = mix(vec3(0.16f * vReflectance * vReflectance), vAlbedoColor, vMetallic);
	return mix(F0, ConvertF0ClearCoat2Surface(F0), vClearCoatThickness);
}

//***********************************************************************************************
//Function:
float CalculateRoughnessWithCleaCoat(float vRoughness, float vClearCoatRoughness, float vClearCoatThickness)
{
	float ClearCoatRoughness = RemapClearCoatRoughness(vClearCoatRoughness);
	return mix(vRoughness, max(vRoughness, ClearCoatRoughness), vClearCoatThickness);
}

//***********************************************************************************************
//Function:
vec3 FresnelSchlickRoughness(float vCosTheta, vec3 vF0, float vRoughness)
{
    return vF0 + (max(vec3(1.0f - vRoughness), vF0) - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

//***********************************************************************************************
//Function:
vec3 FresnelSchlick(float vCosTheta, const vec3 vF0)
{
	return vF0 + (1.0f - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

float FresnelSchlick(float vCosTheta, float vF0)
{
	return vF0 + (1.0f - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

//***********************************************************************************************
//Function:
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

//***********************************************************************************************
//Function:
float GeometrySchlickGGX(float vNormalDotViewDir, float vRoughness)
{
	float r = (vRoughness + 1.0f);
	float k = r * r / 8.0f;

	float Nom = vNormalDotViewDir;
	float Denom = vNormalDotViewDir * (1.0f - k) + k;
	return Nom / Denom;
}

//***********************************************************************************************
//Function:
float GeometrySmith(vec3 vNormal, vec3 vViewDir, vec3 vLightDir, float vRoughness)
{
	float NormalDotViewDir = max(dot(vNormal, vViewDir), 0.0f);
	float NormalDotLightDir = max(dot(vNormal, vLightDir), 0.0f);

	return GeometrySchlickGGX(NormalDotViewDir, vRoughness) * GeometrySchlickGGX(NormalDotLightDir, vRoughness);
}

//***********************************************************************************************
//Function:
vec3 CorrectNormal(vec3 vNormal, vec3 vViewDir)
{
	if(dot(vNormal, vViewDir) < 0.0f)
		vNormal = normalize(vNormal - 1.01 * vViewDir * dot(vNormal, vViewDir));
	return vNormal;
}

//***********************************************************************************************
//Function:
vec3 CalculateBRDF(const SBRDFParameterInfo vBRDFParameterInfo)
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

//***********************************************************************************************
//Function:Main
void main()
{
	vec3 OriginalColor = texture(u_ModelMaterial.Diffuse, v2f_TextureCoord).rgb;
	//OriginalColor = pow(OriginalColor, vec3(2.2f));
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
	BRDFParameterInfo.Roughness = CalculateRoughnessWithCleaCoat(u_Roughness, u_ClearCoatRoughness, u_ClearCoatThickness);
	BRDFParameterInfo.Metallic = u_Metallic;

	float Distance = length(u_LightInfo.Position - v2f_FragPos);
	//Direct Light PBR
	vec3 ResultColor = u_LightInfo.Intensity * CalculateBRDF(BRDFParameterInfo) * u_LightInfo.Color / (Distance * Distance);

	//IBL Difuse
	//vec3 F0 = mix(BRDFParameterInfo.F0, OriginalColor, BRDFParameterInfo.Metallic);
	vec3 Ks = CalculateF0WithClearCoat(BRDFParameterInfo.F0, u_ClearCoatThickness, u_Reflectance, BRDFParameterInfo.AlbedoColor, BRDFParameterInfo.Metallic);//FresnelSchlickRoughness(max(dot(Normal, BRDFParameterInfo.ViewDir), 0.0f), F0, BRDFParameterInfo.Roughness);
	vec3 Kd = vec3(1.0f) - Ks;
	Kd *= 1.0f - BRDFParameterInfo.Metallic;
	vec3 IBLDiffuseColor = OriginalColor * Kd * texture(u_IrradianceMap, Normal).rgb;

	//IBL Specular
	const float MAX_REFLECTION_LOD = 4.0f;
	vec3 RelfectDir = reflect(-BRDFParameterInfo.ViewDir, Normal);
	vec3 PrefilteredEnvironmentColor = textureLod(u_EnvironmentMap, RelfectDir, BRDFParameterInfo.Roughness * MAX_REFLECTION_LOD).rgb;
	vec2 BRDFTerm = texture(u_BRDFTexture, vec2(max(dot(Normal, BRDFParameterInfo.ViewDir), 0.0f), BRDFParameterInfo.Roughness)).rg;
	vec3 IBLSpecularColor = PrefilteredEnvironmentColor * (Ks * BRDFTerm.x + BRDFTerm.y);

	//Clear Coat
	vec3 CC_Normal = Normal;
	float CC_NormalDotViewDir = max(dot(CC_Normal, BRDFParameterInfo.ViewDir), 0.0f);
	vec3 CC_ReflectDir = reflect(-BRDFParameterInfo.ViewDir, CC_Normal);
	float CC_F0 = 0.04f;
	float CC_F = FresnelSchlick(CC_NormalDotViewDir, CC_F0) * u_ClearCoatThickness;
	float Attenuation = 1.0f - CC_F;

	IBLDiffuseColor *= Attenuation;
	IBLSpecularColor *= Attenuation * Attenuation;
	vec3 CC_PrefilteredEnvironmentColor = textureLod(u_EnvironmentMap, CC_ReflectDir, u_ClearCoatRoughness * MAX_REFLECTION_LOD).rgb;
	vec3 CC_IBLSpecularColor = CC_PrefilteredEnvironmentColor * CC_F;
	IBLSpecularColor += CC_IBLSpecularColor;
	
	ResultColor += IBLDiffuseColor + IBLSpecularColor;

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + 1.0f);
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));
	//ResultColor = vec3(1);
	gl_FragColor = vec4(ResultColor, 1.0f);
}