#pragma once

#include "CoreMinimal.h"
#include "BloodBurstRequest.generated.h"

USTRUCT(BlueprintType)
struct FBloodBurstRequest
{
	GENERATED_BODY();
	
	UPROPERTY()
	FVector ImpactNormal = FVector::ZeroVector;

	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY()
	FVector Direction = FVector::ForwardVector;

	UPROPERTY()
	float Radius = 50.f;

	UPROPERTY()
	float Strength = 1.f;
};

USTRUCT(BlueprintType)
struct FBloodSplat
{
	GENERATED_BODY();
	
	UPROPERTY()
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY()
	float Radius = 0.5f;

	UPROPERTY()
	float Strength = 1.f;
};