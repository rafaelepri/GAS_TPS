// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "Character/TpsCharacterBase.h"
#include "Weapons/Weapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerController/TpsPlayerController.h"
#include "HUD/TpsHUD.h"

UCombatComponent::UCombatComponent() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarryingAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::BeginPlay() {
	Super::BeginPlay();

	if (Character)
	{
		// TpsPlayerController = Cast<ATpsPlayerController>(Character->Controller);
		// if (TpsPlayerController)
		// {
		// 	HUD = Cast<ATpsHUD>(TpsPlayerController->GetHUD());
		// }
		// the above code is commented because this cast is failing on the server, I've removed from begin play and added to
		// equip weapon function

		if (Character->HasAuthority())
		{
			InitializeCarryingAmmo();
		}
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::SetHUDCrosshairs(const float DeltaTime) {
	if (!Character) return;

	if (EquippedWeapon) {
		HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
		HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
		HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
		HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
		HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
	} else
	{
		HUDPackage.CrosshairCenter = nullptr;
		HUDPackage.CrosshairLeft = nullptr;
		HUDPackage.CrosshairRight = nullptr;
		HUDPackage.CrosshairTop = nullptr;
		HUDPackage.CrosshairBottom = nullptr;
	}

	// Calculate Crosshairs spread;
	const FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
	const FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());
	
	if (Character->CharacterInputState.bWantsToAim)
	{
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.5f, DeltaTime, 30.f);
	} else
	{
		CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.f, DeltaTime, 30.f);
	}

	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 30.f);
	
	HUDPackage.CrosshairSpread = 0.8f + // baseline value
		CrosshairVelocityFactor -
		CrosshairAimingFactor +
		CrosshairShootingFactor;
	
	if (HUD)
	{
		HUD->setHUDPackage(HUDPackage);
	}
}

void UCombatComponent::FireButtonPressed(const bool bPressed) {
	if (EquippedWeapon == nullptr || !Character) return;
	
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}	
}

bool UCombatComponent::CanFire() const
{
	if (EquippedWeapon->IsEmpty() || !bCanFire || CombatState == ECombatState::ECS_Reloading) return false;
	return true;
}

void UCombatComponent::Fire() {
	if (!CanFire()) return;
	bCanFire = false;
	
	FHitResult HitResult;
	TraceUnderCrosshairs(HitResult);
		
	const FVector_NetQuantize ProjectileSpawnLocation = EquippedWeapon->GetWeaponMesh()->GetSocketLocation("MuzzleSocket");
	const FVector ToTarget = HitResult.ImpactPoint - ProjectileSpawnLocation;
	const FRotator TargetRotation = ToTarget.Rotation();
	// the above code is taking care of the muzzle socket location and sending it down the line of fire, so the server have an accurate position
	// of the socket in order to spawn the projectile in the correct position on all clients

	ServerFire(HitResult.ImpactPoint, ProjectileSpawnLocation, TargetRotation);

	CrosshairShootingFactor = 1.f;

	StartFireTimer();
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) { // called in a client and executed on the server (RPC)
	MulticastFire(TraceHitTarget, ProjectileSpawnLocation, TargetRotation);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) { // If we call a function on the server from the client, we use this multicast rpc function to broadcast the event to all clients
	// handle recoil here
	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		EquippedWeapon->Fire(TraceHitTarget, ProjectileSpawnLocation, TargetRotation);
	}
}

void UCombatComponent::StartFireTimer() {
	Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished() {
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bIsWeaponAutomatic)
	{
		Fire();
	}
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Reload()
{
	if (CombatState == ECombatState::ECS_Reloading || EquippedWeapon == nullptr || !Character) return;
	
	if (CarryingAmmo > 0)
	{
		ServerReload();
	}
}

void UCombatComponent::HandleReload()
{
	EquippedWeapon->PlayReloadAnimation();
}


void UCombatComponent::ServerReload_Implementation()
{
	CombatState = ECombatState::ECS_Reloading;
	
	HandleReload();
}

void UCombatComponent::OnReloadCompleted()
{
	if (Character && Character->HasAuthority() && CombatState == ECombatState::ECS_Reloading)
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}

	if (bFireButtonPressed)
	{
		Fire();
	}
}

int32 UCombatComponent::AmountToReload()
{
	const int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();

	if (CarryingAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		const int32 CarryingAmount = 	CarryingAmmoMap[EquippedWeapon->GetWeaponType()];
		const int32 Least = FMath::Min(RoomInMag, CarryingAmount);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	
	return 0;
}

void UCombatComponent::UpdateAmmoValues()
{
	const int32 ReloadAmount = AmountToReload();
	if (CarryingAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarryingAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarryingAmmo = CarryingAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Character->Controller) : TpsPlayerController; 	
	if (TpsPlayerController)
	{
		TpsPlayerController->SetHUDCarryingAmmo(CarryingAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	default:
		break;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult) const {
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	const FVector2D CrosshairLocation(ViewportSize.X / 2, ViewportSize.Y / 2);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	const bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this,0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		const float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
		Start += CrosshairWorldDirection * (DistanceToCharacter + 65.f);

		const FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);

		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}

		// if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInteractionInterface>())
		// {
		// 	HUDPackage.CrosshairColor = FLinearColor::Red;
		// } else
		// {
		// 	HUDPackage.CrosshairColor = FLinearColor::White;
		// }

		DrawDebugSphere(
			GetWorld(),
			TraceHitResult.ImpactPoint,
			12.f,
			12,
			FColor::Red
		);
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip) {
	if (!Character || !WeaponToEquip) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	switch (EquippedWeapon->GetWeaponType()) {
		case EWeaponType::EWT_Melee:
			Character->CharacterWeaponState.bHasMelee = true;
			Character->CharacterWeaponState.bHasPistol = false;
			Character->CharacterWeaponState.bHasRifle = false;

			if (const USkeletalMeshSocket* MeleeWeaponSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))) {
				MeleeWeaponSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		case EWeaponType::EWT_Pistol:
			Character->CharacterWeaponState.bHasMelee = false;
			Character->CharacterWeaponState.bHasPistol = true;
			Character->CharacterWeaponState.bHasRifle = false;

			if (const USkeletalMeshSocket* PistolSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))) {
				PistolSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		case EWeaponType::EWT_Rifle:
			Character->CharacterWeaponState.bHasMelee = false;
			Character->CharacterWeaponState.bHasPistol = false;
			Character->CharacterWeaponState.bHasRifle = true;
			
			if (const USkeletalMeshSocket* RifleSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket_Rifle"))) {
				RifleSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		default:
			break;
	}
	
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if (CarryingAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarryingAmmo = CarryingAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Character->Controller) : TpsPlayerController;

	if (TpsPlayerController)
	{
		TpsPlayerController->SetHUDCarryingAmmo(CarryingAmmo);

		HUD = Cast<ATpsHUD>(TpsPlayerController->GetHUD());
	}
}

void UCombatComponent::OnRep_EquippedWeapon() const {
	if (EquippedWeapon && Character) {
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

		switch (EquippedWeapon->GetWeaponType()) {
		case EWeaponType::EWT_Melee:
			if (const USkeletalMeshSocket* MeleeWeaponSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))) {
				MeleeWeaponSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		case EWeaponType::EWT_Pistol:
			if (const USkeletalMeshSocket* PistolSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"))) {
				PistolSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		case EWeaponType::EWT_Rifle:
			if (const USkeletalMeshSocket* RifleSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket_Rifle"))) {
				RifleSocket->AttachActor(EquippedWeapon, Character->GetMesh());
			}
			break;
		default:
			break;
		}
	}
}


void UCombatComponent::OnRep_CarryingAmmo()
{
	TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Character->Controller) : TpsPlayerController;
	if (TpsPlayerController)
	{
		TpsPlayerController->SetHUDCarryingAmmo(CarryingAmmo);
	}
}

void UCombatComponent::InitializeCarryingAmmo()
{
	CarryingAmmoMap.Emplace(EWeaponType::EWT_Rifle, Starting_AR_Ammo);
	CarryingAmmoMap.Emplace(EWeaponType::EWT_Pistol, Starting_Pistol_Ammo);
}




