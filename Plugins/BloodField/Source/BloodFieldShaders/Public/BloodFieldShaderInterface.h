#pragma once

#include "CoreMinimal.h"

class BLOODFIELDSHADERS_API FBloodFieldShaderInterface
{
public:
	static void Dispatch(FTextureRenderTargetResource* TargetResource, const struct FBloodFieldPassInput& Input);

private:
	static void AddBloodFieldPass(FRDGBuilder& GraphBuilder, FRDGTextureRef VolumeTexture, const FGlobalShaderMap* InShaderMap, const struct FBloodFieldPassInput& Input);
};