// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerState/MainPlayerState.h"
#include "Character/TpsCharacterBase.h"
#include "PlayerController/TpsPlayerController.h"
#include "Net/UnrealNetwork.h"



void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerState, Defeats);
}

void AMainPlayerState::UpdateScore(const float& ScoreToAdd)
{
	float CurrentScore = GetScore();
	SetScore(CurrentScore += ScoreToAdd);
	
	Character = !Character ? Cast<ATPSCharacterBase>(GetPawn())  : Character;
	if (Character)
	{
		Controller = !Controller ? Cast<ATpsPlayerController>(Character->Controller)  : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMainPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = !Character ? Cast<ATPSCharacterBase>(GetPawn())  : Character;
	if (Character)
	{
		Controller = !Controller ? Cast<ATpsPlayerController>(Character->Controller)  : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void AMainPlayerState::UpdateDefeats(const int32& DefeatsAmount)
{
	Defeats += DefeatsAmount;
	
	Character = !Character ? Cast<ATPSCharacterBase>(GetPawn())  : Character;
	if (Character)
	{
		Controller = !Controller ? Cast<ATpsPlayerController>(Character->Controller)  : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void AMainPlayerState::OnRep_Defeats()
{
	Character = !Character ? Cast<ATPSCharacterBase>(GetPawn())  : Character;
	if (Character)
	{
		Controller = !Controller ? Cast<ATpsPlayerController>(Character->Controller)  : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

