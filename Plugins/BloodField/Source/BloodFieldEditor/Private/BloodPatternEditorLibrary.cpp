#include "BloodPatternEditorLibrary.h"

TArray<FVector2D> UBloodPatternEditorLibrary::AnalyzeBloodPatternTexture(UTexture2D* Texture)
{
	if (!IsValid(Texture)) return TArray<FVector2D>();
	TArray<FVector2D> Result;
	uint8 Cellsize = 25;
	FImage Image;
	Texture->Source.GetMipImage(Image, 0);
	const FColor* Pixels = reinterpret_cast<const FColor*>(Image.RawData.GetData());
	uint8 Threshold = 0;

	uint8 Width = Image.SizeX / Cellsize;
	uint8 Height = Image.SizeY / Cellsize;
	//UE_LOG(LogTemp, Warning, TEXT("Image: %d x %d, Width: %d, Height: %d"), Image.SizeX, Image.SizeY, Width, Height);
	for (int i = 0; i < Cellsize; ++i)
	{
		int ValidPixels = 0;
		for (int y = i * Height; y < i * Height + Height; ++y)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Round %d %d"), i * Height, i * Width);
			for (int x = i * Width; x < i * Width + Width; ++x)
			{
				FColor Pixel = Pixels[y * Image.SizeX + x];
				//UE_LOG(LogTemp, Warning, TEXT("x=%d y=%d index=%lld"), x, y, y * Image.SizeX + x);
				if (Pixel.A > Threshold) ++ValidPixels;
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("ValidPixels: %d"), ValidPixels);
		if (ValidPixels < 100) continue;
		double UVX = (i * Width + Width / 2) / float(Image.SizeX);
		double UVY = (i * Height + Height / 2) / float(Image.SizeY);
		Result.AddUnique({ UVX, UVY });
	}
	return Result;
}