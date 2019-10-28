#version 430 core

out vec4 FragColor;

in vec2 v2f_TexCoord;
in vec3 v2f_FragPos;
in vec3 v2f_Normal;
in vec4 v2f_FragPosInLightSpace;

uniform vec3 uCameraPos;
uniform sampler2D uTexture;
uniform sampler2D u_ShadowMapTex;

vec3 LightPos = vec3(-2.0f, 4.0f, -1.0f);
vec3 LightColor = vec3(1.0);

float ShadowCalculation(vec4 fragPosLightSpace)
{
	// ִ��͸�ӳ���
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	// �任��[0,1]�ķ�Χ
	projCoords = projCoords * 0.5 + 0.5;
	// ȡ�����������(ʹ��[0,1]��Χ�µ�fragPosLight������)
	float closestDepth = texture(u_ShadowMapTex, projCoords.xy).r;
	// ȡ�õ�ǰƬԪ�ڹ�Դ�ӽ��µ����
	float currentDepth = projCoords.z;
	// ��鵱ǰƬԪ�Ƿ�����Ӱ��
	vec3 normal = normalize(v2f_Normal);
	vec3 lightDir = normalize(LightPos - v2f_FragPos);
	float bias = 0.00001f;// max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	if (projCoords.z > 1.0)
		shadow = 0.0;
	return shadow;
}

void main()
{
	vec3 Normal = normalize(v2f_Normal);
	float AmbientStrength = 0.1;
	vec3 AlbedoColor = vec3(1.0f);

	vec3 LightDir = normalize(LightPos - v2f_FragPos);
	vec3 ViewDir = normalize(uCameraPos - v2f_FragPos);
	vec3 HalfwayDir = normalize(LightDir + ViewDir);

	vec3 Ambient = AmbientStrength * LightColor;

	vec3 Diffuse = max(dot(Normal, LightDir), 0.0) * LightColor;

	float SpecularStrength = 1;
	vec3 ReflectDir = reflect(-LightDir, Normal);
	vec3 Specular = SpecularStrength * pow(max(dot(Normal, HalfwayDir), 0.0), 32) * LightColor;

	float Shadow = ShadowCalculation(v2f_FragPosInLightSpace);
	vec3 ResultColor = (Ambient + (1.0 - Shadow) * (Diffuse + Specular)) * AlbedoColor;

	FragColor = vec4(ResultColor, 1.0);
	//FragColor = vec4(vec3(Shadow), 1.0);
}