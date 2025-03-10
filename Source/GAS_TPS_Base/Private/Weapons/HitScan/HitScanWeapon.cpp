// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/HitScan/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

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
		const FVector End = Start + (TraceHitTarget - Start) * 1.25f; // extends the trace after the hit so we always have accurate hit

		if (UWorld* World = GetWorld())
		{
			FHitResult FireHit;
			
			World->LineTraceSingleByChannel(
				FireHit,
				Start,
				End,
				ECC_Visibility
			);

			FVector BeamEnd = End;

			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;

				ATPSCharacterBase* CharacterBase = Cast<ATPSCharacterBase>(FireHit.GetActor());
				if (CharacterBase && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(CharacterBase, Damage, InstigatorController, this, UDamageType::StaticClass());
				}

				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(GetWorld(), HitSound, FireHit.ImpactPoint);
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}
			
			if (BeamParticles)
			{
				if (UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform))
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
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
					GetActorLocation()
				);
			}
		}
	}
}

