// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Ammo/AmmoPickup.h"
#include "Character/TpsCharacterBase.h"
#include "Components/CombatComponent.h"

AAmmoPickup::AAmmoPickup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();
}

void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (const ATPSCharacterBase* Character = Cast<ATPSCharacterBase>(OtherActor))
	{
		if (UCombatComponent* CombatComponent = Character->GetCombatComponent())
		{
			CombatComponent->PickupAmmo(WeaponType, AmmoAmount);
		}
	}

	Destroy();
}
