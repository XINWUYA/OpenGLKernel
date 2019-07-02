#version 430 core

out vec4 gl_FragColor;

in vec3 g2f_WorldPos;

//#define USE_HDR_TEXTURE

#ifdef USE_HDR_TEXTURE
uniform sampler2D u_CubeMapTexture;
#else
uniform samplerCube u_CubeMapTexture;
#endif

uniform float u_Roughness;

const float PI = 3.14159265359f;
const vec2 INV_ATAN = vec2(0.1591f, 0.3183f);

vec2 ComputeSamplerHDRTextureUV(vec3 vDir)
{
	vec2 uv = vec2(atan(vDir.z, vDir.x), asin(vDir.y));
	uv *= INV_ATAN;
	uv += 0.5f;
	return uv;
}

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
	float CosTheta = sqrt((1.0 - vXi.y) / (1.0 + (a*a - 1.0) * vXi.y));
	float SinTheta = sqrt(1.0 - CosTheta * CosTheta);

	// from spherical coordinates to cartesian coordinates
	vec3 H;
	H.x = cos(Phi) * SinTheta;
	H.y = sin(Phi) * SinTheta;
	H.z = CosTheta;

	// from tangent-space vector to world-space sample vector
	vec3 Up = abs(vNormal.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 Tangent = normalize(cross(Up, vNormal));
	vec3 Bitangent = cross(vNormal, Tangent);

	vec3 SampleVec = Tangent * H.x + Bitangent * H.y + vNormal * H.z;
	return normalize(SampleVec);
}

void main()
{
	vec3 Normal = normalize(g2f_WorldPos);
	vec3 R = Normal;
	vec3 ViewDir = R;

	const uint SAMPLE_COUNT = 1024u;
	float TotalWeight = 0.0f;
	vec3 PrefilteredColor = vec3(0.0f);
	for (uint i = 0u; i < SAMPLE_COUNT; ++i)
	{
		vec2 Xi = Hammersley(i, SAMPLE_COUNT);
		vec3 H = ImportanceSampleGGX(Xi, Normal, u_Roughness);
		vec3 L = normalize(2.0f * dot(ViewDir, H) * H - ViewDir);

		float NormalDotL = max(dot(Normal, L), 0.0f);
		if (NormalDotL > 0.0f)
		{
#ifdef USE_HDR_TEXTURE
			vec2 uv = ComputeSamplerHDRTextureUV(L);
			PrefilteredColor += texture(u_CubeMapTexture, uv).rgb * NormalDotL;
#else
			PrefilteredColor += texture(u_CubeMapTexture, L).rgb * NormalDotL;
#endif
			TotalWeight += NormalDotL;
		}
	}
	PrefilteredColor = PrefilteredColor / TotalWeight;
	gl_FragColor = vec4(PrefilteredColor, 1.0f);
	//gl_FragColor = vec4(g2f_WorldPos, 1.0f);
}