#pragma once

#include "CoreMinimal.h"
#include "BloodPatternEditorLibrary.generated.h"

UCLASS()
class BLOODFIELDEDITOR_API UBloodPatternEditorLibrary
	: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "BloodField")
	static TArray<FVector2D> AnalyzeBloodPatternTexture(UTexture2D* Texture);
};