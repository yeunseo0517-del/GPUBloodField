#pragma once

#include "CoreMinimal.h"
#include "BloodBurstRequest.generated.h"

USTRUCT(BlueprintType)
struct FBloodBurstRequest
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector Direction = FVector::ForwardVector;

	UPROPERTY(EditAnywhere)
	float Radius = 50.f;

	UPROPERTY(EditAnywhere)
	float Strength = 1.f;
};