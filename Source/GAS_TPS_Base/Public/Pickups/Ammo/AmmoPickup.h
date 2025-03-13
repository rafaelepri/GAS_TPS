// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/PickupBase.h"
#include "Weapons/Enums/WeaponTypes.h"
#include "AmmoPickup.generated.h"

UCLASS()
class GAS_TPS_BASE_API AAmmoPickup : public APickupBase
{
	GENERATED_BODY()

public:
	AAmmoPickup();
	virtual void Tick(float DeltaTime) override;

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
	int32 AmmoAmount = 30;
	
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

};
