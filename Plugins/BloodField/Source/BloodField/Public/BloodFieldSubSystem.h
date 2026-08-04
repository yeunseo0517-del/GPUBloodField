// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SubSystems/LocalPlayerSubsystem.h"
#include "BloodFieldSubSystem.generated.h"

UCLASS()
class BLOODFIELD_API UBloodFieldSubSystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	void RequestBloodSplat(struct FBloodBurstRequest Request);
	
	void SetFieldOrigin(const FVector3f& InOrigin);
	class UTextureRenderTargetVolume* GetBloodFieldTarget() const { return BloodFieldTarget; }

private:
	UPROPERTY(Transient)
	TObjectPtr<class UTextureRenderTargetVolume> BloodFieldTarget;

	FVector3f FieldScale = FVector3f(1000.f, 1000.f, 500.f);
	FVector3f FieldOrigin = FVector3f::ZeroVector;

	UPROPERTY(EditAnywhere)
	int32 Resolution = 128;
};
