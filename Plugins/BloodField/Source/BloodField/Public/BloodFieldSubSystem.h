// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystems/LocalPlayerSubsystem.h"
#include "BloodBurstRequest.h"
#include "BloodFieldSubSystem.generated.h"

struct FSurfaceBasis
{
	FVector Normal;
	FVector Tangent;
	FVector Bitangent;
};

UCLASS()
class BLOODFIELD_API UBloodFieldSubSystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
public:	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual TStatId GetStatId() const override;
	virtual void Tick(float DeltaTime) override;

	void RequestBloodSplat(FBloodBurstRequest Request);
	
	void SetFieldOrigin(const FVector3f& InOrigin);
	class UTextureRenderTargetVolume* GetBloodFieldTarget() const { return BloodFieldTarget; }

private:
	bool CalculateSplatLocation(FSplatGPUData& Splat, const FSurfaceBasis& Basis, const FBloodBurstRequest& Request, const FVector2D& SampleUV);
	void FlushRequests();
	bool DoTrace(FHitResult& Result, const FVector& StartLocation, const FVector& Dir, float EndDistance);
	FSurfaceBasis BuildSurfaceBasis(const FBloodBurstRequest& Request);
	bool TryWrapSharpEdge(FVector& OutLocation, FSurfaceBasis& Basis, const FVector& Center, const FVector& ImpactPoint, const FVector& Offset, const FVector& NewNormal, const FVector& OldNormal);
	FHitResult FindCloseDistanceNormal(const FHitResult& ResultA, const FHitResult& ResultB, const FVector& Origin, const FVector& OriginNormal);

	bool bShouldFlushRequests = false;
	int32 FramesSinceLastFlush = 0;
	int32 FlushIntervalFrames = 1;
	float SmoothNormalThreshold = 0.9f;

	UPROPERTY()
	TArray<FBloodSplatGroup> SplatGroup;

	UPROPERTY(Transient)
	TObjectPtr<class UTextureRenderTargetVolume> BloodFieldTarget;

	FIntVector Resolution = { 512,512,52 };
	FVector3f FieldScale = FVector3f(5000.f, 5000.f, 500.f);
	FVector3f FieldOrigin = FVector3f::ZeroVector;
};