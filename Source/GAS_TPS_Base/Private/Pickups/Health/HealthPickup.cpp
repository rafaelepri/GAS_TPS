// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/Health/HealthPickup.h"

#include "Character/TpsCharacterBase.h"
#include "Components/BuffComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"




AHealthPickup::AHealthPickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

void AHealthPickup::BeginPlay()
{
	Super::BeginPlay();
	
}

void AHealthPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHealthPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (const ATPSCharacterBase* Character = Cast<ATPSCharacterBase>(OtherActor))
	{
		if (UBuffComponent* Buff = Character->GetBuffComponent())
		{
			Buff->Heal(HealAmount, HealingTime);
		}
	}

	Destroy();
}

void AHealthPickup::Destroyed()
{
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}

	Super::Destroyed();
}



