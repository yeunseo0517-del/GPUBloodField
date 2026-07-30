// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/BloodFieldManager.h"

//UTextureRenderTargetVolume 참조 보관
//Resolution, Scale, Origin
//혈흔이 생긴 Location, Radius
//초기화할 때 또는 피격될 때 셰이더 실행 요청

// Sets default values
ABloodFieldManager::ABloodFieldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABloodFieldManager::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABloodFieldManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

