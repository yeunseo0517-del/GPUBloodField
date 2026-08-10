// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodFieldSubSystem.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "BloodFieldShaderInterface.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialParameterCollectionInstance.h"

namespace
{
	constexpr int32 PatternMask[5][5] =
	{
		{ 0, 1, 0, 0, 0 },
		{ 1, 1, 1, 0, 0 },
		{ 0, 0, 1, 1, 1 },
		{ 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 1, 0 }
	};
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

	//UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(BloodFieldMPC);

	/*if (MPCInstance)
	{
		MPCInstance->SetVectorParameterValue(TEXT("FieldOrigin"), FieldOrigin);
		MPCInstance->SetVectorParameterValue(TEXT("FieldScale"), FieldScale);
	}*/
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
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 5; ++j)
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
	Splat.Radius = Request.Radius;
	Splat.Strength = Request.Strength;

	FSurfaceBasis Basis = BuildSurfaceBasis(Request);
	FVector Center = Request.WorldLocation;
	const float XOffset = Request.Radius * (x - 2);
	const float YOffset = Request.Radius * (y - 2);
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
				return FBloodSplat();
			}

			if (!FinalResult.bBlockingHit) return FBloodSplat();

			Location = TryWrapSharpEdge(Center, FinalResult.ImpactPoint, Offset, FinalResult.ImpactNormal, Basis.Normal);
			
			if (Location == FVector::ZeroVector) return FBloodSplat();
		}
	}
	Splat.WorldLocation = Location;

	return Splat;
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

FVector UBloodFieldSubSystem::TryWrapSharpEdge(const FVector& Center, const FVector& ImpactPoint, const FVector& Offset, const FVector& NewNormal, const FVector& OldNormal)
{
	FVector MoveDir = Offset.GetSafeNormal();
	if (FMath::Abs(FVector::DotProduct(MoveDir, NewNormal)) <= 0.1f) return FVector::ZeroVector;
	float UsedDistance = FVector::DotProduct((ImpactPoint - Center), NewNormal) / FVector::DotProduct(MoveDir, NewNormal);
	float TotalDistance = Offset.Size();
	if (UsedDistance < 0.f || UsedDistance > TotalDistance) return FVector::ZeroVector;
	
	FVector CornerPoint = Center + MoveDir * UsedDistance;

	FQuat DeltaRotation = FQuat::FindBetweenNormals(OldNormal, NewNormal);
	FVector RotatedMoveDir = DeltaRotation.RotateVector(MoveDir);
	return CornerPoint + RotatedMoveDir * (TotalDistance - UsedDistance);
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

	for (const auto& Splat : BloodSplats)
	{
		DrawDebugSphere(GetWorld(), Splat.WorldLocation, Splat.Radius, 12, FColor::Red, false, 5.f);
		FBloodFieldShaderInterface::Dispatch(RT, static_cast<uint32>(Resolution), FieldScale, FieldOrigin, FVector3f(Splat.WorldLocation), Splat.Radius);
	}

	BloodSplats.Empty();
	bShouldFlushRequests = false;
}

