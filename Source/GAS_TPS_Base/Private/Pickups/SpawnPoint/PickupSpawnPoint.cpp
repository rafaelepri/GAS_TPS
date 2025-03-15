// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SpawnPoint/PickupSpawnPoint.h"
#include "Pickups/PickupBase.h"

APickupSpawnPoint::APickupSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	
}

void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	StartSpawnTimer((AActor*)nullptr);
}

void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupSpawnPoint::SpawnPickup()
{
	if (PickupClasses.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, PickupClasses.Num() - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickupBase>(PickupClasses[Selection], GetActorTransform());

		if (HasAuthority() && SpawnedPickup)
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnTimer);
		}
	}
}

void APickupSpawnPoint::StartSpawnTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(MinimalSpawnTime, MaximalSpawnTime);
	GetWorldTimerManager().SetTimer(
		SpawnTimer,
		this,
		&APickupSpawnPoint::SpawnTimerFinished,
		SpawnTime
	);
}

void APickupSpawnPoint::SpawnTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

