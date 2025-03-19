// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScan/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"

AHitScanWeapon::AHitScanWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AHitScanWeapon::BeginPlay()
{
	Super::BeginPlay();
}


void AHitScanWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHitScanWeapon::Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation,
	const FRotator& TargetRotation)
{
	Super::Fire(TraceHitTarget, ProjectileSpawnLocation, TargetRotation);

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
 
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleSocket"))
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(TraceHitTarget, Start, FireHit);

		ATPSCharacterBase* CharacterBase = Cast<ATPSCharacterBase>(FireHit.GetActor());
		if (CharacterBase && HasAuthority() && InstigatorController)
		{
			UGameplayStatics::ApplyDamage(CharacterBase, Damage, InstigatorController, this, UDamageType::StaticClass());
		}

		if (UWorld* World = GetWorld())
		{
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
			
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}

			if (MuzzleFlashParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					World,
					MuzzleFlashParticles,
					SocketTransform
				);
			}

			if (FireSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					FireSound,
					Start
				);
			}
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, FHitResult& OutHit)
{
	if (const UWorld* World = GetWorld())
	{
		const FVector End = ProjectileSpawnLocation + (TraceHitTarget - ProjectileSpawnLocation) * 1.25f; // extends the trace after the hit so we always have accurate hit

		World->LineTraceSingleByChannel(
			OutHit,
			ProjectileSpawnLocation,
			End,
			ECC_Visibility
		);

		FVector BeamEnd = End;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}

		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles)
		{
			if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, ProjectileSpawnLocation))
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}