#version 430 core

out vec2 gl_FragColor;

in vec2 v2f_TextureCoord;

const float PI = 3.14159265359f;

float RadicalInverseVanDerCorpus(uint vBits)
{
	vBits = (vBits << 16u) | (vBits >> 16u);
	vBits = ((vBits & 0x55555555u) << 1u) | ((vBits & 0xAAAAAAAAu) >> 1u);
	vBits = ((vBits & 0x33333333u) << 2u) | ((vBits & 0xCCCCCCCCu) >> 2u);
	vBits = ((vBits & 0x0F0F0F0Fu) << 4u) | ((vBits & 0xF0F0F0F0u) >> 4u);
	vBits = ((vBits & 0x00FF00FFu) << 8u) | ((vBits & 0xFF00FF00u) >> 8u);
	return float(vBits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint vIndex, uint vTotalNum)
{
	return vec2(float(vIndex) / float(vTotalNum), RadicalInverseVanDerCorpus(vIndex));
}

vec3 ImportanceSampleGGX(vec2 vXi, vec3 vNormal, float vRoughness)
{
	float a = vRoughness * vRoughness;
	float Phi = 2.0f * PI * vXi.x;
	float CosTheta = sqrt((1.0f - vXi.y) / (1.0f + (a*a - 1.0f) * vXi.y));
	float SinTheta = sqrt(1.0f - CosTheta * CosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(Phi) * SinTheta;
	H.y = sin(Phi) * SinTheta;
	H.z = CosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 Up = abs(vNormal.z) < 0.999f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
	vec3 Tangent = normalize(cross(Up, vNormal));
	vec3 Bitangent = cross(vNormal, Tangent);

	vec3 SampleVec = Tangent * H.x + Bitangent * H.y + vNormal * H.z;
	return normalize(SampleVec);
}

float GeometrySchlickGGX(float vNdotV, float vRoughness)
{
	float a = vRoughness;
	float k = (a * a) / 2.0f;

	float Nom = vNdotV;
	float Denom = vNdotV * (1.0f - k) + k;

	return Nom / Denom;
}

float GeometrySmith(vec3 vN, vec3 vV, vec3 vL, float vRoughness)
{
	float NdotV = max(dot(vN, vV), 0.0f);
	float NdotL = max(dot(vN, vL), 0.0f);
	float GGX2 = GeometrySchlickGGX(NdotV, vRoughness);
	float GGX1 = GeometrySchlickGGX(NdotL, vRoughness);

	return GGX1 * GGX2;
}

vec2 IntegrateBRDF(float vNormalDotViewDir, float vRoughness)
{
//	vec3 V;
//	V.x = sqrt(1.0f - vNormalDotViewDir * vNormalDotViewDir);
//	V.y = 0.0f;
//	V.z = vNormalDotViewDir;
//
//	float A = 0.0f;
//	float B = 0.0f;
//	vec3 N = vec3(0.0f, 0.0f, 1.0f);
//	const uint SAMPLE_COUNT = 1024u;
//	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
//	{
//		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
//		vec3 H = ImportanceSampleGGX(Xi, N, vRoughness);
//		vec3 L = normalize(2.0f * dot(V, H) * H - V);
//
//		float NdotL = max(L.z, 0.0f);
//		float NdotH = max(H.z, 0.0f);
//		float VdotH = max(dot(V, H), 0.0f);
//
//		if (NdotL > 0.0f)
//		{
//			float G = GeometrySmith(N, V, L, vRoughness);
//			float G_Vis = (G * VdotH) / (NdotH * vNormalDotViewDir);
//			float Fc = pow(1.0f - VdotH, 5.0f);
//
//			A += (1.0f - Fc) * G_Vis;
//			B += Fc * G_Vis;
//		}
//	}
//
//	A /= float(SAMPLE_COUNT);
//	B /= float(SAMPLE_COUNT);
	const vec4 c0 = { -1, -0.0275, -0.572, 0.022 };
	const vec4 c1 = { 1, 0.0425, 1.04, -0.04 };
	vec4 r = vRoughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( -9.28 * vNormalDotViewDir ) ) * r.x + r.y;
	vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;

	return vec2(AB.x, AB.y);
}

void main()
{
	vec2 IntegrateBRDF = IntegrateBRDF(v2f_TextureCoord.x, v2f_TextureCoord.y);
	gl_FragColor = IntegrateBRDF;
}