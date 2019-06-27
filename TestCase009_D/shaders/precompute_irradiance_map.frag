#version 430 core

out vec4 gl_FragColor;

in vec3 g2f_WorldPos;

//#define USE_HDR_TEXTURE

#ifdef USE_HDR_TEXTURE
uniform sampler2D u_CubeMapTexture;
#else
uniform samplerCube u_CubeMapTexture;
#endif

const float PI = 3.14159265359f;
const vec2 INV_ATAN = vec2(0.1591f, 0.3183f);

vec2 ComputeSamplerHDRTextureUV(vec3 vDir)
{
	vec2 uv = vec2(atan(vDir.z, vDir.x), asin(vDir.y));
	uv *= INV_ATAN;
	uv += 0.5f;
	return uv;
}

void main()
{
	vec3 Normal = normalize(g2f_WorldPos);
	vec3 Up = vec3(0.0f, 1.0f, 0.0f);
	vec3 Right = cross(Up, Normal);
	Up = cross(Normal, Right);

	vec3 ResultIrradiance = vec3(0.0f);
	float SampleDelta = 0.025f;
	int SampleCnt = 0;
	for (float vPhi = 0.0f; vPhi < 2.0f * PI; vPhi += SampleDelta)
	{
		for (float vTheta = 0.0f; vTheta < 0.5f * PI; vTheta += SampleDelta)
		{
			vec3 TangentSample = vec3(sin(vTheta) * cos(vPhi), sin(vTheta) * sin(vPhi), cos(vTheta));
			vec3 SampleVec = TangentSample.x * Right + TangentSample.y * Up + TangentSample.z * Normal;
#ifdef USE_HDR_TEXTURE
			vec2 uv = ComputeSamplerHDRTextureUV(SampleVec);
			ResultIrradiance += texture(u_CubeMapTexture, uv).rgb * cos(vTheta) * sin(vTheta);
#else
			ResultIrradiance += texture(u_CubeMapTexture, SampleVec).rgb * cos(vTheta) * sin(vTheta);
#endif
			SampleCnt++;
		}
	}
	ResultIrradiance = PI * ResultIrradiance * (1.0f / float(SampleCnt));
	gl_FragColor = vec4(ResultIrradiance, 1.0f);
	//gl_FragColor = vec4(g2f_WorldPos, 1.0f);
}