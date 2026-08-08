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
	FBloodSplat CalculateSplatLocation(const FBloodBurstRequest& Request, int x, int y);
	void FlushRequests();
	bool DoTrace(FHitResult& Result, const FVector& StartLocation, const FVector& Dir, float EndDistance);
	FSurfaceBasis BuildSurfaceBasis(const FBloodBurstRequest& Request);
	FVector TryWrapSharpEdge(const FVector& Center, const FVector& ImpactPoint, const FVector& Offset, const FVector& NewNormal, const FVector& OldNormal);
	FHitResult FindCloseDistanceNormal(const FHitResult& ResultA, const FHitResult& ResultB, const FVector& Origin, const FVector& OriginNormal);

	bool bShouldFlushRequests = false;
	int32 FramesSinceLastFlush = 0;
	int32 FlushIntervalFrames = 1;

	UPROPERTY()
	TArray<FBloodSplat> BloodSplats;

	UPROPERTY(Transient)
	TObjectPtr<class UTextureRenderTargetVolume> BloodFieldTarget;

	FVector3f FieldScale = FVector3f(1000.f, 1000.f, 500.f);
	FVector3f FieldOrigin = FVector3f::ZeroVector;

	UPROPERTY(EditAnywhere)
	int32 Resolution = 128;
};
