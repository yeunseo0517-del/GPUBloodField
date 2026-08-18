#include "BloodPatternEditorLibrary.h"

TArray<FVector2D> UBloodPatternEditorLibrary::AnalyzeBloodPatternTexture(UTexture2D* Texture)
{
	if (!IsValid(Texture)) return TArray<FVector2D>();

	TArray<FVector2D> Result;
	FImage Image;
	Texture->Source.GetMipImage(Image, 0);
	const FColor* Pixels = reinterpret_cast<const FColor*>(Image.RawData.GetData());

	uint8 GridSize = 5;
	uint8 Threshold = 32;
	TArray<int> ValidPixels;
	ValidPixels.SetNumZeroed(GridSize * GridSize);

	for (int y = 0; y < Image.SizeY; ++y)
	{
		for (int x = 0; x < Image.SizeX; ++x)
		{
			FColor Pixel = Pixels[y * Image.SizeX + x];

			if (Pixel.A > Threshold)
			{
				const int SampleX = x * GridSize / Image.SizeX;
				const int SampleY = y * GridSize / Image.SizeY;

				const int SampleIndex = SampleY * GridSize + SampleX;

				++ValidPixels[SampleIndex];
			}
		}
	}
	
	const float CellPixelCount = (Image.SizeX / GridSize) * (Image.SizeY / GridSize);
	for (int i = 0; i < ValidPixels.Num(); ++i)
	{
		const float Coverage = static_cast<float>(ValidPixels[i]) / CellPixelCount;
		if (Coverage < 0.03f) continue;
		const double UVX = (i % GridSize + 0.5f) / GridSize;
		const double UVY = (i / GridSize + 0.5f) / GridSize;
		Result.AddUnique({ UVX, UVY });
	}
	return Result;
}