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
uniform float u_ModelRoughness;
uniform float u_ModelMetallic;
uniform float u_UpperLayerRoughness;
uniform float u_UpperLayerMetallic;
uniform float u_Transmittance;
uniform float u_LayerThickness;
uniform float u_Scale = 1.0f;
uniform float u_Gamma = 1.0f;
uniform bool u_IsUseApproximateMethod = true;

const float PI = 3.14159265359f;
const float INV_PI = 0.31830988618f;
const uint NB_SAMPLES = 25600;
const uint NB_PER_RUN = 256;

vec3 FresnelSchlick(float vCosTheta, vec3 vF0)
{
	return vF0 + (1.0 - vF0) * pow(1.0 - vCosTheta, 5.0f);
}

float FresnelTransmittance(float vRefractRatio, float vCosIncidentAngle, float vCosRefractAngle)
{
	float r1 = (vRefractRatio * vCosIncidentAngle - vCosRefractAngle) / (vRefractRatio * vCosIncidentAngle + vCosRefractAngle);
	float r2 = (vCosIncidentAngle - vRefractRatio * vCosRefractAngle) / (vCosIncidentAngle + vRefractRatio * vCosRefractAngle);
	float Kr = 0.5f * (r1 * r1 + r2 * r2);
	return 1 - Kr;
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

vec3 Gamma(vec3 R)
{
	return exp(u_Gamma * log(u_Scale * R));
}

float MapRoughness2Variance(float vRoughness)
{
	float a = pow(vRoughness, 1.1);
	return  a / (1 - a);
}

float MapVariance2Roughness(float vVariance)
{
	return pow(vVariance / (1.0f + vVariance), 1.0f / 1.1f);
}

vec2 QMC_Additive_2D(uint vIndex)
{
	return mod(vec2(0.5545497f, 0.308517f) * float(vIndex), vec2(1.0f, 1.0f));
}

void GGX_Sample_VNDF(in vec3 wi, in vec3 vNormal, in float alpha, in vec2 uv, out vec3 m, out float pdf) {
    // stretch view
    vec3 V = normalize(vec3(alpha * wi.x, alpha * wi.y, wi.z));
    // orthonormal basis
    vec3 T1 = (V.z < 0.9999) ? normalize(cross(V, vNormal)) : vec3(1,0,0);
    vec3 T2 = cross(T1, V);
    // sample point with polar coordinates (r, phi)
    float a = 1.0 / (1.0 + V.z);
    float r = sqrt(uv.x);
    float phi = (uv.y<a) ? uv.y/a * PI : PI + (uv.y-a)/(1.0-a) * PI;
    float P1 = r*cos(phi);
    float P2 = r*sin(phi)*((uv.y<a) ? 1.0 : V.z);
    // compute normal
    vec3 N = P1*T1 + P2*T2 + sqrt(max(0.0, 1.0 - P1*P1 - P2*P2))*V;
    // unstretch
    m = normalize(vec3(alpha*N.x, alpha*N.y, max(0.0, N.z)));
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
	
	vec3 PosInProjectiveSpace;
	PosInProjectiveSpace.xy = v2f_FragPos.xy;
	float sinTo = length(PosInProjectiveSpace.xy);
	if(sinTo <= 1.0f)
	{
		PosInProjectiveSpace.z = sqrt(1.0f - sinTo * sinTo);
		if(u_IsUseApproximateMethod)
		{
			float NewRoughness = MapVariance2Roughness(MapRoughness2Variance(u_UpperLayerRoughness) + MapRoughness2Variance(u_ModelRoughness));

			SBRDFParameterInfo BRDFParameterInfo;
			BRDFParameterInfo.ViewDir = PosInProjectiveSpace;//normalize(u_CameraPos - v2f_FragPos);
			BRDFParameterInfo.LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
			BRDFParameterInfo.Normal = CorrectNormal(Normal, BRDFParameterInfo.ViewDir);
			BRDFParameterInfo.F0 = vec3(0.04f);
			BRDFParameterInfo.AlbedoColor = LowestLayerAlbedoColor;
			BRDFParameterInfo.Roughness = NewRoughness;
			BRDFParameterInfo.Metallic = u_ModelMetallic;

			float Distance = length(u_LightInfo.Position - v2f_FragPos);
			ResultColor += /*LowestLayerAmbientColor + */u_LightInfo.Intensity * CalculateBRDF(BRDFParameterInfo, 0.0f) * u_LightInfo.Color;// / (Distance * Distance);
		}
		else
		{
			for(uint i = 0; i < NB_PER_RUN; ++i)
			{
				uint Idx = uint(mod(NB_PER_RUN + i, NB_SAMPLES));
				vec2 Rnd = QMC_Additive_2D(Idx);
				vec3 m;
				float pdf;
				GGX_Sample_VNDF(PosInProjectiveSpace, CorrectNormal(Normal, normalize(u_CameraPos - v2f_FragPos)), u_ModelRoughness, Rnd.xy, m, pdf);
				vec3 ws = reflect(PosInProjectiveSpace, m);
				ws.z = -ws.z;
				if(ws.z > 0.0f)
				{
					SBRDFParameterInfo BRDFParameterInfo;
					BRDFParameterInfo.ViewDir = ws;//normalize(u_CameraPos - v2f_FragPos);
					BRDFParameterInfo.LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
					BRDFParameterInfo.Normal = CorrectNormal(Normal, BRDFParameterInfo.ViewDir);
					BRDFParameterInfo.F0 = vec3(0.04f);
					BRDFParameterInfo.AlbedoColor = LowestLayerAlbedoColor;
					BRDFParameterInfo.Roughness = u_UpperLayerRoughness;
					BRDFParameterInfo.Metallic = u_ModelMetallic;

					float Distance = length(u_LightInfo.Position - v2f_FragPos);
					ResultColor += /*LowestLayerAmbientColor + */u_LightInfo.Intensity * CalculateBRDF(BRDFParameterInfo, 0.0f) * u_LightInfo.Color;// / (Distance * Distance);
				}
			}
			ResultColor /= float(NB_PER_RUN);
		}
	}
	

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + vec3(1.0f));
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));

	gl_FragColor = vec4(ResultColor, 1.0f);
}