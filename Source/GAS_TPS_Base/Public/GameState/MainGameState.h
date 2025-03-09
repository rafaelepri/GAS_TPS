// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MainGameState.generated.h"

/**
 * 
 */
UCLASS()
class GAS_TPS_BASE_API AMainGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class AMainPlayerState* ScoringPlayer);
	
	UPROPERTY(Replicated)
	TArray<AMainPlayerState*> TopScoringPlayers;

private:
	float TopScore = 0.f;
};
