#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FBloodSelectCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBloodSelectCS);
	SHADER_USE_PARAMETER_STRUCT(FBloodSelectCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, Resolution)
		SHADER_PARAMETER(FVector3f, Scale)
		SHADER_PARAMETER(FVector3f, Origin)
		SHADER_PARAMETER(float, Radius)
		SHADER_PARAMETER(uint32, SplatCount)
		SHADER_PARAMETER(uint32, VoxelsPerSplat)
		SHADER_PARAMETER(FIntVector, VoxelRadius)
		SHADER_PARAMETER(FIntVector, Side)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FSplatGPUData>, SplatBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<FVector4f>, OutVolume)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<uint32>, ResolveTextureUAV)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(
		const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};
class FBloodFieldResolveCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBloodFieldResolveCS);
	SHADER_USE_PARAMETER_STRUCT(FBloodFieldResolveCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, Resolution)
		SHADER_PARAMETER(FVector3f, Scale)
		SHADER_PARAMETER(FVector3f, Origin)
		SHADER_PARAMETER(float, Radius)
		SHADER_PARAMETER(FVector2f, PatternWorldSize)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FSplatGPUData>, SplatBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FSplatGPUGroupData>, GroupBuffer)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture3D<float4>, OutVolume)
		SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture3D<uint>, ResolveTextureSRV)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(
		const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};