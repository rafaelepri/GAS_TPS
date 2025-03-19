

#include "Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Character/TpsCharacterBase.h"
#include "PlayerController/TpsPlayerController.h"
#include "Animation/AnimationAsset.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/Casing.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon() {
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetCollisionResponseToChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::BeginPlay() {
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidget) {
		PickupWidget->SetVisibility(false);
	}
}

void AWeapon::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	//
	if (ATPSCharacterBase* Character = Cast<ATPSCharacterBase>(OtherActor)) {
		Character->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	//
	if (ATPSCharacterBase* Character = Cast<ATPSCharacterBase>(OtherActor)) {
		Character->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetWeaponState(const EWeaponState State) {
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquippedStateSet();
		break;

	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondaryStateSet();
		break;
		
	case EWeaponState::EWS_Dropped:
		OnDroppedStateSet();
		break;
	}
}

void AWeapon::OnEquippedStateSet()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // this stops overlap events
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	EnableCustomDepth(false);
}

void AWeapon::OnDroppedStateSet()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);	
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);
}

void AWeapon::OnEquippedSecondaryStateSet()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // this stops overlap events
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		
	EnableCustomDepth(false);
}

void AWeapon::OnRep_WeaponState() {
	switch (WeaponState)
	{
		case EWeaponState::EWS_Equipped:
			ShowPickupWidget(false);
			WeaponMesh->SetSimulatePhysics(false);
			WeaponMesh->SetEnableGravity(false);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

			EnableCustomDepth(false);
		break;
		
		case EWeaponState::EWS_Dropped:
			WeaponMesh->SetSimulatePhysics(true);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
			WeaponMesh->MarkRenderStateDirty();
			EnableCustomDepth(true);
		break;
	}
}

void AWeapon::ShowPickupWidget(const bool bShowWidget) const {
	if (PickupWidget) {
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) {
	if (FireAnimation) {
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (CasingClass)
	{
		if (const USkeletalMeshSocket* ShellSocket = WeaponMesh->GetSocketByName("ShellEjectionSocket"))
		{
			const FTransform SocketTransform = ShellSocket->GetSocketTransform(WeaponMesh);
			
			if (UWorld* World = GetWorld())
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}

	SpendRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	TPSChar = nullptr;
	TPSPlayerController = nullptr;
}

void AWeapon::SetHUDAmmo()
{
	TPSChar = TPSChar == nullptr ? Cast<ATPSCharacterBase>(GetOwner()) : TPSChar;
	if (TPSChar)
	{
		TPSPlayerController = TPSPlayerController == nullptr ? Cast<ATpsPlayerController>(TPSChar->Controller) : TPSPlayerController;
		if (TPSPlayerController)
		{
			TPSPlayerController->SetHUDWeaponAmmo(WeaponAmmo);
		}
	}
}

void AWeapon::SpendRound()
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo - 1, 0, MagCapacity);
	SetHUDAmmo();

	if (HasAuthority())
	{
		ClientUpdateAmmo(WeaponAmmo);
	} else
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority())
	{
		return;
	}
	
	WeaponAmmo = ServerAmmo;
	--Sequence;
	WeaponAmmo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(const int32 AmmoToAdd)
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();

	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(const int32 AmmoToAdd)
{
	WeaponAmmo = FMath::Clamp(WeaponAmmo + AmmoToAdd, 0, MagCapacity);
	TPSChar = !TPSChar ? Cast<ATPSCharacterBase>(GetOwner()) : TPSChar;
	if (TPSChar)
	{
		// handle shotgun reload here / i skipped this class
	}
	SetHUDAmmo();
}

void AWeapon::PlayReloadAnimation() const
{
	if (ReloadAnimation) {
		WeaponMesh->PlayAnimation(ReloadAnimation, false);
	}
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (!Owner)
	{
		TPSChar = nullptr;
		TPSPlayerController = nullptr;
	} else
	{
		TPSChar = !TPSChar ? Cast<ATPSCharacterBase>(Owner) : TPSChar;
		if (TPSChar && TPSChar->GetEquippedWeapon() && TPSChar->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

bool AWeapon::IsEmpty() const
{
	return WeaponAmmo <= 0;
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleSocket");
	if (!MuzzleFlashSocket) return FVector::ZeroVector;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();
	
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLocation = SphereCenter + RandomVector;
	const FVector ToEndLocation = EndLocation - TraceStart;
	
	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLocation , 4.f, 24, FColor::Orange, true);
	// DrawDebugLine(
	// 	GetWorld(),
	// TraceStart,
	// FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()),
	// FColor::Green,
	// true
	// );

	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}

