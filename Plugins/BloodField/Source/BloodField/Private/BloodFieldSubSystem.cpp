// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodFieldSubSystem.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "BloodBurstRequest.h"
#include "BloodFieldShaderInterface.h"

//UTextureRenderTargetVolume 참조 보관
//Resolution, Scale, Origin
//혈흔이 생긴 Location, Radius
//초기화할 때 또는 피격될 때 셰이더 실행 요청

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

void UBloodFieldSubSystem::RequestBloodSplat(FBloodBurstRequest Request)
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
	FBloodFieldShaderInterface::Dispatch(RT, static_cast<uint32>(Resolution), FieldScale, FieldOrigin, FVector3f(Request.WorldLocation), Request.Radius);
}

void UBloodFieldSubSystem::SetFieldOrigin(const FVector3f& InOrigin)
{
	FieldOrigin = InOrigin;
}

