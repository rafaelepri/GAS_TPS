
#include "Weapons/ProjectileWeapon.h"

#include "Weapons/Projectile.h"

void AProjectileWeapon::Fire(const FVector_NetQuantize& TraceHitTarget, const FVector_NetQuantize& ProjectileSpawnLocation, const FRotator& TargetRotation) {
	Super::Fire(TraceHitTarget, ProjectileSpawnLocation, TargetRotation);


	if (!HasAuthority()) return; 
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	if (ProjectileClass && InstigatorPawn)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;
		if (UWorld* World = GetWorld())
		{
			World->SpawnActor<AProjectile>(
				ProjectileClass,
				ProjectileSpawnLocation,
				TargetRotation,
				SpawnParams
			);
		}
	}
}
