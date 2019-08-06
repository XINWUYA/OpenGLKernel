#version 430 core

out vec4 gl_FragColor;

in vec2 v2f_TextureCoord;
in vec3 v2f_Normal;
in vec3 v2f_FragPos;

struct SLightInfo
{
	vec3 Direction;
	vec3 Color;
	float Intensity;
};

struct SBSDFData
{
	vec3 TopViewDir;
	vec3 BottomViewDir;
	vec3 TopLightDir;
	vec3 BottomLightDir;
	vec3 HalfVec;
	vec3 Normal;
	vec3 F0;
	vec3 BaseColor;
	float Roughness;
	float Metallic;
	float AO;

	float CoatRoughness;
	float CoatThickness;
	float CoatMask;
	float CoatIOR;
	vec3  CoatExtinctionColor;//消光颜色
	vec3  CoatNormal;
};

uniform SLightInfo u_LightInfo;
uniform sampler2D u_DiffuseTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_AOTexture;
uniform sampler2D u_MetallicTexture;
uniform vec3 u_CameraPos;
uniform float u_ModelRoughness;
uniform float u_ModelMetallic;
uniform float u_ClearCoatRoughness;
uniform float u_ClearCoatThickness;
uniform float u_ModelDielectricIOR = 2.416f;
uniform float u_ClearCoatIOR = 1.5f;
uniform vec3 u_ClearCoatTint;

const float PI = 3.14159265359f;
const float FLT_EPS = 5.960464478e-8f; // 2^-24, machine epsilon: 1 + EPS = 1 (half of the ULP for 1.0f)
const float INV_PI = 0.31830988618379067154f;
const float FLT_MIN = 1.175494351e-38f; // Minimum normalized positive floating-point number
const float FLT_MAX = 3.402823466e+38; // Maximum representable floating-point number

#define VLAYERED_DIFFUSE_ENERGY_HACKED_TERM

//***********************************************************************************************
//Function: 
vec3 ConvertSRGBtoLinear(vec3 vColor)
{
        return pow(vColor, vec3(2.2f));
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
float GetCoatEta(float vCoatIOR)
{
	return vCoatIOR / 1.0f;
}

//***********************************************************************************************
//Function:
float Sq(float x)
{
        return x * x;
}

vec2 Sq(vec2 x)
{
  return vec2(x.x * x.x, x.y * x.y);
}

vec3 Sq(vec3 x)
{
  return vec3(x.x * x.x, x.y * x.y, x.z * x.z);
}

//***********************************************************************************************
//Function:// Precompute part of lambdaV
float GetSmithJointGGXPartLambdaV(float NdotV, float roughness)
{
    float a2 = Sq(roughness);
    return sqrt((-NdotV * a2 + NdotV) * NdotV + a2);
}

//***********************************************************************************************
//Function:
float DV_SmithJointGGX(float NdotH, float NdotL, float NdotV, float roughness, float partLambdaV)
{
    float a2 = Sq(roughness);
    float s = (NdotH * a2 - NdotH) * NdotH + 1.0;

    float lambdaV = NdotL * partLambdaV;
    float lambdaL = NdotV * sqrt((-NdotL * a2 + NdotL) * NdotL + a2);

    vec2 D = vec2(a2, s * s);            // Fraction without the multiplier (1/Pi)
    vec2 G = vec2(1, lambdaV + lambdaL); // Fraction without the multiplier (1/2)

    // This function is only used for direct lighting.
    // If roughness is 0, the probability of hitting a punctual or directional light is also 0.
    // Therefore, we return 0. The most efficient way to do it is with a max().
    return INV_PI * 0.5 * (D.x * G.x) / max(D.y * G.y, FLT_MIN);
}

//***********************************************************************************************
//Function:
float CalculateEnergyCompensationFromSpecularReflectivity(float specularReflectivity)
{
    // Ref: Practical multiple scattering compensation for microfacet models.
    // We only apply the formulation for metals.
    // For dielectrics, the change of reflectance is negligible.
    // We deem the intensity difference of a couple of percent for high values of roughness
    // to not be worth the cost of another precomputed table.
    // Note: this formulation bakes the BSDF non-symmetric!

    // Note that using this factor for all specular lighting assumes all
    // BSDFs are from GGX.
    // (That's the FGD we use above to get integral[BSDF/F (N.w) dw] )

    // Make it roughly usable with a lerp factor with - 1.0, see ApplyEnergyCompensationToSpecularLighting()
    // The "lerp factor" will be fresnel0
    float energyCompensation = 1.0 / specularReflectivity - 1.0;
    return energyCompensation;
}

//***********************************************************************************************
//Function:// Use fresnel0 as a lerp factor for energy compensation (if 0, none applied)
vec3 ApplyEnergyCompensationToSpecularLighting(vec3 specularLighting, vec3 fresnel0, float energyCompensation)
{
    // Apply the fudge factor (boost) to compensate for multiple scattering not accounted for in the BSDF.
    // This assumes all spec comes from a GGX BSDF.
    specularLighting *= 1.0 + fresnel0 * energyCompensation;
    return specularLighting;
}

//***********************************************************************************************
//Function:
vec3 GetEnergyCompensationFactor(float specularReflectivity, vec3 fresnel0)
{
    float ec = CalculateEnergyCompensationFromSpecularReflectivity(specularReflectivity);
    return ApplyEnergyCompensationToSpecularLighting(vec3(1.0, 1.0, 1.0), fresnel0, ec);
}

//***********************************************************************************************
//Function:
float IorToFresnel0(float transmittedIor, float incidentIor)
{
	return Sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

vec3 IorToFresnel0(vec3 transmittedIor, vec3 incidentIor)
{
	return Sq((transmittedIor - incidentIor) / (transmittedIor + incidentIor));
}

float IorToFresnel0(float transmittedIor)
{
    return IorToFresnel0(transmittedIor, 1.0f);
}

vec3 IorToFresnel0(vec3 transmittedIor)
{
    return IorToFresnel0(transmittedIor, vec3(1.0f));
}

float Fresnel0ToIor(float fresnel0)
{
	return ((1.0 + sqrt(fresnel0)) / (1.0 - sqrt(fresnel0)));
}

vec3 Fresnel0ToIor(vec3 fresnel0)
{
	return ((1.0 + sqrt(fresnel0)) / (1.0 - sqrt(fresnel0)));
}

//***********************************************************************************************
//Function:
float mean(vec3 a) { return (a.x+a.y+a.z)/3.0; }

//***********************************************************************************************
//Function:
// Return the unpolarized version of the complete dielectric Fresnel equations
// from `FresnelDielectric` without accounting for wave phase shift.
// TODO: verify we have in BSDF lib
float FresnelUnpolarized(in float ct1, in float n1, in float n2)
{
    float cti = ct1;
    float st2 = (1.0 - Sq(cti));
    float nr  = n2/n1;
    if(nr == 1.0) { return 0.0; }

    if(Sq(nr)*st2 <= 1.0) {
        float ctt = sqrt(1.0 - Sq(nr)*st2) ;
        float tpp = (nr*cti-ctt) / (nr*cti + ctt);
        float tps = (cti-nr*ctt) / (nr*ctt + cti);
        return 0.5 * (tpp*tpp + tps*tps);
    } else {
        return 0.0;
    }
}

//***********************************************************************************************
//Function:
float ClampNdotV(float NdotV)
{
    return max(NdotV, 0.0001); // Approximately 0.0057 degree bias
}

//***********************************************************************************************
//Function:
// Linearized variance from roughness to be able to express an atomic
// adding operator on variance.
float RoughnessToLinearVariance(float a)
{
    a = clamp(a, 0.0, 0.9999);
    float a3 = pow(a, 1.1);
    return a3 / (1.0f - a3);
}

//***********************************************************************************************
//Function:
vec3 GetDirFromAngleAndOrthoFrame(vec3 V, vec3 N, float newVdotN)
{
    float sintheta = sqrt(1.0 - Sq(newVdotN));
    vec3 newV = newVdotN * N + sintheta * V;
    return newV;
}

//***********************************************************************************************
//Function:
// ----------------------------------------------------------------------------
// Helper for Disney parametrization
// ----------------------------------------------------------------------------
vec3 ComputeDiffuseColor(vec3 baseColor, float metallic)
{
    return baseColor * (1.0 - metallic);
}

//***********************************************************************************************
//Function:
vec3 ComputeFresnel0(vec3 baseColor, float metallic, vec3 dielectricF0)
{
    return mix(dielectricF0, baseColor, metallic);
}

//***********************************************************************************************
//Function:
vec3 ConvertF0ForAirInterfaceToF0ForNewTopIor(vec3 fresnel0, float newTopIor)
{
    vec3 ior = Fresnel0ToIor(min(fresnel0, vec3(1,1,1)*0.999)); // guard against 1.0
    return IorToFresnel0(ior, vec3(newTopIor));
}

//***********************************************************************************************
//Function:
float ClampRoughnessForAnalyticalLights(float roughness)
{
    return max(roughness, 1.0 / 1024.0);
}

//***********************************************************************************************
//Function:
float PerceptualRoughnessToRoughness(float perceptualRoughness)
{
    return perceptualRoughness * perceptualRoughness;
}

//***********************************************************************************************
//Function:
float RoughnessToPerceptualRoughness(float roughness)
{
    return sqrt(roughness);
}

//***********************************************************************************************
//Function:
float LinearVarianceToRoughness(float v)
{
    v = max(v, 0.0);
    float a = pow(v / (1.0 + v), 1.0/1.1);
    return a;
}

//***********************************************************************************************
//Function:
float LinearVarianceToPerceptualRoughness(float v)
{
    return RoughnessToPerceptualRoughness(LinearVarianceToRoughness(v));
}

//***********************************************************************************************
//Function:
void ConvertValueAnisotropyToValueTB(float value, float anisotropy, out float valueT, out float valueB)
{
    // Use the parametrization of Sony Imageworks.
    // Ref: Revisiting Physically Based Shading at Imageworks, p. 15.
    valueT = value * (1 + anisotropy);
    valueB = value * (1 - anisotropy);
}

//***********************************************************************************************
//Function:
void ConvertAnisotropyToRoughness(float perceptualRoughness, float anisotropy, out float roughnessT, out float roughnessB)
{
    float roughness = PerceptualRoughnessToRoughness(perceptualRoughness);
    ConvertValueAnisotropyToValueTB(roughness, anisotropy, roughnessT, roughnessB);
}

//***********************************************************************************************
//Function:
// WARNING: this has been deprecated, and should not be used!
// Same as ConvertAnisotropyToRoughness but
// roughnessT and roughnessB are clamped, and are meant to be used with punctual and directional lights.
void ConvertAnisotropyToClampRoughness(float perceptualRoughness, float anisotropy, out float roughnessT, out float roughnessB)
{
    ConvertAnisotropyToRoughness(perceptualRoughness, anisotropy, roughnessT, roughnessB);

    roughnessT = ClampRoughnessForAnalyticalLights(roughnessT);
    roughnessB = ClampRoughnessForAnalyticalLights(roughnessB);
}

//***********************************************************************************************
//Function:
vec3 F_Schlick(vec3 f0, float f90, float u)
{
    float x = 1.0 - u;
    float x2 = x * x;
    float x5 = x * x2 * x2;
    return f0 * (1.0 - x5) + (f90 * x5);        // sub mul mul mul sub mul mad*3
}

//***********************************************************************************************
//Function:
vec3 F_Schlick(vec3 f0, float u)
{
    return F_Schlick(f0, 1.0, u);               // sub mul mul mul sub mad*3
}

//***********************************************************************************************
//Function:
void ComputeStatistics(in float cti, in int i, in SBSDFData vBSDFData,
                       out float ctt,
                       out vec3 R12,   out vec3 T12,   out vec3 R21,   out vec3 T21,
                       out float  s_r12, out float  s_t12, out float  j12,
                       out float  s_r21, out float  s_t21, out float  j21)//#####################接口不同
{

    // Case of the dielectric coating
    if( i == 0 )
    {
        // Update energy
        float R0, n12;

        n12 = GetCoatEta(vBSDFData.CoatIOR); //n2/n1;//####################略微不同，功能一样
        R0  = FresnelUnpolarized(cti, n12, 1.0);

		// At this point cti should be properly (coatNormalWS dot V) or NdotV or VdotH, see ComputeAdding.
        // In the special case where we do have a coat normal, we will propagate a different angle than
        // (coatNormalWS dot V) and vOrthoGeomN will be used.
        // vOrthoGeomN is the orthogonal complement of V wrt geomNormalWS.
//        if (useGeomN)//#############################没用到，已注释
//        {
//            cti = ClampNdotV(dot(bsdfData.geomNormalWS, V));
//        }

        R12 = vec3(R0); // TODO: FGD//#####################受编译器影响
        T12 = 1.0 - R12;
        R21 = R12;
        T21 = T12;

        // Update mean
        float sti = sqrt(1.0 - Sq(cti));
        float stt = sti / n12;
        if( stt <= 1.0f )
        {
            // See p5 fig5 a) vs b) : using a refraction as a peak mean is the dotted line, while the ref is the solid line.
            // The scale is a hack to reproduce this effect: 
            // As roughness -> 1, remove the effect of changing angle of entry.
            // Note that we never track complete means per se because of symmetry, we have no azimuth, so the "space" of the
            // means sin(theta) (here sti and stt) is just a line perpendicular to the normal in the plane of incidence.
            // Moreover, we don't consider offspecular effect as well as never outputting final downward lobes anyway, so we
            // never output the means but just track them as cosines of angles (cti) for energy transfer calculations
            // (should do with FGD but depends, see comments above).
            const float alpha = vBSDFData.CoatRoughness;//#################################受接口影响
            const float scale = clamp((1.0-alpha)*(sqrt(1.0-alpha) + alpha), 0.0, 1.0);
            //http://www.wolframalpha.com/input/?i=f(alpha)+%3D+(1.0-alpha)*(sqrt(1.0-alpha)+%2B+alpha)+alpha+%3D+0+to+1
            stt = scale*stt + (1.0-scale)*sti;
            ctt = sqrt(1.0 - stt*stt);
        }
        else
        {
            // Even though we really track stats for a lobe, we decide on average we have TIR
            // (since we can't propagate anyway, we have no direction to use)
            // TODO: Right now, we block the UI from using coat IOR < 1.0, so can't happen.
            ctt = -1.0;
        }

        // Update variance
        s_r12 = RoughnessToLinearVariance(vBSDFData.CoatRoughness);//#################################受接口影响
        s_t12 = RoughnessToLinearVariance(vBSDFData.CoatRoughness * 0.5 * abs((ctt*n12 - cti)/(ctt*n12)));//#################################受接口影响
        j12   = (ctt/cti)*n12;

        s_r21 = s_r12;
        s_t21 = RoughnessToLinearVariance(vBSDFData.CoatRoughness * 0.5 * abs((cti/n12 - ctt)/(cti/n12)));//#################################受接口影响
        j21   = 1.0/j12;

    // Case of the media layer
    }
    else if(i == 1)
    {
        // TODO: if TIR is permitted by UI, do this, or early out
        //float stopFactor = float(cti > 0.0);
        //T12 = stopFactor * exp(- bsdfData.coatThickness * bsdfData.coatExtinction / cti);
        // Update energy
        R12 = vec3(0.0, 0.0, 0.0);//#####################受编译器影响
        T12 = exp(- vBSDFData.CoatThickness * vBSDFData.CoatExtinctionColor / cti);//#################################受接口影响
        R21 = R12;
        T21 = T12;

        // Update mean
        ctt = cti;

        // Update variance
        s_r12 = 0.0;
        s_t12 = 0.0;
        j12   = 1.0;

        s_r21 = 0.0;
        s_t21 = 0.0;
        j21   = 1.0;

    // Case of the dielectric / conductor base
    }
    else
    {
        float ctiForFGD = cti;

		// If we use the geometric normal propagation hack, we want to calculate FGD / Fresnel with
        // an angle at the bottom interface between the average propagated direction and the normal from
        // the bottom normal map. For that, we will recover a direction from the angle we propagated in
        // the "V and geomNormalWS" plane of incidence. That direction will then serve to calculate an
        // angle with the non-coplanar bottom normal from the normal map.
//        if (useGeomN)//#############################为用到
//        {
//            float3 bottomDir = GetDirFromAngleAndOrthoFrame(vOrthoGeomN, bsdfData.geomNormalWS, cti);
//            ctiForFGD = ClampNdotV(dot(bsdfData.normalWS, bottomDir));
//        }
        // We will also save this average bottom angle:
        float bottomAngleFGD = ctiForFGD;//##################不同

        // Update energy
        R12 = F_Schlick(vBSDFData.F0, ctiForFGD);
/*
        if (HasFlag(bsdfData.materialFeatures, MATERIALFEATUREFLAGS_STACK_LIT_IRIDESCENCE))//#########################未用到
        {
            if (vBSDFData.iridescenceMask > 0.0)
            {
                //float topIor = bsdfData.coatIor;
                // TODO:
                // We will avoid using coatIor directly as with the fake refraction, it can cause TIR
                // which even when handled in EvalIridescence (tested), doesn't look pleasing and
                // creates a discontinuity.
                float scale = clamp((1.0-vBSDFData.coatPerceptualRoughness), 0.0, 1.0);
                float topIor = lerp(1.0001, bsdfData.coatIor, scale);
                R12 = lerp(R12, EvalIridescence(topIor, ctiForFGD, vBSDFData.iridescenceThickness, vBSDFData.F0), vBSDFData.iridescenceMask);
            }
        }
*/
        T12 = vec3(0.0);
#ifdef VLAYERED_DIFFUSE_ENERGY_HACKED_TERM
        // Still should use FGD!
        // (note that although statistics T12 == T21 for the interface, the stack has terms e_T0i != e_Ti0)
        T12 = 1.0 - R12;
#endif
        R21 = R12;
        T21 = T12;

        // Update mean
        ctt = cti;

        // Update variance
        //
        // HACK: we will not propagate all needed last values, as we have 4,
        // but the adding cycle for the last layer can be shortcircuited for
        // the last lobes we need without computing the whole state of the
        // current stack (ie the i0 and 0i terms).
        //
        // We're only interested in _s_r0m and m_R0i.
        s_r12 = 0.0;
        //s_r12 = RoughnessToLinearVariance(bsdfData.roughnessAT);
        //s_r12_lobeB = RoughnessToLinearVariance(bsdfData.roughnessBT);
        // + anisotropic parts
        //

        s_t12 = 0.0;
        j12   = 1.0;

        s_r21 = s_r12;
        s_t21 = 0.0;
        j21   = 1.0;
    }

} //...ComputeStatistics()

//***********************************************************************************************
//Function://###################################只摘了中间一部分
void ComputeAdding(float _cti, SBSDFData vBSDFData, out vec3 voTopLayerEnergyCoeff, out vec3 voBottomLayerEnergyCoeff, out vec3 voDiffuseEnergy, out float voLayeredCoatRoughness, out float voLayeredRoughnessT)
{
	float  cti  = _cti;
    vec3 R0i = vec3(0.0, 0.0, 0.0), Ri0 = vec3(0.0, 0.0, 0.0),
         T0i = vec3(1.0, 1.0, 1.0), Ti0 = vec3(1.0, 1.0, 1.0);
    float  s_r0i=0.0, s_ri0=0.0, s_t0i=0.0, s_ti0=0.0;
    float  j0i=1.0, ji0=1.0;

    float _s_r0m, s_r12, m_rr; // we will need these outside the loop for further calculations

	vec3 localvLayerEnergyCoeff[3];
	int LayerNum = 3;
    // Iterate over the layers
    for(int i = 0; i < LayerNum; ++i)
    {
        // Variables for the adding step
        vec3 R12, T12, R21, T21;
        s_r12=0.0;
        float s_r21=0.0, s_t12=0.0, s_t21=0.0, j12=1.0, j21=1.0, ctt;

        // Layer specific evaluation of the transmittance, reflectance, variance
        ComputeStatistics(cti, i, vBSDFData, ctt, R12, T12, R21, T21, s_r12, s_t12, j12, s_r21, s_t21, j21);

        // Multiple scattering forms
        vec3 denom = (vec3(1.0, 1.0, 1.0) - Ri0*R12); //i = new layer, 0 = cumulative top (llab3.1 to 3.4)
        vec3 m_R0i = (mean(denom) <= 0.0f)? vec3(0.0, 0.0, 0.0) : (T0i*R12*Ti0) / denom; //(llab3.1)
        vec3 m_Ri0 = (mean(denom) <= 0.0f)? vec3(0.0, 0.0, 0.0) : (T21*Ri0*T12) / denom; //(llab3.2)
        vec3 m_Rr  = (mean(denom) <= 0.0f)? vec3(0.0, 0.0, 0.0) : (Ri0*R12) / denom;
        float  m_r0i = mean(m_R0i);
        float  m_ri0 = mean(m_Ri0);
        m_rr  = mean(m_Rr);

        // Evaluate the adding operator on the energy
        vec3 e_R0i = R0i + m_R0i; //(llab3.1)
        vec3 e_T0i = (T0i*T12) / denom; //(llab3.3)
        vec3 e_Ri0 = R21 + (T21*Ri0*T12) / denom; //(llab3.2)
        vec3 e_Ti0 = (T21*Ti0) / denom; //(llab3.4)

        // Scalar forms for the energy
        float r21   = mean(R21);
        float r0i   = mean(R0i);
        float e_r0i = mean(e_R0i);
        float e_ri0 = mean(e_Ri0);

        // Evaluate the adding operator on the normalized variance
        _s_r0m = s_ti0 + j0i*(s_t0i + s_r12 + m_rr*(s_r12+s_ri0));
        float _s_r0i = (r0i*s_r0i + m_r0i*_s_r0m) / e_r0i;
        float _s_t0i = j12*s_t0i + s_t12 + j12*(s_r12 + s_ri0)*m_rr;
        float _s_rim = s_t12 + j12*(s_t21 + s_ri0 + m_rr*(s_r12+s_ri0));
        float _s_ri0 = (r21*s_r21 + m_ri0*_s_rim) / e_ri0;
        float _s_ti0 = ji0*s_t21 + s_ti0 + ji0*(s_r12 + s_ri0)*m_rr;
        _s_r0i = (e_r0i > 0.0) ? _s_r0i/e_r0i : 0.0;
        _s_ri0 = (e_ri0 > 0.0) ? _s_ri0/e_ri0 : 0.0;

        // Store the coefficient and variance
        localvLayerEnergyCoeff[i] = (m_r0i > 0.0) ? m_R0i : vec3(0.0, 0.0, 0.0);
        //preLightData.vLayerPerceptualRoughness[i] = (m_r0i > 0.0) ? LinearVarianceToPerceptualRoughness(_s_r0m) : 0.0;

        // Update energy
        R0i = e_R0i;
        T0i = e_T0i;
        Ri0 = e_Ri0;
        Ti0 = e_Ti0; // upward transmittance: we need this fully computed "past" the last layer see below for diffuse


        // Update mean
        cti = ctt;

        // We need to escape this update on the last vlayer iteration,
        // as we will use a hack to compute all needed bottom layer
        // anisotropic roughnesses. The compiler should easily factor
        // this out when the loop is unrolled anyway
        if( i < LayerNum - 1 )
        {
            // Update variance
            s_r0i = _s_r0i;
            s_t0i = _s_t0i;
            s_ri0 = _s_ri0;
            s_ti0 = _s_ti0;

            // Update jacobian
            j0i *= j12;
            ji0 *= j21;
        }
    }

	voTopLayerEnergyCoeff = localvLayerEnergyCoeff[0];
	voBottomLayerEnergyCoeff = localvLayerEnergyCoeff[2];
	voDiffuseEnergy = Ti0;

	voLayeredCoatRoughness = ClampRoughnessForAnalyticalLights(vBSDFData.CoatRoughness);
	s_r12 = RoughnessToLinearVariance(PerceptualRoughnessToRoughness(vBSDFData.Roughness));
    _s_r0m = s_ti0 + j0i*(s_t0i + s_r12 + m_rr*(s_r12+s_ri0));
    float BaseIBLPerceptualRoughness = LinearVarianceToPerceptualRoughness(_s_r0m);
	float LayeredRoughnessT, LayeredRoughnessB;
	ConvertAnisotropyToClampRoughness(BaseIBLPerceptualRoughness, 0.0f, LayeredRoughnessT, LayeredRoughnessB);

	voLayeredRoughnessT = LayeredRoughnessT;
}

//***********************************************************************************************
//Function: Main
void main()
{
	vec3 AlbedoColor = ConvertSRGBtoLinear(texture(u_DiffuseTexture, v2f_TextureCoord).rgb);
	float AO = texture(u_AOTexture, v2f_TextureCoord).r;
	float AmbientWeight = 0.03f;
	vec3 AmbientColor = AO * AmbientWeight * AlbedoColor;

	vec3 Normal = CalculateNormalFromNormalMap();
	float ModelMetallic = u_ModelMetallic * texture(u_MetallicTexture, v2f_TextureCoord).r;
	
	vec3 ResultColor = vec3(0.0f);

	//BSDF Data
	SBSDFData BSDFData;
	BSDFData.TopViewDir = normalize(u_CameraPos - v2f_FragPos);
	BSDFData.TopLightDir = normalize(u_LightInfo.Direction);
	BSDFData.BottomViewDir = BSDFData.TopViewDir;//未使用折射
	BSDFData.BottomLightDir = BSDFData.TopLightDir;//未使用折射
	BSDFData.HalfVec = normalize(BSDFData.TopViewDir + BSDFData.TopLightDir);
	BSDFData.Normal = Normal;//CorrectNormal(Normal, BSDFData.TopViewDir);
	BSDFData.BaseColor = AlbedoColor;
	BSDFData.Roughness = u_ModelRoughness;
	BSDFData.Metallic = ModelMetallic;
	BSDFData.F0 = mix(vec3(IorToFresnel0(u_ModelDielectricIOR)), BSDFData.BaseColor, BSDFData.Metallic);
	BSDFData.AO = AO;
	BSDFData.CoatRoughness = u_ClearCoatRoughness * u_ClearCoatRoughness;
	BSDFData.CoatThickness = u_ClearCoatThickness;
	BSDFData.CoatMask = 0.0f;
	BSDFData.CoatIOR = u_ClearCoatIOR;
	BSDFData.CoatExtinctionColor = u_ClearCoatTint;
	BSDFData.CoatNormal = normalize(v2f_Normal);

	//if(u_ClearCoatThickness > 0.0f)
		BSDFData.F0 = ConvertF0ForAirInterfaceToF0ForNewTopIor(BSDFData.F0, BSDFData.CoatIOR);
	
	
	float BaseSpecularReflectivity = 1.0f;//和IBL有关,為1時沒有能量補償
	float CoatSpecularReflectivity = 1.0f;
	
	vec3 BaseLobeEnergyCompensationFactor = GetEnergyCompensationFactor(BaseSpecularReflectivity, BSDFData.F0);
	vec3 CoatLobeEnergyCompensationFactor = GetEnergyCompensationFactor(CoatSpecularReflectivity, vec3(IorToFresnel0(BSDFData.CoatIOR)));

	float BaseNDotL = dot(BSDFData.Normal, BSDFData.BottomLightDir);
	float UnclampedBaseNDotV = dot(BSDFData.Normal, BSDFData.BottomViewDir);
	float BaseNDotV = clamp(UnclampedBaseNDotV, 0.0f, 1.0f);	

	float CoatNDotL = dot(BSDFData.CoatNormal, BSDFData.TopLightDir);
	float UnclampedCoatNDotV = dot(BSDFData.CoatNormal, BSDFData.TopViewDir);
	float CoatNDotV = clamp(UnclampedCoatNDotV, 0.0f, 1.0f);


	float LDotV = dot(BSDFData.TopLightDir, BSDFData.TopViewDir);
	float InvLenLV = 1.0f / sqrt(max(2.0f * LDotV + 2.0f, FLT_EPS));
	vec3 BottomLayerEnergyCoeff = vec3(1.0f);
	vec3 TopLayerEnergyCoeff = vec3(1.0f);
	vec3 DiffuseEnergy = vec3(1.0f);
	float LayeredCoatRoughness = BSDFData.Roughness;
	float LayeredRoughnessT = BSDFData.CoatRoughness;
	ComputeAdding(CoatNDotV, BSDFData, TopLayerEnergyCoeff, BottomLayerEnergyCoeff, DiffuseEnergy, LayeredCoatRoughness, LayeredRoughnessT);

	float BaseLambdaV = GetSmithJointGGXPartLambdaV(BaseNDotV, LayeredRoughnessT);
	float CoatPartLambdaV = GetSmithJointGGXPartLambdaV(CoatNDotV, LayeredCoatRoughness);

	float CoatNDotH = clamp((CoatNDotL + UnclampedCoatNDotV) * InvLenLV, 0.0f, 1.0f);
	float BaseNDotH = clamp((dot(BSDFData.Normal, BSDFData.TopLightDir) + UnclampedBaseNDotV) * InvLenLV, 0.0f, 1.0f);

	float BaseLayerDV = DV_SmithJointGGX(BaseNDotH, BaseNDotL, BaseNDotV, LayeredRoughnessT, BaseLambdaV);
	float CoatLayerDV = DV_SmithJointGGX(CoatNDotH, CoatNDotL, CoatNDotV, LayeredCoatRoughness, CoatPartLambdaV);

	vec3 SpecularLighting = max(vec3(0.0f), BaseNDotL) * BottomLayerEnergyCoeff * BaseLayerDV * BaseLobeEnergyCompensationFactor;
	vec3 CoatSpecularLighting = max(vec3(0.0f), CoatNDotL) * TopLayerEnergyCoeff * CoatLayerDV * CoatLobeEnergyCompensationFactor;
	SpecularLighting += CoatSpecularLighting;

	vec3 DiffuseColor = DiffuseEnergy * ComputeDiffuseColor(BSDFData.BaseColor, BSDFData.Metallic);
	vec3 DiffuseLighting = AO * DiffuseColor * max(0.0f, BaseNDotL) * INV_PI;

	ResultColor = (DiffuseLighting + SpecularLighting) * u_LightInfo.Intensity * u_LightInfo.Color;

	//Gamma Correction
	ResultColor = ResultColor / (ResultColor + vec3(1.0f));
	ResultColor = pow(ResultColor, vec3(1.0f / 2.2f));

	gl_FragColor = vec4(ResultColor, 1.0f);
}