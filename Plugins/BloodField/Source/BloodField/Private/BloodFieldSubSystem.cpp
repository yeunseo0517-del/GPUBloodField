// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodFieldSubSystem.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "BloodFieldShaderInterface.h"

namespace
{
	constexpr int32 PatternMask[5][5] =
	{
		{ 0, 1, 0, 0, 0 },
		{ 1, 1, 1, 0, 0 },
		{ 0, 1, 1, 1, 1 },
		{ 0, 0, 1, 0, 0 }
	};

	constexpr int32 offset = 0.5;
}

void UBloodFieldSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BloodFieldTarget = NewObject<UTextureRenderTargetVolume>(this, TEXT("BloodFieldTarget"));

	if (!BloodFieldTarget) return;

	BloodFieldTarget->bCanCreateUAV = true;
	// 혈흔 값은 선형 데이터
	BloodFieldTarget->bForceLinearGamma = true;
	// 초기값 0
	BloodFieldTarget->ClearColor = FLinearColor::Black;
	// 128 × 128 × 128, 복셀당 float 하나
	BloodFieldTarget->Init(
		Resolution,
		Resolution,
		Resolution,
		PF_R32_FLOAT);
	// 리소스를 즉시 갱신하고 검은색으로 클리어
	BloodFieldTarget->UpdateResourceImmediate(true);
}

void UBloodFieldSubSystem::Deinitialize()
{
	BloodFieldTarget = nullptr;
	Super::Deinitialize();
}

bool UBloodFieldSubSystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

TStatId UBloodFieldSubSystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(
		UBloodFieldSubsystem,
		STATGROUP_Tickables
	);
}

void UBloodFieldSubSystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldFlushRequests)
	{
		++FramesSinceLastFlush;

		if (FramesSinceLastFlush >= FlushIntervalFrames)
		{
			FlushRequests();
			FramesSinceLastFlush = 0;
		}
	}
}

void UBloodFieldSubSystem::RequestBloodSplat(FBloodBurstRequest Request)
{
	if (!bShouldFlushRequests) bShouldFlushRequests = true;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (PatternMask[i][j] == 1)
			{
				BloodSplats.Add(CalculateSplatLocation(Request, i, j));
			}
		}
	}
}

void UBloodFieldSubSystem::SetFieldOrigin(const FVector3f& InOrigin)
{
	FieldOrigin = InOrigin;
}

FBloodSplat UBloodFieldSubSystem::CalculateSplatLocation(const FBloodBurstRequest& Request, int x, int y)
{
	FBloodSplat Splat;

	FVector Normal = Request.ImpactNormal.GetSafeNormal();
	FVector RefN = FVector::UpVector;
	if (abs(FVector::DotProduct(Normal, RefN) > 0.99)) RefN = FVector::ForwardVector;
	FVector Tangent = FVector::CrossProduct(Normal, RefN).GetSafeNormal();
	FVector Bitangent = FVector::CrossProduct(Normal, Tangent).GetSafeNormal();

	FVector Location = Request.WorldLocation + Tangent * (offset * (x - 2)) + Bitangent * (offset * (y - 2));

	Splat.WorldLocation = Location;
	Splat.Radius = Request.Radius;
	Splat.Strength = Request.Strength;

	return Splat;
}

void UBloodFieldSubSystem::FlushRequests()
{
	if (!BloodFieldTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Blood Field Target"));
		return;
	}

	FTextureRenderTargetResource* RT = BloodFieldTarget->GameThread_GetRenderTargetResource();

	if (!RT)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Get Render Target Resource from Game Thread"));
		return;
	}

	for (const auto& Splat : BloodSplats)
	{
		FBloodFieldShaderInterface::Dispatch(RT, static_cast<uint32>(Resolution), FieldScale, FieldOrigin, FVector3f(Splat.WorldLocation), Splat.Radius);
	}

	BloodSplats.Empty();
	bShouldFlushRequests = false;
}

