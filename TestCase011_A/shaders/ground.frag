#version 430 core

out vec4 gl_FragColor;

in vec3 v2f_FragPos;

uniform vec3  u_CameraPos;
uniform vec3  u_PolygonalLightVertexPos[4];
uniform vec3 u_DiffuseColor = vec3(1.0f);
uniform vec3 u_SpecularColor = vec3(1.0f);
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
uniform float u_Roughness;
uniform bool  u_IsTwoSided = false;

uniform sampler2D u_LTCMatTexture;
uniform sampler2D u_LTCAmpTexture;

const float LUT_SIZE = 32.0;
const float LUT_SCALE = (LUT_SIZE - 1.0) / LUT_SIZE;
const float LUT_BIAS = 0.5 / LUT_SIZE;
const float PI = 3.14159265;

void clipPolygonalLightByZPlane(inout vec3 vioPolygonalLightVertexPos[5], out int voPolygonalLightVertexNumAfterClipping)
{
	int Flag4HowManyVertexAboveZPlane = 0;
	if(vioPolygonalLightVertexPos[0].z > 0.0) Flag4HowManyVertexAboveZPlane += 1;
	if(vioPolygonalLightVertexPos[1].z > 0.0) Flag4HowManyVertexAboveZPlane += 2;
	if(vioPolygonalLightVertexPos[2].z > 0.0) Flag4HowManyVertexAboveZPlane += 4;
	if(vioPolygonalLightVertexPos[3].z > 0.0) Flag4HowManyVertexAboveZPlane += 8;

	voPolygonalLightVertexNumAfterClipping = 0;
	if(Flag4HowManyVertexAboveZPlane == 0)
	{
	}
	else if(Flag4HowManyVertexAboveZPlane == 1)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 2)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 3)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 4)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
	}
	else if(Flag4HowManyVertexAboveZPlane == 5)
	{
		voPolygonalLightVertexNumAfterClipping = 0;
	}
	else if(Flag4HowManyVertexAboveZPlane == 6)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 7)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 8)
	{
		voPolygonalLightVertexNumAfterClipping = 3;
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = vioPolygonalLightVertexPos[3];
	}
	else if(Flag4HowManyVertexAboveZPlane == 9)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 10)
	{
		voPolygonalLightVertexNumAfterClipping = 0;
	}
	else if(Flag4HowManyVertexAboveZPlane == 11)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2];
	}
	else if(Flag4HowManyVertexAboveZPlane == 12)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
	}
	else if(Flag4HowManyVertexAboveZPlane == 13)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[3];
		vioPolygonalLightVertexPos[3] = vioPolygonalLightVertexPos[2];
		vioPolygonalLightVertexPos[2] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[2] + vioPolygonalLightVertexPos[2].z * vioPolygonalLightVertexPos[1];
		vioPolygonalLightVertexPos[1] = -vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0] + vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1];
	}
	else if(Flag4HowManyVertexAboveZPlane == 14)
	{
		voPolygonalLightVertexNumAfterClipping = 5;
		vioPolygonalLightVertexPos[4] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[3] + vioPolygonalLightVertexPos[3].z * vioPolygonalLightVertexPos[0];
		vioPolygonalLightVertexPos[0] = -vioPolygonalLightVertexPos[0].z * vioPolygonalLightVertexPos[1] + vioPolygonalLightVertexPos[1].z * vioPolygonalLightVertexPos[0];
	}
	else if(Flag4HowManyVertexAboveZPlane == 15)
	{
		voPolygonalLightVertexNumAfterClipping = 4;
	}

	if(voPolygonalLightVertexNumAfterClipping == 3)
		vioPolygonalLightVertexPos[3] = vioPolygonalLightVertexPos[0];
	if(voPolygonalLightVertexNumAfterClipping == 4)
		vioPolygonalLightVertexPos[4] = vioPolygonalLightVertexPos[0];
}

float integrateEdge(vec3 vVertex1, vec3 vVertex2)
{
	float CosTheta = dot(vVertex1, vVertex2);
	CosTheta = clamp(CosTheta, -0.9999, 0.9999);
	float Theta = acos(CosTheta);
	//return cross(vVertex1, vVertex2).z * Theta / sin(Theta);
	return cross(vVertex1, vVertex2).z * ((Theta > 0.001) ? Theta / sin(Theta) : 1.0);

	//float x = dot(vVertex1, vVertex2);
	//float y = abs(x);
	//float a = 5.42031 + (3.12829 + 0.0902326 * y) * y;
	//float b = 3.45068 + (4.18814 + y) * y;
	//float theta_sintheta = a / b;
	//if(x < 0.0)
	//	theta_sintheta = PI * sqrt(1.0 - x*x) - theta_sintheta;
	//return theta_sintheta * cross(vVertex1, vVertex2).z;
}

float integrateLTC(vec3 vNormal, vec3 vViewDir, vec3 vFragPos, mat3 vLTCMatrix, vec3 vPolygonalLightVertexPos[4], bool vTwoSided)
{	
	//着色点上的切线空间正交基
	vec3 Tangent = normalize(vViewDir - vNormal * dot(vViewDir, vNormal));
	vec3 Bitangent = cross(vNormal, Tangent);

	//将变换矩阵转换到切线空间
	vLTCMatrix = vLTCMatrix * transpose(mat3(Tangent, Bitangent, vNormal));
	//多边形顶点(因为被平面z=0裁剪以后，四边形可能变成5个顶点)
	vec3 PolygonalLightVertexPosInTangentSpace[5];
	PolygonalLightVertexPosInTangentSpace[0] = vLTCMatrix * (vPolygonalLightVertexPos[0] - vFragPos);
	PolygonalLightVertexPosInTangentSpace[1] = vLTCMatrix * (vPolygonalLightVertexPos[1] - vFragPos);
	PolygonalLightVertexPosInTangentSpace[2] = vLTCMatrix * (vPolygonalLightVertexPos[2] - vFragPos);
	PolygonalLightVertexPosInTangentSpace[3] = vLTCMatrix * (vPolygonalLightVertexPos[3] - vFragPos);
	//PolygonalLightVertexPosInTangentSpace[4] = PolygonalLightVertexPosInTangentSpace[3];

	int PolygonalLightVertexNumAfterClipping;
	clipPolygonalLightByZPlane(PolygonalLightVertexPosInTangentSpace, PolygonalLightVertexNumAfterClipping);
	
	if(PolygonalLightVertexNumAfterClipping == 0)
		return 0;
	
	//把裁剪后的多边形投影到球面上（也就是对每个顶点坐标向量归一化）
	PolygonalLightVertexPosInTangentSpace[0] = normalize(PolygonalLightVertexPosInTangentSpace[0]);
	PolygonalLightVertexPosInTangentSpace[1] = normalize(PolygonalLightVertexPosInTangentSpace[1]);
	PolygonalLightVertexPosInTangentSpace[2] = normalize(PolygonalLightVertexPosInTangentSpace[2]);
	PolygonalLightVertexPosInTangentSpace[3] = normalize(PolygonalLightVertexPosInTangentSpace[3]);
	PolygonalLightVertexPosInTangentSpace[4] = normalize(PolygonalLightVertexPosInTangentSpace[4]);

	//用累加边的公式来求解积分
	float Sum = 0.0;
	Sum += integrateEdge(PolygonalLightVertexPosInTangentSpace[0], PolygonalLightVertexPosInTangentSpace[1]);
	Sum += integrateEdge(PolygonalLightVertexPosInTangentSpace[1], PolygonalLightVertexPosInTangentSpace[2]);
	Sum += integrateEdge(PolygonalLightVertexPosInTangentSpace[2], PolygonalLightVertexPosInTangentSpace[3]);
	if(PolygonalLightVertexNumAfterClipping >= 4)
		Sum += integrateEdge(PolygonalLightVertexPosInTangentSpace[3], PolygonalLightVertexPosInTangentSpace[4]);
	if(PolygonalLightVertexNumAfterClipping == 5)
		Sum += integrateEdge(PolygonalLightVertexPosInTangentSpace[4], PolygonalLightVertexPosInTangentSpace[0]);
	Sum = vTwoSided ? abs(Sum) : max(Sum, 0.0);
	return Sum;
}

void main()
{
	vec3 GroundNormal = vec3(0, 1, 0);	//其他几何体的话应该由其法线乘以模型矩阵来算
	vec3 ViewDir = normalize(u_CameraPos - v2f_FragPos);

	float Theta = acos(dot(GroundNormal, ViewDir));
	vec2 UV = vec2(u_Roughness, Theta / (0.5 * PI));
	UV = UV * LUT_SCALE + LUT_BIAS;

	vec4 LTCMatrixComponents = texture2D(u_LTCMatTexture, UV);
	mat3 LTCMatrix = mat3
	(
		vec3(1, 0, LTCMatrixComponents.y),
		vec3(0, LTCMatrixComponents.z, 0),
		vec3(LTCMatrixComponents.w, 0, LTCMatrixComponents.x)
	);

	float Diffuse = integrateLTC(GroundNormal, ViewDir, v2f_FragPos, mat3(1), u_PolygonalLightVertexPos, u_IsTwoSided);
	float Specular = integrateLTC(GroundNormal, ViewDir, v2f_FragPos, LTCMatrix, u_PolygonalLightVertexPos, u_IsTwoSided);
	Specular *= texture2D(u_LTCAmpTexture, UV).w;

	vec3 ResultColor = u_LightColor * u_LightIntensity * (Diffuse * u_DiffuseColor + Specular * u_SpecularColor);
	//vec3 ResultColor = u_Intensity * Diffuse * u_DiffuseColor;
	ResultColor /= 2.0 * PI;

	//ResultColor = vec3(Specular);

	vec3 ReinhardMappedColor = ResultColor / (ResultColor + vec3(1.0));
	vec3 GammaedColor = pow(ReinhardMappedColor, vec3(1.0 / 2.2));

	gl_FragColor = vec4(GammaedColor, 1.0);
}