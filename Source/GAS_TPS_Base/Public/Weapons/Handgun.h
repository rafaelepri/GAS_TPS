
#pragma once

#include "GameFramework/Actor.h"
#include "Handgun.generated.h"

UCLASS()
class AHandgun : public AActor  {
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "Handgun")
	USkeletalMeshComponent* SkeletalMeshComponent;
public:
	AHandgun();
};