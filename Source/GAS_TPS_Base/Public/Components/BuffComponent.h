// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GAS_TPS_BASE_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	friend class ATPSCharacterBase;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealingTime);
protected:
	virtual void BeginPlay() override;

	void HealRampUp(const float DeltaTime);
private:
	UPROPERTY()
	ATPSCharacterBase* Character;

	bool bIsHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
	
public:

};
