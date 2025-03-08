﻿#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

UCLASS()
class UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarryingAmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;
};
