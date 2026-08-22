#pragma once

#include "CoreMinimal.h"
#include "BloodFieldShaderTypes.h"
#include "BloodBurstRequest.generated.h"

USTRUCT(BlueprintType)
struct FBloodBurstRequest
{
	GENERATED_BODY()
	
	UPROPERTY()
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector Direction = FVector::ForwardVector;

	UPROPERTY()
	float Radius = 50.f;
};

USTRUCT()
struct FBloodSplatGroup
{
	GENERATED_BODY()

	TArray<FSplatGPUData> BloodSplats;
	FSplatGPUGroupData SplatGroupData;
};