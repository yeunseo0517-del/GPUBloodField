#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ImageCore.h"
#include "BloodPatternData.h"
#include "BloodPatternSettings.h"
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
	TArray<TSoftObjectPtr<UBloodPatternData>> BloodPatternData;

	UPROPERTY(Config, EditAnywhere, Category = "Blood Pattern")
	TSoftObjectPtr<UTexture2DArray> BloodPatternArray;

	// Pattern Analysis
	UPROPERTY(EditAnywhere, Config, Category = "Pattern")
	FBloodPatternSettings BloodPatternSettings;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	void RebuildPatternTextureArray();
#endif
};