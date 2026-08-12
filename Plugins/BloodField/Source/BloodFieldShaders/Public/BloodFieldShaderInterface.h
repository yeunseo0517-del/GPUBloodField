#pragma once

#include "CoreMinimal.h"

class BLOODFIELDSHADERS_API FBloodFieldShaderInterface
{
public:
	static void Dispatch(FTextureRenderTargetResource* TargetResource, const FIntVector& InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius);

private:
	static void AddBloodFieldPass(FRDGBuilder& GraphBuilder, const FGlobalShaderMap* InShaderMap, const FIntVector& InResolution, const FVector3f& InScale, const FVector3f& InOrigin, const FVector3f& InLocation, float InRadius, FRDGTextureRef VolumeTexture);
};