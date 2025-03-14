// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "HealthPickup.generated.h"

UCLASS()
class GAS_TPS_BASE_API AHealthPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AHealthPickup();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;

	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	) override;

private:
	UPROPERTY(EditAnywhere)
	float HealAmount = 50.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
	
public:
};
