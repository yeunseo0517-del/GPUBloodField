#pragma once

#include "RenderGraphResources.h"

struct FSplatGPUData
{
	FVector3f Location;
	FVector2f SampleUV;
	uint32 GroupIndex;
	FVector3f Tangent;
	FVector3f Bitangent;
};

struct FSplatGPUGroupData
{
	uint32 PatternID;
};

struct FBloodFieldPassInput
{
	FIntVector Resolution;
	FVector3f Scale;
	FVector3f Origin;
	float Radius;
	FVector2f PatternWorldSize;
	TArray<FSplatGPUData> SplatsData;
	TArray<FSplatGPUGroupData> GroupData;
	FRHITexture* BloodTextures;
};