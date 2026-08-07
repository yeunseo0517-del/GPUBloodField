
// Fill out your copyright notice in the Description page of Project Settings.


#include "BloodFieldDebugActor.h"
#include "BloodFieldSubSystem.h"
#include "Engine/TextureRenderTargetVolume.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ABloodFieldDebugActor::ABloodFieldDebugActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	DebugPlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Mesh"));
	SetRootComponent(DebugPlaneMesh);
}

// Called when the game starts or when spawned
void ABloodFieldDebugActor::BeginPlay()
{
	Super::BeginPlay();

	UBloodFieldSubSystem* BloodFieldSubsystem = GetWorld()->GetSubsystem <UBloodFieldSubSystem>();
	if (!BloodFieldSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Find Blood Field Subsystem"));
		return;
	}
	UMaterialInstanceDynamic* DebugMID = DebugPlaneMesh->CreateDynamicMaterialInstance(0); if (!DebugMID) { return; } DebugMID->SetTextureParameterValue(TEXT("BloodFieldVolume"), BloodFieldSubsystem->GetBloodFieldTarget());
}

// Called every frame
void ABloodFieldDebugActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

