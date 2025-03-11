// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScan/Shotgun/Shotgun.h"

#include "Engine/SkeletalMeshSocket.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/GameplayStatics.h"
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

void AShotgun::Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation,
	const FRotator& TargetRotation)
{
	AWeapon::Fire(TraceHitTarget, ProjectileSpawnLocation, TargetRotation);
	
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
 
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleSocket"))
	{
		UWorld* World = GetWorld();
		
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		TMap<ATPSCharacterBase*, uint32> HitMap;
		
		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(TraceHitTarget, Start, FireHit);

			ATPSCharacterBase* CharacterBase = Cast<ATPSCharacterBase>(FireHit.GetActor());
			if (CharacterBase && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(CharacterBase))
				{
					HitMap[CharacterBase]++;
				} else
				{
					HitMap.Emplace(CharacterBase, 1);
				}
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

		for (auto const HitPair: HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
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
