// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockBase.generated.h"

UCLASS()
class GAS_TPS_BASE_API ABlockBase : public AActor {

	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABlockBase();

	UPROPERTY(BlueprintReadWrite, Category = "Components")
	TArray<class USplineComponent*> Ledges;
	UPROPERTY(BlueprintReadWrite, Category = "Components")
	TMap<USplineComponent*, USplineComponent*> OppositeLedges;
	UPROPERTY(BlueprintReadWrite, Category = "Components")
	float MinLedgeWidth = 60.f;

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	USplineComponent* FindLedgeClosestToActor(FVector ActorLocation);

public:

	UFUNCTION(BlueprintCallable)
	void GetLedgeTransform(FVector HitLocation, FVector ActorLocation, class ATPSCharacterBase* Character);
};
