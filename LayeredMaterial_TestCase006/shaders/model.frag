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
uniform samplerCube u_IrradianceMap;
uniform samplerCube u_EnvironmentMap;
uniform sampler2D u_BRDFTexture;
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_MetallicTexture;
uniform sampler2D u_RoughnessTexture;
uniform sampler2D u_AOTexture;
uniform sampler2D u_IridescenceThickTexture;
uniform vec3 u_CameraPos;
uniform float u_Roughness;
uniform float u_Metallic;
uniform float u_Reflectance;
uniform float u_ClearCoatRoughness;
uniform float u_ClearCoatThickness;
uniform float u_ThinFilmIridescene;
uniform float u_ThinFilmThickness;
uniform float u_ThinFilmIOR;
uniform float u_ThinFilmExtinctionK;

const float PI = 3.141592653f;
const float MAX_CLEAR_COAT_ROUGHNESS = 0.6f;
const float BRDF_CLEAR_COAT_IOR = 1.5f;
const float MIN_ROUGHNESS = 0.045f;
// XYZ to CIE 1931 RGB color space (using neutral E illuminant)
const mat3 XYZ_TO_RGB = mat3(2.3706743, -0.5138850, 0.0052982, -0.9000405, 1.4253036, -0.0146949, -0.4706338, 0.0885814, 1.0093968);

//***********************************************************************************************
//Function: 
vec3 ConvertSRGBtoLinear(vec3 vColor)
{
        return pow(vColor, vec3(2.2f));
}

//***********************************************************************************************
//Function: 
vec3 ConvertF0ClearCoat2Surface(const vec3 vF0)
{
  // Approximation of iorToF0(f0ToIor(f0), 1.5). Assumes IOR of 1.5 (4% reflectance)
  return clamp(vF0 * (vF0 * (0.941892f - 0.263008f * vF0) + 0.346479f) - 0.0285998f, 0.0f, 1.0f);
}

//***********************************************************************************************
//Function: 
float RemapClearCoatRoughness(float vInputRoughness)
{
	return mix(MIN_ROUGHNESS, MAX_CLEAR_COAT_ROUGHNESS, vInputRoughness);//Remapped to 0.0..0.6 matches the fact that clear coat layers are almost always glossy.
}

//***********************************************************************************************
//Function:
vec3 CalculateF0WithClearCoat(vec3 vF0, float vClearCoatThickness)
{
	return mix(vF0, ConvertF0ClearCoat2Surface(vF0), vClearCoatThickness);
}

//***********************************************************************************************
//Function:
float CalculateRoughnessWithClearCoat(float vRoughness, float vClearCoatRoughness, float vClearCoatThickness)
{
	float ClearCoatRoughness = RemapClearCoatRoughness(vClearCoatRoughness);
	return mix(vRoughness, max(vRoughness, ClearCoatRoughness), vClearCoatThickness);
}

//***********************************************************************************************
//Function:
vec3 FresnelSchlickRoughness(float vCosTheta, vec3 vF0, float vRoughness)
{
    return vF0 + (max(vec3(1.0f - vRoughness), vF0) - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

//***********************************************************************************************
//Function:
vec3 FresnelSchlick(float vCosTheta, const vec3 vF0)
{
	return vF0 + (1.0f - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

float FresnelSchlick(float vCosTheta, float vF0)
{
	return vF0 + (1.0f - vF0) * pow(1.0f - vCosTheta, 5.0f);
}

//***********************************************************************************************
//Function:
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

//***********************************************************************************************
//Function:
float GeometrySchlickGGX(float vNormalDotViewDir, float vRoughness)
{
	float r = (vRoughness + 1.0f);
	float k = r * r / 8.0f;

	float Nom = vNormalDotViewDir;
	float Denom = vNormalDotViewDir * (1.0f - k) + k;
	return Nom / Denom;
}

//***********************************************************************************************
//Function:
float GeometrySmith(vec3 vNormal, vec3 vViewDir, vec3 vLightDir, float vRoughness)
{
	float NormalDotViewDir = max(dot(vNormal, vViewDir), 0.0f);
	float NormalDotLightDir = max(dot(vNormal, vLightDir), 0.0f);

	return GeometrySchlickGGX(NormalDotViewDir, vRoughness) * GeometrySchlickGGX(NormalDotLightDir, vRoughness);
}

//***********************************************************************************************
//Function:
vec3 CorrectNormal(vec3 vNormal, vec3 vViewDir)
{
	if(dot(vNormal, vViewDir) < 0.0f)
		vNormal = normalize(vNormal - 1.01 * vViewDir * dot(vNormal, vViewDir));
	return vNormal;
}

//***********************************************************************************************
//Function:
vec3 CalculateBRDF(const SBRDFParameterInfo vBRDFParameterInfo)
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

	return (DiffuseItem + SpecularItem) * max(dot(vBRDFParameterInfo.Normal, vBRDFParameterInfo.LightDir), 0.0f);
}

//***********************************************************************************************
//Function:
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

//***********************************************************************************************
//Function:
vec3 CalculateNormalFromNormalMap()
{
	vec3 TangentNormal = 2.0f * texture(u_NormalTexture,v2f_TextureCoord).xyz - 1.0f;
	mat3 TBN = CalculateTBN(v2f_FragPos, v2f_TextureCoord, v2f_Normal);
	
	return normalize(TBN * TangentNormal);
}

//***********************************************************************************************
//Function:
float CalculateSpecularAO(float vNDotV, float vDiffuseAO, float vRoughness)
{
	return clamp(pow(vNDotV + vDiffuseAO, exp2(-16.0f * vRoughness - 1.0f)) - 1.0f + vDiffuseAO, 0.0f, 1.0f);
}

//***********************************************************************************************
//Function:
float CalculateIridescenceMask(float vIridescence)
{
	return clamp(1.0f - pow(1.0f - vIridescence, 3.0f), 0.0f, 1.0f); 
}

//***********************************************************************************************
//Function:
float sq(float x)
{
        return x * x;
}

vec2 sq(vec2 x)
{
  return vec2(x.x * x.x, x.y * x.y);
}

// Belcour & Barla 2017, "A practical extension to microfacet theory for the modeling of varying iridescence"
// Fresnel equations for dielectric/dielectric interfaces.
void F_Dielectric(in float ct1, in float n1, in float n2, out vec2 R, out vec2 phi)
{
  // Sinus theta1 'squared'
  float st1 = (1 - ct1*ct1);
  float nr = n1/n2;

  if(sq(nr) * st1 > 1.0) // Total Internal Reflection
  {
    R = vec2(1.0);
    phi = 2.0 * atan(vec2(-sq(nr) *  sqrt(st1 - 1.0/sq(nr)) / ct1,-sqrt(st1 - 1.0/sq(nr)) / ct1));
  }
  else // Transmission & Reflection
  {

    float ct2 = sqrt(1 - sq(nr) * st1);
    vec2 r = vec2((n2*ct1 - n1*ct2) / (n2*ct1 + n1*ct2),
            (n1*ct1 - n2*ct2) / (n1*ct1 + n2*ct2));
    phi.x = (r.x < 1e-5) ? PI : 0.0;
    phi.y = (r.y < 1e-5) ? PI : 0.0;
    R = sq(r);
  }
}

// Belcour & Barla 2017, "A practical extension to microfacet theory for the modeling of varying iridescence"
// Fresnel equations for dielectric/conductor interfaces.
void F_Conductor(in float ct1, in float n1, in float n2, in float k, out vec2 R, out vec2 phi)
{
  if (k < 1e-5)
  {
    // Use dielectric formula to avoid numerical issues
    F_Dielectric(ct1, n1, n2, R, phi);
    return;
  }

  float A = sq(n2) * (1-sq(k)) - sq(n1) * (1-sq(ct1));
  float B = sqrt( sq(A) + sq(2*sq(n2)*k) );
  float U = sqrt((A+B)/2.0);
  float V = sqrt((B-A)/2.0);

  R.y = (sq(n1*ct1 - U) + sq(V)) / (sq(n1*ct1 + U) + sq(V));
  phi.y = atan(sq(U)+sq(V)-sq(n1*ct1), 2*n1 * V*ct1) + PI;

  R.x = (sq(sq(n2)*(1-sq(k))*ct1 - n1*U) + sq(2*sq(n2)*k*ct1 - n1*V))
    / (sq(sq(n2)*(1-sq(k))*ct1 + n1*U) + sq(2*sq(n2)*k*ct1 + n1*V));
  phi.x = atan(sq(sq(n2)*(1+sq(k))*ct1) - sq(n1)*(sq(U)+sq(V)), 2*n1*sq(n2)*ct1 * (2*k*U - (1-sq(k))*V));
}

// Belcour & Barla 2017, "A practical extension to microfacet theory for the modeling of varying iridescence"
// Evaluation XYZ sensitivity curves in Fourier space
vec3 evaluateSensitivity(float opd, float shift)
{
  // OPD: Optical Path Difference
  // Use Gaussian fits, given by 3 parameters: val, pos and var
  float phase = 2*PI * opd * 1.0e-6;
  vec3 val = vec3(5.4856e-13, 4.4201e-13, 5.2481e-13);
  vec3 pos = vec3(1.6810e+6, 1.7953e+6, 2.2084e+6);
  vec3 var = vec3(4.3278e+9, 9.3046e+9, 6.6121e+9);
  vec3 xyz = val * sqrt(2*PI * var) * cos(pos * phase + shift) * exp(-var * phase*phase);
  xyz.x   += (9.7470e-14 * sqrt(2*PI * 4.5282e+9) * cos(2.2399e+6 * phase + shift) * exp(-4.5282e+9 * phase*phase));
  return xyz / 1.0685e-7;
}

float Depol (vec2 polV)
{
  return 0.5 * (polV.x + polV.y);
}

vec3 DepolColor (vec3 colS, vec3 colP)
{
  return 0.5 * (colS + colP);
}

//***********************************************************************************************
//Function:
vec3 CalculateFresnelAiry(float ct1, float ct2, float Dinc, float eta_1, float eta_2, float eta_3, float k)
{
	// First interface
  vec2 R12, phi12;
  F_Dielectric(ct1, eta_1, eta_2, R12, phi12);
  vec2 R21  = vec2(R12);
  vec2 T121 = vec2(1.0) - R12;
  vec2 phi21 = vec2(PI, PI) - phi12;

  // Second interface
  vec2 R23, phi23;
  F_Conductor(ct2, eta_2, eta_3, k, R23, phi23);

  // Phase shift
  float OPD = Dinc * ct2;
  vec2 phi2 = phi21 + phi23;

  // Compound terms
  vec3 I = vec3(0, 0, 0);
  vec2 R123 = R12 * R23;
  vec2 r123 = sqrt(R123);
  vec2 Rs   = sq(T121) * R23 / (vec2(1.0) - R123);

  // Reflectance term for m=0 (DC term amplitude)
  vec2 C0 = R12 + Rs;
  vec3 S0 = evaluateSensitivity(0.0, 0.0);
  I += (Depol(C0) * S0);

  // Reflectance term for m>0 (pairs of diracs)
  vec2 Cm = Rs - T121;
  for (int m = 1; m <= 3; ++m)
  {
    Cm *= r123;
    vec3 SmS = 2.0 * evaluateSensitivity(m * OPD, m * phi2.x);
    vec3 SmP = 2.0 * evaluateSensitivity(m * OPD, m * phi2.y);
    I += DepolColor(Cm.x * SmS, Cm.y * SmP);
  }

  // Convert back to RGB reflectance
  I = clamp(I * XYZ_TO_RGB, 0.0, 1.0);

  return I;
}

//***********************************************************************************************
//Function:iridescenceThickness unit is micrometer for this equation here. Mean 0.5 is 500nm.
vec3 CalculateIridescenceFresnel(float vThinFilmThickness, float vThinFilmIOR, float vThinFilmExtinctionK, float vClearCoatThickness, float vNDotV)
{
	float Dinc = vThinFilmThickness * 3.0f;
	float Eta0 = 1.0f;
	float Eta1 = mix(1.0f, BRDF_CLEAR_COAT_IOR, vClearCoatThickness);
	float Eta2 = mix(2.0f, 1.0f, vThinFilmThickness);
	float Eta3 = vThinFilmIOR;

	float CosTheta1 = sqrt(1.0f + sq(Eta0 / Eta1) * (sq(vNDotV) - 1.0f));
	float CosTheta2 = sqrt(1.0f - sq(Eta1 / Eta2) * (1.0f - sq(CosTheta1)));

	return CalculateFresnelAiry(CosTheta1, CosTheta2, Dinc, Eta1, Eta2, Eta3, vThinFilmExtinctionK);
}

//***********************************************************************************************
//Function:Main
void main()
{
	vec3 ModelDiffuseColor = ConvertSRGBtoLinear(texture(u_DiffuseTexture, v2f_TextureCoord).rgb);
	float ModelDiffuseAO = texture(u_AOTexture, v2f_TextureCoord).r;
	float ModelRoughness = texture(u_RoughnessTexture, v2f_TextureCoord).r;
	float ModelMetallic = texture(u_MetallicTexture, v2f_TextureCoord).r;

	vec3 Normal = CalculateNormalFromNormalMap();//normalize(v2f_Normal);
	vec3 ViewDir = normalize(u_CameraPos - v2f_FragPos);
	vec3 LightDir = normalize(u_LightInfo.Position - v2f_FragPos);
	float Distance = length(u_LightInfo.Position - v2f_FragPos);

	SBRDFParameterInfo BRDFParameterInfo;
	BRDFParameterInfo.ViewDir = ViewDir;
	BRDFParameterInfo.LightDir = LightDir;
	BRDFParameterInfo.Normal = CorrectNormal(Normal, BRDFParameterInfo.ViewDir);
	BRDFParameterInfo.F0 = vec3(0.04f);
	BRDFParameterInfo.AlbedoColor = ModelDiffuseColor;
	BRDFParameterInfo.Roughness = u_Roughness * CalculateRoughnessWithClearCoat(ModelRoughness, u_ClearCoatRoughness, u_ClearCoatThickness);
	BRDFParameterInfo.Metallic = u_Metallic * ModelMetallic;

	//Direct Light PBR
	vec3 ResultColor = ModelDiffuseAO * u_LightInfo.Intensity * CalculateBRDF(BRDFParameterInfo) * u_LightInfo.Color / (Distance * Distance);

	//Thin Film Layer
	float IridescenceThick = u_ThinFilmThickness * texture(u_IridescenceThickTexture, v2f_TextureCoord).r;
	vec3 IridescenceFresnel = vec3(0.0f);
	float IridescenceMask = CalculateIridescenceMask(u_ThinFilmIridescene);
	if(IridescenceMask > 0.0f)
		IridescenceFresnel = CalculateIridescenceFresnel(IridescenceThick, u_ThinFilmIOR, u_ThinFilmExtinctionK, u_ClearCoatThickness, max(dot(Normal, ViewDir), 0.0f));
	
	vec3 IridescenceAttenuation = vec3(1.0f) - IridescenceFresnel * IridescenceMask;
	
	//IBL Difuse
	//vec3 F0 = mix(BRDFParameterInfo.F0, OriginalColor, BRDFParameterInfo.Metallic);
	vec3 F0 = mix(vec3(0.16f * u_Reflectance * u_Reflectance), BRDFParameterInfo.AlbedoColor, BRDFParameterInfo.Metallic);
	vec3 Ks = CalculateF0WithClearCoat(F0, u_ClearCoatThickness);//FresnelSchlickRoughness(max(dot(Normal, BRDFParameterInfo.ViewDir), 0.0f), F0, BRDFParameterInfo.Roughness);
	vec3 Kd = vec3(1.0f) - Ks;
	Kd *= 1.0f - BRDFParameterInfo.Metallic;
	vec3 IBLDiffuseColor = ModelDiffuseAO * ModelDiffuseColor * Kd * texture(u_IrradianceMap, Normal).rgb * IridescenceAttenuation;

	float SpecularAO = CalculateSpecularAO(max(dot(BRDFParameterInfo.Normal, BRDFParameterInfo.ViewDir), 0.0f), ModelDiffuseAO, BRDFParameterInfo.Roughness);
	
	//IBL Specular
	const float MAX_REFLECTION_LOD = 4.0f;
	vec3 RelfectDir = reflect(-BRDFParameterInfo.ViewDir, Normal);
	vec3 PrefilteredEnvironmentColor = textureLod(u_EnvironmentMap, RelfectDir, BRDFParameterInfo.Roughness * MAX_REFLECTION_LOD).rgb;
	vec2 BRDFTerm = texture(u_BRDFTexture, vec2(max(dot(Normal, BRDFParameterInfo.ViewDir), 0.0f), BRDFParameterInfo.Roughness)).rg;
	vec3 IBLSpecularColor = SpecularAO * PrefilteredEnvironmentColor * mix((Ks * BRDFTerm.x + BRDFTerm.y), IridescenceFresnel, IridescenceMask);

	//Clear Coat: Clear Coat doesn't has diffuse
	vec3 CC_Normal = normalize(v2f_Normal);
	float CC_NormalDotViewDir = max(dot(CC_Normal, ViewDir), 0.0f);
	vec3 CC_ReflectDir = reflect(-ViewDir, CC_Normal);
	float CC_F0 = 0.04f;
	float CC_F = FresnelSchlick(CC_NormalDotViewDir, CC_F0) * u_ClearCoatThickness;
	float Attenuation = 1.0f - CC_F;

	IBLDiffuseColor *= Attenuation;
	IBLSpecularColor *= Attenuation * Attenuation;
	vec3 CC_PrefilteredEnvironmentColor = textureLod(u_EnvironmentMap, CC_ReflectDir, u_ClearCoatRoughness * MAX_REFLECTION_LOD).rgb;
	vec2 CC_BRDFTerm = texture(u_BRDFTexture, vec2(CC_NormalDotViewDir, u_ClearCoatRoughness)).rg;
	vec3 CC_IBLSpecularColor = SpecularAO * CC_PrefilteredEnvironmentColor * (CC_F);// * (CC_F * CC_BRDFTerm.x + CC_BRDFTerm.y);//
	IBLSpecularColor += CC_IBLSpecularColor;
	
	ResultColor += IBLDiffuseColor + IBLSpecularColor;

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + 1.0f);
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));
	//ResultColor = vec3(1);
	gl_FragColor = vec4(ResultColor, 1.0f);
}