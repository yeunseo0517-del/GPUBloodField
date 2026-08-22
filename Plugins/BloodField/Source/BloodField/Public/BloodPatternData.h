#pragma once

#include "CoreMinimal.h"
#include "BloodPatternData.generated.h"

UCLASS(BlueprintType)
class BLOODFIELD_API UBloodPatternData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UTexture2D> SourceTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> SampleUVs;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};