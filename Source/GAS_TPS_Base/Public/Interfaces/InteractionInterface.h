// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GAS_TPS_BASE_API IInteractionInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION()
	virtual void SetInteractionTransform(FTransform InTransform) {}
	UFUNCTION()
	virtual FTransform GetInteractionTransform() { return FTransform(); }


	UFUNCTION()
	virtual void PlayTraversalMontage(UAnimMontage* MontageToPlay, float PlayRate, float StartTime) {}
};
