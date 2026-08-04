// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BloodFieldDebugActor.generated.h"

UCLASS()
class BLOODSURFACERESEARCH_API ABloodFieldDebugActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABloodFieldDebugActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* DebugPlaneMesh;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
