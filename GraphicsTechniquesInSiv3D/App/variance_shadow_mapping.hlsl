//-----------------------------------------------
//
//	This file is part of the Siv3D Engine.
//
//	Copyright (c) 2008-2021 Ryo Suzuki
//	Copyright (c) 2016-2021 OpenSiv3D Project
//
//	Licensed under the MIT License.
//
//-----------------------------------------------

// Soft shadow using Variance Shadow Map
// Based on default3d_forward.hlsl

// References：
//	 M. Fisher, "Matt's Webcorner - Variance Shadow Maps", https://graphics.stanford.edu/~mdfisher/Shadows.html (accessed: Dec. 12, 2021)
//	 A. Lauritzen, "Chapter 8. Summed-Area Variance Shadow Maps", in GPU Gems 3, https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps (accessed: Dec. 12, 2021)

//
//	Textures
//
Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);
Texture2D<float2>	g_shadowMapTexture : register(t1);
SamplerState	g_shadowMapSampler : register(s1);

namespace s3d
{
	//
	//	VS Input
	//
	struct VSInput
	{
		float4 position : POSITION;
		float3 normal : NORMAL;
		float2 uv : TEXCOORD0;
	};

	//
	//	VS Output / PS Input
	//
	struct PSInput
	{
		float4 position : SV_POSITION;
		float3 worldPosition : TEXCOORD0;
		float2 uv : TEXCOORD1;
		float3 normal : TEXCOORD2;
		float4 positionFromSun : TEXCOORD3;
	};
}

//
//	Constant Buffer
//
cbuffer VSPerView : register(b1)
{
	row_major float4x4 g_worldToProjected;
}

cbuffer VSPerObject : register(b2)
{
	row_major float4x4 g_localToWorld;
}

// Constant buffer for varaince shadow mapping
cbuffer VSVarianceShadowMap : register(b4)
{
	row_major float4x4 g_sunCameraMatrix; // View/Projection matrix of the sun as a camera
}

cbuffer PSPerFrame : register(b0)
{
	float3 g_gloablAmbientColor;
	float3 g_sunColor;
	float3 g_sunDirection;
}

cbuffer PSPerView : register(b1)
{
	float3 g_eyePosition;
}

cbuffer PSPerMaterial : register(b3)
{
	float3 g_amibientColor;
	uint   g_hasTexture;
	float4 g_diffuseColor;
	float3 g_specularColor;
	float  g_shininess;
	float3 g_emissionColor;
}

//
//	Functions
//
s3d::PSInput VS(s3d::VSInput input)
{
	s3d::PSInput result;

	const float4 worldPosition = mul(input.position, g_localToWorld);

	result.position			= mul(worldPosition, g_worldToProjected);
	result.worldPosition	= worldPosition.xyz;
	result.uv				= input.uv;
	result.normal			= mul(input.normal, (float3x3)g_localToWorld);
	result.positionFromSun	= mul(worldPosition, g_sunCameraMatrix);
	return result;
}

float4 GetDiffuseColor(float2 uv)
{
	float4 diffuseColor = g_diffuseColor;

	if (g_hasTexture)
	{
		diffuseColor *= g_texture0.Sample(g_sampler0, uv);
	}

	return diffuseColor;
}

float3 CalculateDiffuseReflection(float3 n, float3 l, float3 lightColor, float3 diffuseColor, float3 ambientColor)
{
	const float3 directColor = lightColor * saturate(dot(n, l));
	return ((ambientColor + directColor) * diffuseColor);
}

float3 CalculateSpecularReflection(float3 n, float3 h, float shininess, float nl, float3 lightColor, float3 specularColor)
{
	const float highlight = pow(saturate(dot(n, h)), shininess) * float(0.0 < nl);
	return (lightColor * specularColor * highlight);
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
	// Calculate UV of shadow map
	const float2 shadowMapUV = float2(
		0.5 * input.positionFromSun.x / input.positionFromSun.w + 0.5,
		-0.5 * input.positionFromSun.y / input.positionFromSun.w + 0.5);

	float shadowProbMax;
	if (all(shadowMapUV >= 0.0 && shadowMapUV <= 1.0))
	{
		// Look up from shadow map
		const float2 shadowMapValue = g_shadowMapTexture.Sample(g_shadowMapSampler, shadowMapUV);
		const float shadowMapDepth = shadowMapValue.r;
		const float shadowMapDepthSquared = shadowMapValue.g;
		// Calculate probability of being occuluded from the sun using Chebyshev's inequality
		// Note: strictly speaking, the inequality used here is Cantelli's inequality, which is one-sided version of Chebyshev's inequality.
		//	(See: https://en.wikipedia.org/wiki/Cantelli%27s_inequality )
		const float shadowMapDepthVariance = max(shadowMapDepthSquared - shadowMapDepth * shadowMapDepth, 0.000001);  // Clamping reduces noise
		const float differenceFromMean = input.positionFromSun.z - shadowMapDepth;
		shadowProbMax = shadowMapDepthVariance / (shadowMapDepthVariance + differenceFromMean * differenceFromMean);
	}
	else
	{
		shadowProbMax = 1.0;
	}

	const float3 lightColor = g_sunColor * shadowProbMax; // Reduce light intensity according to probability calculated above
	const float3 lightDirection = g_sunDirection;

	const float3 n = normalize(input.normal);
	const float3 l = lightDirection;
	const float4 diffuseColor = GetDiffuseColor(input.uv);
	const float3 ambientColor = (g_amibientColor * g_gloablAmbientColor);

	// Diffuse
	const float3 diffuseReflection = CalculateDiffuseReflection(n, l, lightColor, diffuseColor.rgb, ambientColor);

	// Specular
	const float3 v = normalize(g_eyePosition - input.worldPosition);
	const float3 h = normalize(v + lightDirection);
	const float3 specularReflection = CalculateSpecularReflection(n, h, g_shininess, dot(n, l), lightColor, g_specularColor);

	return float4(diffuseReflection + specularReflection + g_emissionColor, diffuseColor.a);
}
