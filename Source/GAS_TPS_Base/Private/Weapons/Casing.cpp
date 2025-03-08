
#include "Weapons/Casing.h"

#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

ACasing::ACasing() {
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>("CasingMesh");
	SetRootComponent(CasingMesh);
	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = FMath::RandRange(5.5f, 6.3f);
	ShellAngularVelocity = FMath::VRand() * FMath::RandRange(2100.f, 2500.f);
}

void ACasing::BeginPlay() {
	Super::BeginPlay();
	
	CasingMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	CasingMesh->AddAngularImpulseInDegrees(ShellAngularVelocity);

	SetLifeSpan(4.5f);
}

void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	UGameplayStatics::PlaySoundAtLocation(
		this,
		CasingSound,
		GetActorLocation()
	);

	CasingMesh->SetNotifyRigidBodyCollision(false);
}


