#pragma once

#include "CoreMinimal.h"
#include "BloodPatternSettings.generated.h"

USTRUCT(BlueprintType)
struct FBloodPatternSettings
{
    GENERATED_BODY()

	// Pattern Analysis
	UPROPERTY(EditAnywhere, Config, Category = "Pattern Analysis")
	int32 GridSize = 5;

	// Splat
	UPROPERTY(EditAnywhere, Config, Category = "Splat")
	float SplatRadius = 5.f;
};