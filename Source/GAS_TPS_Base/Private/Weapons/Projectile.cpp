// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Projectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Character/TpsCharacterBase.h"
#include "GAS_TPS_Base/Public/GAS_TPS_Base.h"

AProjectile::AProjectile() {
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AProjectile::BeginPlay() {
	Super::BeginPlay();

	if (Tracer) {
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			  Tracer,
			  CollisionBox,
			  FName(),
			  GetActorLocation(),
			  GetActorRotation(),
			  EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority()) {
		CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	}
}

void AProjectile::Destroyed()
{
	Super::Destroyed();

	if (ConcreteParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ConcreteParticles,
			GetActorTransform()
		);
	}
	
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());	
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, 
                        FVector NormalImpulse, const FHitResult& HitResult) {
	//
	if (ATPSCharacterBase* TpsCharacter = Cast<ATPSCharacterBase>(OtherActor))
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "Cast to Character OK");
	}
	
	Destroy();
}

void AProjectile::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

