#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ImageCore.h"
#include "BloodFieldSettings.generated.h"

class UMaterialParameterCollection;
class UTextureRenderTargetVolume;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Blood Field"))
class BLOODFIELD_API UBloodFieldSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	TSoftObjectPtr<UMaterialParameterCollection> BloodFieldMPC;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	TSoftObjectPtr<UTextureRenderTargetVolume> BloodFieldRenderTarget;

	UPROPERTY(Config, EditAnywhere, Category = "Resources")
	TSoftObjectPtr<UTexture2D> BloodTexture;
};