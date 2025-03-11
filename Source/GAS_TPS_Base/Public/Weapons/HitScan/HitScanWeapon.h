// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class GAS_TPS_BASE_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:
	AHitScanWeapon();

	virtual void Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) override;

protected:
	virtual void BeginPlay() override;

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);
	void WeaponTraceHit(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlashParticles;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
public:
	virtual void Tick(float DeltaTime) override;

private:
	/*
	 **
	 *    Trace end with scatter;
	 */

	UPROPERTY(EditAnywhere, category = WeaponScatter)
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere, category = WeaponScatter)
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere, category = WeaponScatter)
	bool bUseScatter = false;
};
