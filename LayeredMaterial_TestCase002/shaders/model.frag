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
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AOTexture;
uniform sampler2D u_MetallicTexture;
uniform vec3 u_CameraPos;
uniform float u_Roughness;
uniform float u_Metallic;
uniform float u_UpperLayerRoughness;
uniform float u_UpperLayerMetallic;
uniform float u_Transmittance;
uniform float u_LayerThickness;
uniform bool u_IsUseDoubleLayer = true;

const float PI = 3.14159265359f;

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

vec3 CalculateNormalFromNormalMap()
{
	vec3 TangentNormal = 2.0f * texture(u_NormalTexture,v2f_TextureCoord).xyz - 1.0f;
	vec3 Q1 = dFdx(v2f_FragPos);
	vec3 Q2 = dFdy(v2f_FragPos);
	vec2 St1 = dFdx(v2f_TextureCoord);
	vec2 St2 = dFdy(v2f_TextureCoord);

	vec3 Normal = normalize(v2f_Normal);
	vec3 Tangent = normalize(Q1*St2.t - Q2*St1.t);
	vec3 BiTangent = -normalize(cross(Normal, Tangent));
	mat3 TBN = mat3(Tangent, BiTangent, Normal);
	
	return normalize(TBN * TangentNormal);
}

float CalculateAbsorption(vec3 vNormal, vec3 vViewDir, vec3 vLightDir, float vLayerThickness, float vAlpha)//vAlpha：常量，指介质对某波长光线的吸收系数
{
	float vNormalDotViewDir = max(dot(vNormal, vViewDir), 0.0f);
	float vNormalDotLightDir = max(dot(vNormal, vLightDir), 0.0f);

	float Absorption = exp(-vAlpha * vLayerThickness * (vNormalDotViewDir + vNormalDotLightDir) / (vNormalDotViewDir * vNormalDotLightDir + 0.001f));
	return Absorption;
}

vec3 CalculateBRDF(SBRDFParameterInfo vBRDFParameterInfo, float vTransmittance)
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

	vec3 ResultBRDF = (DiffuseItem * (1.0f - vTransmittance) + SpecularItem) * max(dot(vBRDFParameterInfo.Normal, vBRDFParameterInfo.LightDir), 0.0f);
	return ResultBRDF;
}

void main()
{
	//Lowest Layer Info
	vec3 LowestLayerAlbedoColor = vec3(0.0f, 1.0f, 0.0f); //texture(u_DiffuseTexture, v2f_TextureCoord).rgb;
	LowestLayerAlbedoColor = pow(LowestLayerAlbedoColor, vec3(2.2f));
	float LowestLayerAO = 1.0f;//texture(u_AOTexture, v2f_TextureCoord).r;
	float LowestLayerAmbientWeight = 0.03f;
	vec3 LowestLayerAmbientColor = LowestLayerAO * LowestLayerAmbientWeight * LowestLayerAlbedoColor;

	vec3 Normal = CalculateNormalFromNormalMap();
	
	vec3 ResultColor = vec3(0.0f);
	if(u_IsUseDoubleLayer)
	{
		//Upper Layer
		vec3 UpperLayerAlbedoColor = vec3(1.0f, 1.0f, 0.0f);
		float UpperLayerAmbientWeight = 0.03f;
		vec3 UpperLayerAmbientColor = UpperLayerAmbientWeight * UpperLayerAlbedoColor;
		vec3 UpperLayerPos = v2f_FragPos + u_LayerThickness * Normal;

		vec3 UpperLayerViewDir = normalize(u_CameraPos - UpperLayerPos);
		vec3 UpperLayerLightDir = normalize(u_LightInfo.Position - UpperLayerPos);
		vec3 UpperLayerNormal = CorrectNormal(Normal, UpperLayerViewDir);

		float Distance = length(u_LightInfo.Position - UpperLayerPos);
		vec3 UpperLayerHalfVec = normalize(UpperLayerViewDir + UpperLayerLightDir);
		vec3 UpperLayerF0 = vec3(0.04f);
		UpperLayerF0 = mix(UpperLayerF0, UpperLayerAlbedoColor, u_UpperLayerMetallic);
		vec3 UpperLayerF = FresnelSchlick(max(dot(UpperLayerHalfVec, UpperLayerViewDir), 0.0f), UpperLayerF0);
		float UpperLayerD = DistributionGGX(UpperLayerNormal, UpperLayerHalfVec, u_UpperLayerRoughness);
		float UpperLayerG = GeometrySmith(UpperLayerNormal, UpperLayerViewDir, UpperLayerLightDir, u_UpperLayerRoughness);
		vec3 UpperLayerSpecularItem = (UpperLayerD * UpperLayerG * UpperLayerF) / (4.0f * max(dot(UpperLayerNormal, UpperLayerViewDir), 0.0f) * max(dot(UpperLayerNormal, UpperLayerLightDir), 0.0f) + 0.001f);

		vec3 UpperLayerKs = UpperLayerF;
		vec3 UpperLayerKd = vec3(1.0f) - UpperLayerKs;
		UpperLayerKd *= 1.0f - u_UpperLayerMetallic;
		vec3 UpperLayerDiffuseItem = UpperLayerKd * UpperLayerAlbedoColor / PI;

		vec3 UpperLayerReflectColor = UpperLayerAmbientColor + u_LightInfo.Intensity * (UpperLayerDiffuseItem * (1.0f - u_Transmittance) + UpperLayerSpecularItem) * max(dot(UpperLayerNormal, UpperLayerLightDir), 0.0f) * u_LightInfo.Color / (Distance * Distance);
		ResultColor += UpperLayerReflectColor;

		//Lowest Layer
		vec3 RefractColor = u_LightInfo.Intensity * UpperLayerDiffuseItem * u_Transmittance * max(dot(UpperLayerNormal, UpperLayerLightDir), 0.0f) * u_LightInfo.Color / (Distance * Distance);
		//ResultColor = RefractColor;

		float RefractRatio = 1.00f / 1.52f;
		vec3 LowestLayerViewDir = -normalize(refract(-UpperLayerViewDir, UpperLayerNormal, RefractRatio));
		vec3 LowestLayerLightDir = -normalize(refract(-UpperLayerLightDir, UpperLayerNormal, RefractRatio));
		vec3 LowestLayerNormal = CorrectNormal(Normal, LowestLayerViewDir);
		vec3 LowestLayerHalfVec = normalize(LowestLayerViewDir + LowestLayerLightDir);

		vec3 LowestLayerF0 = vec3(0.04f);
		LowestLayerF0 = mix(LowestLayerF0, LowestLayerAlbedoColor, u_Metallic);
		vec3 LowestLayerF = FresnelSchlick(max(dot(LowestLayerHalfVec, LowestLayerViewDir), 0.0f), LowestLayerF0);
		float LowestLayerD = DistributionGGX(LowestLayerNormal, LowestLayerHalfVec, u_Roughness);
		float LowestLayerG = GeometrySmith(LowestLayerNormal, LowestLayerViewDir, LowestLayerLightDir, u_Roughness);
		vec3 LowestLayerSpecularItem = (LowestLayerD * LowestLayerG * LowestLayerF) / (4.0f * max(dot(LowestLayerNormal, LowestLayerViewDir), 0.0f) * max(dot(LowestLayerNormal, LowestLayerLightDir), 0.0f) + 0.001f);
		
		vec3 LowestLayerKs = LowestLayerF;
		vec3 LowestLayerKd = vec3(1.0f) - LowestLayerKs;
		LowestLayerKd *= 1.0f - u_Metallic;
		vec3 LowestLayerDiffuseItem = LowestLayerKd * LowestLayerAlbedoColor / PI;

		float Alpha = log(1.0f / u_Transmittance) / log(10.0f);
		float Absorption = CalculateAbsorption(LowestLayerNormal, LowestLayerViewDir, LowestLayerLightDir, u_LayerThickness, Alpha);
		vec3 LowestLayerReflectColor = RefractColor * (LowestLayerDiffuseItem + LowestLayerSpecularItem) * max(dot(LowestLayerNormal, LowestLayerLightDir), 0.0f) * Absorption;
		ResultColor += LowestLayerReflectColor;
	}
	else
	{
		SBRDFParameterInfo LowestLayerBRDFParameterInfo;
		LowestLayerBRDFParameterInfo.ViewDir = normalize(u_CameraPos - v2f_FragPos);
		LowestLayerBRDFParameterInfo.LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
		LowestLayerBRDFParameterInfo.Normal = CorrectNormal(Normal, LowestLayerBRDFParameterInfo.ViewDir);
		LowestLayerBRDFParameterInfo.F0 = vec3(0.04f);
		LowestLayerBRDFParameterInfo.AlbedoColor = LowestLayerAlbedoColor;
		LowestLayerBRDFParameterInfo.Roughness = u_Roughness;
		LowestLayerBRDFParameterInfo.Metallic = u_Metallic;

		float Distance = length(u_LightInfo.Position - v2f_FragPos);
		ResultColor = LowestLayerAmbientColor + u_LightInfo.Intensity * CalculateBRDF(LowestLayerBRDFParameterInfo, 0.0f) * u_LightInfo.Color / (Distance * Distance);
	}

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + vec3(1.0f));
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));

	gl_FragColor = vec4(ResultColor, 1.0f);
}