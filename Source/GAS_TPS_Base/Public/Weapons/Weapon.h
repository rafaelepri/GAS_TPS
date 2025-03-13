#pragma once

#include "GameFramework/Actor.h"
#include "Enums/WeaponTypes.h"
#include "Weapon.generated.h"

class USphereComponent;
class UWidgetComponent;
class UAnimationAsset;

UCLASS()
class AWeapon : public AActor  {
	GENERATED_BODY()
	
public:
	AWeapon();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	
	void ShowPickupWidget(bool bShowWidget) const;
	
	virtual void Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation);

	void Dropped();

	void SetHUDAmmo();

	void AddAmmo(const int32& AmmoToAdd);

	void PlayReloadAnimation() const;


	/*
	*			Enable or disable custom depth (outline effect)
	*/

	void EnableCustomDepth(bool bEnable);
	
	
	/*
	*			Textures for weapon crosshairs
	*/

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D* CrosshairBottom;

	/*
	 **
	 * WEAPON PROPERTIES
	 */

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.1f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bIsWeaponAutomatic = true;

	
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	
	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	virtual void OnEquipped();


private:
	UPROPERTY()
	class ATPSCharacterBase* TPSChar;
	UPROPERTY()
	class ATpsPlayerController* TPSPlayerController;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UStaticMeshComponent* MeleeMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USphereComponent* AreaSphere;

	UFUNCTION()
	void OnRep_WeaponState();
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState , VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* FireAnimation;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	UAnimationAsset* ReloadAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ACasing> CasingClass;


	UPROPERTY(EditAnywhere)
	int32 MagCapacity;
	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	int32 WeaponAmmo;
	UFUNCTION()
	void OnRep_Ammo();

	void SpendRound();
public:
	void SetWeaponState(const EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	bool IsEmpty() const;
	FORCEINLINE int32 GetAmmo() const { return WeaponAmmo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }

};
