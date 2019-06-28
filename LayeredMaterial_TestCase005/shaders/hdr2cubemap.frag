#version 430 core

out vec4 gl_FragColor;

in vec3 g2f_WorldPos;
uniform sampler2D u_HDRTexture;

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
	vec2 uv = ComputeSamplerHDRTextureUV(normalize(g2f_WorldPos));
	vec3 ResultColor = texture(u_HDRTexture, uv).rgb;

	gl_FragColor = vec4(ResultColor, 1.0f);
}