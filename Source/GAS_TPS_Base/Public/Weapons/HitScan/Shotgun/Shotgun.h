// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/HitScan/HitScanWeapon.h"
#include "Shotgun.generated.h"

UCLASS()
class GAS_TPS_BASE_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()

public:
	AShotgun();

	virtual void Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) override;

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere, Category = WeaponScatter)
	uint32 NumberOfPellets = 10;
};
