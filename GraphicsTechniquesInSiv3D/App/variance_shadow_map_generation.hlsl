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

// Shader for variance shadow map generation
// Based on default3d_forward.hlsl

// References：
//	 M. Fisher, "Matt's Webcorner - Variance Shadow Maps", https://graphics.stanford.edu/~mdfisher/Shadows.html (accessed: Dec. 12, 2021)
//	 A. Lauritzen, "Chapter 8. Summed-Area Variance Shadow Maps", in GPU Gems 3, https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-8-summed-area-variance-shadow-maps (accessed: Dec. 12, 2021)

//
//	Textures
//
Texture2D		g_texture0 : register(t0);
SamplerState	g_sampler0 : register(s0);

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
	return result;
}

float4 PS(s3d::PSInput input) : SV_TARGET
{
	// Write depth and squared depth
	float depth = input.position.z;
	return float4(depth, depth * depth, 0.0, 1.0);
}
