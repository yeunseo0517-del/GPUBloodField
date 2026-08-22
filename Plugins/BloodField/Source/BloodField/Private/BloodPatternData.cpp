#include "BloodPatternData.h"
#include "BloodFieldSettings.h"

void UBloodPatternData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.GetPropertyName();
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UBloodPatternData, SourceTextures))
	{
		UBloodFieldSettings* Setting = GetMutableDefault<UBloodFieldSettings>();

		Setting->RebuildPatternTextureArray();
	}
}