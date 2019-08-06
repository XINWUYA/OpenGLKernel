#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;
in vec3 v2f_Normal;
in vec3 v2f_FragPos;
in vec3 v2f_Tangent;

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

uniform SModelMaterial u_ModelMaterial;
uniform SLightInfo u_LightInfo;
uniform vec3 u_CameraPos;
uniform bool u_IsUsePBR = true;

const float PI = 3.141592653f;

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

mat3 CalculateTBN(vec3 vWorldPos, vec2 vTextureCoord, vec3 vNormal)
{
	vec3 Q1 = dFdx(vWorldPos);
	vec3 Q2 = dFdy(vWorldPos);
	vec2 St1 = dFdx(vTextureCoord);
	vec2 St2 = dFdy(vTextureCoord);

	vec3 Normal = normalize(vNormal);
	vec3 Tangent = normalize(Q1*St2.t - Q2*St1.t);
	vec3 BiTangent = -normalize(cross(Normal, Tangent));
	mat3 TBN = mat3(Tangent, BiTangent, Normal);
	
	return TBN;
}

vec3 CalculateNormalFromNormalMap()
{
	vec3 TangentNormal = 2.0f * texture(u_ModelMaterial.Normal, v2f_TextureCoord).xyz - 1.0f;
	mat3 TBN = CalculateTBN(v2f_FragPos, v2f_TextureCoord, v2f_Normal);
	
	return normalize(TBN * TangentNormal);
}

vec3 CorrectNormal(vec3 vNormal, vec3 vViewDir)
{
	if(dot(vNormal, vViewDir) < 0.0f)
		vNormal = normalize(vNormal - 1.01 * vViewDir * dot(vNormal, vViewDir));
	return vNormal;
}

void main()
{
	vec3 OriginalColor = texture(u_ModelMaterial.Diffuse, v2f_TextureCoord).rgb;
	OriginalColor = pow(OriginalColor, vec3(2.2f));

	float AmbientWeight = 0.03f;
	vec3 AmbientColor = AmbientWeight * OriginalColor;

	vec3 ViewDir = normalize(u_CameraPos - v2f_FragPos);
	vec3 LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
	float Distance = length(u_LightInfo.Position - v2f_FragPos);
	vec3 HalfVec = normalize(ViewDir + LightDir);

	vec3 Normal = CalculateNormalFromNormalMap();

	vec3 ResultColor;
	if(u_IsUsePBR)
	{
		//PBR
		float Roughness = texture(u_ModelMaterial.Roughness, v2f_TextureCoord).r;
		float Metallic = texture(u_ModelMaterial.Metallic,v2f_TextureCoord).r;

		vec3 F0 = vec3(0.22f);
		F0 = mix(F0, OriginalColor, Metallic);
		vec3 F = FresnelSchlick(max(dot(HalfVec, ViewDir), 0.0f), F0);
		float D = DistributionGGX(Normal, HalfVec, Roughness);
		float G = GeometrySmith(Normal, ViewDir, LightDir, Roughness);
		vec3 SpecularColor = (D * G * F) / (4.0f * max(dot(Normal, ViewDir), 0.0f) * max(dot(Normal, LightDir), 0.0f) + 0.001f);

		vec3 Ks = F;
		vec3 Kd = vec3(1.0f) - Ks;
		Kd *= 1.0f - Metallic;

		vec3 DiffuseColor = OriginalColor;// / PI;

		ResultColor = AmbientColor + u_LightInfo.Intensity * (Kd * DiffuseColor + SpecularColor) * max(dot(Normal, LightDir), 0.0f) * u_LightInfo.Color / (Distance * Distance);
	}
	else
	{
		// Blinn-Phong Model
		vec3 DiffuseColor = max(dot(LightDir, Normal), 0.0f) * OriginalColor;
		vec3 SpecularColor = pow(max(dot(Normal, HalfVec), 0.0f), 64) * OriginalColor;

		ResultColor = AmbientColor + u_LightInfo.Intensity * (DiffuseColor + 0.9 * SpecularColor) * u_LightInfo.Color / (Distance * Distance);
	}

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + 1.0f);
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));

	gl_FragColor = vec4(ResultColor, 1.0f);
}