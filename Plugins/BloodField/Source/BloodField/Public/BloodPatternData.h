#pragma once

#include "CoreMinimal.h"
#include "BloodPatternData.generated.h"

UCLASS(BlueprintType)
class UBloodPatternData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UTexture2D> SourceTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> SampleUVs;
};