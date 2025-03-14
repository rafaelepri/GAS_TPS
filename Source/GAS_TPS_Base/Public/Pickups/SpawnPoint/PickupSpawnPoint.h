// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class GAS_TPS_BASE_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	APickupSpawnPoint();
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickupBase>> PickupClasses;

	UPROPERTY(EditAnywhere)
	APickupBase* SpawnedPickup;

	void SpawnPickup();
	UFUNCTION()
	void StartSpawnTimer(AActor* DestroyedActor);
	void SpawnTimerFinished();
private:
	FTimerHandle SpawnTimer;

	UPROPERTY(EditAnywhere)
	float MinimalSpawnTime = 0.f;
	UPROPERTY(EditAnywhere)
	float MaximalSpawnTime = 0.f;
public:

};
