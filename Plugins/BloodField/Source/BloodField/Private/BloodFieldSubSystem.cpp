// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodFieldSubSystem.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "BloodFieldShaderInterface.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "BloodFieldSettings.h"

void UBloodFieldSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UBloodFieldSettings* Settings = GetDefault<UBloodFieldSettings>();

	const UMaterialParameterCollection* MPC = Settings->BloodFieldMPC.LoadSynchronous();
	if (MPC)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(MPC);

		if (MPCInstance)
		{
			MPCInstance->SetVectorParameterValue(TEXT("FieldOrigin"), FieldOrigin);
			MPCInstance->SetVectorParameterValue(TEXT("FieldScale"), FieldScale);
		}
	}

	BloodFieldTarget = Settings->BloodFieldRenderTarget.LoadSynchronous();

	if (!BloodFieldTarget) return;

	BloodFieldTarget->bCanCreateUAV = true;
	// 혈흔 값은 선형 데이터
	BloodFieldTarget->bForceLinearGamma = true;
	// 초기값 0
	BloodFieldTarget->ClearColor = FLinearColor::Transparent;
	// 128 × 128 × 128, 복셀당 float 하나
	BloodFieldTarget->Init(
		Resolution.X,
		Resolution.Y,
		Resolution.Z,
		PF_FloatRGBA);
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

	FSurfaceBasis Basis = BuildSurfaceBasis(Request);

	FBloodSplatGroup BloodSplatGroup;
	FSplatGPUGroupData GroupData;

	const UBloodFieldSettings* Settings = GetDefault<UBloodFieldSettings>();
	const int32 PatternIndex = FMath::RandRange(0, Settings->BloodPatternData.Num() - 1);
	GroupData.PatternID = PatternIndex;
	const UBloodPatternData* BloodPattern = Settings->BloodPatternData[PatternIndex].LoadSynchronous();
	const TArray<FVector2D> PatternSample = BloodPattern->SampleUVs;
	for (int i = 0; i < PatternSample.Num(); ++i)
	{
		FSplatGPUData SplatData;
		SplatData.Tangent = FVector3f(Basis.Tangent);
		SplatData.Bitangent = FVector3f(Basis.Bitangent);
		if (CalculateSplatLocation(SplatData, Basis, Request, PatternSample[i]))
		{
			BloodSplatGroup.BloodSplats.Add(SplatData);
		}
	}
	BloodSplatGroup.SplatGroupData = GroupData;
	SplatGroup.Add(BloodSplatGroup);
}

void UBloodFieldSubSystem::SetFieldOrigin(const FVector3f& InOrigin)
{
	FieldOrigin = InOrigin;
	const UBloodFieldSettings* Settings = GetDefault<UBloodFieldSettings>();
	const UMaterialParameterCollection* MPC = Settings->BloodFieldMPC.LoadSynchronous();
	if (MPC)
	{
		UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(MPC);

		if (MPCInstance)
		{
			MPCInstance->SetVectorParameterValue(TEXT("FieldOrigin"), FieldOrigin);
			MPCInstance->SetVectorParameterValue(TEXT("FieldScale"), FieldScale);
		}
	}
}

bool UBloodFieldSubSystem::CalculateSplatLocation(FSplatGPUData& Splat, const FSurfaceBasis& Basis, const FBloodBurstRequest& Request, const FVector2D& SampleUV)
{
	const UBloodFieldSettings* Settings = GetDefault<UBloodFieldSettings>();
	const int32 GridSize = Settings->BloodPatternSettings.GridSize;

	Splat.SampleUV = FVector2f(SampleUV);
	FVector Center = Request.WorldLocation;
	const float XOffset = Request.Radius * (SampleUV.X - 0.5) * GridSize;
	const float YOffset = Request.Radius * (SampleUV.Y - 0.5) * GridSize;
	FVector Offset = Basis.Tangent * XOffset + Basis.Bitangent * YOffset;
	FVector Location = Center + Offset;

	//DrawDebugLine(GetWorld(), Center, Center + Basis.Normal * 100.f, FColor::Blue , false, 5.f, 0, 3.f);

	FVector OriginalLoc = Location;

	// 1차 Normal 방향 Trace
	bool bValidNormalHit = false;
	FHitResult NormalHit;
	if (DoTrace(NormalHit, OriginalLoc + Basis.Normal * 50.f, -Basis.Normal, 100.f))
	{
		const float CorrectDistance = (NormalHit.ImpactPoint - OriginalLoc).Size();
		const float NormalDot = FVector::DotProduct(NormalHit.ImpactNormal.GetSafeNormal(), Basis.Normal);

		if (CorrectDistance <= 5.f && NormalDot >= SmoothNormalThreshold)
		{
			Location = NormalHit.ImpactPoint;
			bValidNormalHit = true;
		}
	}
	
	if (!bValidNormalHit)
	{
		// 2차 Trace
		if (!Offset.IsNearlyZero())
		{
			FVector MoveDir = Offset.GetSafeNormal();
			FVector StartPoint = OriginalLoc - Basis.Normal * 1.f;

			FHitResult PositiveDirHit;
			const bool bPositiveDirHit = DoTrace(PositiveDirHit, StartPoint, MoveDir, 50);

			FHitResult NegativeDirHit;
			const bool bNegativeDirHit = DoTrace(NegativeDirHit, StartPoint, -MoveDir, 50.f);

			FHitResult FinalResult;
			if (bPositiveDirHit && bNegativeDirHit)
			{
				FinalResult = FindCloseDistanceNormal(PositiveDirHit, NegativeDirHit, OriginalLoc, Basis.Normal);
			}
			else if (bPositiveDirHit)
			{
				if (FVector::DotProduct(PositiveDirHit.ImpactNormal, Basis.Normal) < SmoothNormalThreshold)
				{
					FinalResult = PositiveDirHit;
				}
			}
			else if (bNegativeDirHit)
			{
				if (FVector::DotProduct(NegativeDirHit.ImpactNormal, Basis.Normal) < SmoothNormalThreshold)
				{
					FinalResult = NegativeDirHit;
				}
			}
			else
			{
				return false;
			}

			if (!FinalResult.bBlockingHit) return false;

			FSurfaceBasis RotateBasis;
			bool IsFound = TryWrapSharpEdge(Location, RotateBasis, Center, FinalResult.ImpactPoint, Offset, FinalResult.ImpactNormal, Basis.Normal);
			if (!IsFound) return false;

			Splat.Tangent = FVector3f(RotateBasis.Tangent);
			Splat.Bitangent = FVector3f(RotateBasis.Bitangent);
		}
	}
	Splat.Location = FVector3f(Location);

	return true;
}

FSurfaceBasis UBloodFieldSubSystem::BuildSurfaceBasis(const FBloodBurstRequest& Request)
{
	FSurfaceBasis Basis;
	Basis.Normal = Request.ImpactNormal.GetSafeNormal();
	FVector ProjectedDirection = Request.Direction - FVector::DotProduct(Request.Direction, Basis.Normal) * Basis.Normal;

	Basis.Tangent = ProjectedDirection.GetSafeNormal();
	if (ProjectedDirection == FVector::ZeroVector)
	{
		FVector RefN = FVector::UpVector;
		if (abs(FVector::DotProduct(Basis.Normal, RefN)) > 0.99) RefN = FVector::ForwardVector;
		Basis.Tangent = FVector::CrossProduct(Basis.Normal, RefN).GetSafeNormal();
	}
	Basis.Bitangent = FVector::CrossProduct(Basis.Normal, Basis.Tangent).GetSafeNormal();
	return Basis;
}

bool UBloodFieldSubSystem::DoTrace(FHitResult& Result, const FVector& StartLocation, const FVector& Dir, float EndDistance)
{
	FVector Start = StartLocation;
	FVector End = StartLocation + Dir * EndDistance;
	return GetWorld()->LineTraceSingleByChannel(Result, Start, End, ECollisionChannel::ECC_Visibility);
}

bool UBloodFieldSubSystem::TryWrapSharpEdge(FVector& OutLocation, FSurfaceBasis& Basis, const FVector& Center, const FVector& ImpactPoint, const FVector& Offset, const FVector& NewNormal, const FVector& OldNormal)
{
	FVector MoveDir = Offset.GetSafeNormal();
	if (FMath::Abs(FVector::DotProduct(MoveDir, NewNormal)) <= 0.1f) return false;
	float UsedDistance = FVector::DotProduct((ImpactPoint - Center), NewNormal) / FVector::DotProduct(MoveDir, NewNormal);
	float TotalDistance = Offset.Size();
	if (UsedDistance < 0.f || UsedDistance > TotalDistance) return false;

	FVector CornerPoint = Center + MoveDir * UsedDistance;

	FQuat DeltaRotation = FQuat::FindBetweenNormals(OldNormal, NewNormal);
	FVector RotatedMoveDir = DeltaRotation.RotateVector(MoveDir);
	
	OutLocation = CornerPoint + RotatedMoveDir * (TotalDistance - UsedDistance);
	Basis.Normal = NewNormal;
	Basis.Tangent = DeltaRotation.RotateVector(Basis.Tangent);
	Basis.Bitangent = DeltaRotation.RotateVector(Basis.Bitangent);
	return true;
}

FHitResult UBloodFieldSubSystem::FindCloseDistanceNormal(const FHitResult& ResultA, const FHitResult& ResultB, const FVector& OriginLoc, const FVector& OriginNormal)
{
	float DistanceA = (ResultA.ImpactPoint - OriginLoc).Size();
	float DistanceB = (ResultB.ImpactPoint - OriginLoc).Size();

	bool bValidA = FMath::Abs(FVector::DotProduct(ResultA.ImpactNormal, OriginNormal)) < 0.2f;
	bool bValidB = FMath::Abs(FVector::DotProduct(ResultB.ImpactNormal, OriginNormal)) < 0.2f;

	if (bValidA && bValidB)
		return DistanceA <= DistanceB ? ResultA : ResultB;
	return FHitResult();
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

	FBloodFieldPassInput Input;
	Input.Resolution = Resolution;
	Input.Scale = FieldScale;
	Input.Origin = FieldOrigin;

	const UBloodFieldSettings* Settings = GetDefault<UBloodFieldSettings>();
	const FBloodPatternSettings& PatternSetting = Settings->BloodPatternSettings;
	Input.Radius = PatternSetting.SplatRadius;
	const float PatternSize = PatternSetting.SplatRadius * PatternSetting.GridSize;
	Input.PatternWorldSize = FVector2f(PatternSize, PatternSize);

	for (int32 GroupIndex = 0; GroupIndex < SplatGroup.Num(); ++GroupIndex)
	{
		const auto& Group = SplatGroup[GroupIndex];
		Input.GroupData.Add(Group.SplatGroupData);
		for (auto Splat : Group.BloodSplats)
		{
			Splat.GroupIndex = static_cast<uint32>(GroupIndex);
			Input.SplatsData.Add(Splat);
		}
	}
	FBloodFieldShaderInterface::Dispatch(RT, Input);

	SplatGroup.Empty();
	bShouldFlushRequests = false;
}

