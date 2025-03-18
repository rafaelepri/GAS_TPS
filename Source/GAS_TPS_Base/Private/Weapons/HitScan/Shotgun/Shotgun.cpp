// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScan/Shotgun/Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AShotgun::AShotgun()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AShotgun::BeginPlay()
{
	Super::BeginPlay();
	
}

void AShotgun::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleSocket"))
	{
		UWorld* World = GetWorld();
		
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// Maps hit character to number of times hit
		TMap<ATPSCharacterBase*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(HitTarget, Start, FireHit);

			if (ATPSCharacterBase* CharacterBase = Cast<ATPSCharacterBase>(FireHit.GetActor()))
			{
				if (HitMap.Contains(CharacterBase))
				{
					HitMap[CharacterBase]++;
				} else
				{
					HitMap.Emplace(CharacterBase, 1);
				}

				if (World)
				{
					if (ImpactParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
			
					if (HitSound)
					{
						UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
					}
				}
			}
		}

		for (auto const HitPair: HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key, // character that was hit
					Damage * HitPair.Value, // multiply damage by number of times hit
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}

		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				Start
			);
		}

		if (MuzzleFlashParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				World,
				MuzzleFlashParticles,
				SocketTransform
			);
		}
		
	}
}

void AShotgun::ShotgunTraceEndWithScatter(const FVector_NetQuantize& TraceHitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleSocket");
	if (!MuzzleFlashSocket) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (TraceHitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		const FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLocation = SphereCenter + RandomVector;
		FVector ToEndLocation = EndLocation - TraceStart;
		ToEndLocation = TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size();
		
		HitTargets.Add(ToEndLocation);
	}
}
