#include "BloodFieldSettings.h"

#include "BloodPatternData.h"
#include "Engine/Texture2DArray.h"

#if WITH_EDITOR
void UBloodFieldSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetMemberPropertyName();
	if (PropertyName != GET_MEMBER_NAME_CHECKED(UBloodFieldSettings, BloodPatternData)) return;

	RebuildPatternTextureArray();
}
void UBloodFieldSettings::RebuildPatternTextureArray()
{
	UTexture2DArray* TextureArray = BloodPatternArray.LoadSynchronous();
	if (!TextureArray) return;

	TextureArray->Modify();
	TextureArray->SourceTextures.Reset();

	for (const TSoftObjectPtr<UBloodPatternData>& PatternPtr : BloodPatternData)
	{
		UBloodPatternData* Pattern = PatternPtr.LoadSynchronous();
		if (!Pattern || !Pattern->SourceTextures) continue;

		TextureArray->SourceTextures.Add(Pattern->SourceTextures);
	}

	TextureArray->UpdateSourceFromSourceTextures(false);
	TextureArray->UpdateResource();
	TextureArray->MarkPackageDirty();
}
#endif
