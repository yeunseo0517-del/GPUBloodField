#pragma once

#include "CoreMinimal.h"

class BLOODFIELDSHADERS_API FBloodFieldShaderInterface
{
public:
	static void Dispatch(FTextureRenderTargetResource* TargetResource, uint32 InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius);

private:
	static void AddBloodFieldPass(FRDGBuilder& GraphBuilder, const FGlobalShaderMap* InShaderMap, uint32 InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius, FRDGTextureRef VolumeTexture);
};