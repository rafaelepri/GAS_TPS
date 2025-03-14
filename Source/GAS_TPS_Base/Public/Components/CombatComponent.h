// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/TpsHUD.h"
#include "Weapons/Enums/WeaponTypes.h"
#include "Enums/CombatState.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_TPS_BASE_API UCombatComponent : public UActorComponent {
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class ATPSCharacterBase;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(AWeapon* WeaponToEquip);

	void FireButtonPressed(const bool bPressed);

	void PickupAmmo(const EWeaponType WeaponType,const int32 AmmoAmount);
protected:
	virtual void BeginPlay() override;

	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult) const;

	void SetHUDCrosshairs(const float DeltaTime);
	
private:
	UPROPERTY()
	ATPSCharacterBase* Character;

	UPROPERTY()
	class ATpsPlayerController* TpsPlayerController;
	
	UPROPERTY()
	ATpsHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;
	UFUNCTION()
	void OnRep_CombatState();
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_CarryingAmmo)
	int32 CarryingAmmo;
	UFUNCTION()
	void OnRep_CarryingAmmo();

	TMap<EWeaponType, int32> CarryingAmmoMap;
	UPROPERTY(EditAnywhere)
	int32 Starting_AR_Ammo = 120;
	UPROPERTY(EditAnywhere)
	int32 Starting_Pistol_Ammo = 42;
	UPROPERTY(EditAnywhere)
	int32 Starting_Smg_Ammo = 42;
	UPROPERTY(EditAnywhere)
	int32 Starting_Shotgun_Ammo = 20;
	UPROPERTY(EditAnywhere)
	int32 Starting_Sniper_Ammo = 20;
	void InitializeCarryingAmmo();

	bool CanFire() const;
	void Fire();
	bool bFireButtonPressed;


	void Reload();
	void HandleReload();
	UFUNCTION(Server, Reliable)
	void ServerReload();
	UFUNCTION(BlueprintCallable)
	void OnReloadCompleted();
	int32 AmountToReload();
	void UpdateAmmoValues(const bool bIsReloading);

	/* *
	 *   HUD and Crosshairs
	 */

	FHUDPackage HUDPackage;

	float CrosshairVelocityFactor;
	float CrosshairAimingFactor;
	float CrosshairShootingFactor;


	/*
	 **
	 *  Automatic Fire
	*/
	bool bCanFire = true;
	
	FTimerHandle FireTimer;

	void StartFireTimer();
	void FireTimerFinished();

	
	
public:	
};
