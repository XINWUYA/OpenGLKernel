#version 430 core

out vec4 FragColor;

in vec2 v2f_TexCoord;
in vec3 v2f_FragPos;
in vec3 v2f_Normal;

uniform sampler2D uTexture;
uniform samplerCube u_ShadowMapTex;
uniform float u_FarPlane;
uniform vec3 uCameraPos;
uniform vec3 u_LightPos = vec3(-2.0f, 4.0f, -1.0f);
uniform bool u_IsScence = false;

vec3 LightColor = vec3(1.0);

float ShadowCalculation(vec3 fragPos)
{
	// Get vector between fragment position and light position
	vec3 fragToLight = fragPos - u_LightPos;
	// Use the light to fragment vector to sample from the depth map    
	float closestDepth = texture(u_ShadowMapTex, normalize(fragToLight)).r;
	// It is currently in linear range between [0,1]. Re-transform back to original value
	//closestDepth *= u_FarPlane;
	// Now get current linear depth as the length between the fragment and light position
	float currentDepth = length(fragToLight);
	// Now test for shadows
	float bias = 0.05;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;

	return closestDepth;
}

void main()
{
	vec3 Normal = normalize(v2f_Normal);
	if (u_IsScence)
		Normal = -Normal;

	float AmbientStrength = 0.1;
	vec3 AlbedoColor = vec3(1.0f);

	vec3 LightDir = normalize(u_LightPos - v2f_FragPos);
	vec3 ViewDir = normalize(uCameraPos - v2f_FragPos);
	vec3 HalfwayDir = normalize(LightDir + ViewDir);

	vec3 Ambient = AmbientStrength * LightColor;

	vec3 Diffuse = max(dot(Normal, LightDir), 0.0) * LightColor;

	float SpecularStrength = 1;
	vec3 ReflectDir = reflect(-LightDir, Normal);
	vec3 Specular = SpecularStrength * pow(max(dot(Normal, HalfwayDir), 0.0), 32) * LightColor;

	float Shadow = ShadowCalculation(v2f_FragPos);
	vec3 ResultColor = (Ambient + (1.0 - Shadow) * (Diffuse + Specular)) * AlbedoColor;

	//FragColor = vec4(ResultColor, 1.0);
	FragColor = vec4(vec3(texture(u_ShadowMapTex, normalize(v2f_FragPos)).g), 1.0);
	//FragColor = vec4(normalize(vec3(v2f_FragPos)), 1.0);
}