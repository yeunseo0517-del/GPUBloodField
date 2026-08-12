#pragma once

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"

class FBloodFieldCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FBloodFieldCS);
	SHADER_USE_PARAMETER_STRUCT(FBloodFieldCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FIntVector, Resolution)
		SHADER_PARAMETER(FVector3f, Scale)
		SHADER_PARAMETER(FVector3f, Origin)
		SHADER_PARAMETER(FVector3f, Location)
		SHADER_PARAMETER(float, Radius)

		SHADER_PARAMETER_RDG_TEXTURE_UAV(
			RWTexture3D<float>,
			OutVolume
		)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(
		const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};