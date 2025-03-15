// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/TpsPlayerController.h"
#include "HUD/TpsHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Character/TpsCharacterBase.h"
#include "GameFramework/GameMode.h"
#include "GameMode/MainGameMode.h"
#include "HUD/Widget/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameState/MainGameState.h"
#include "PlayerState/MainPlayerState.h"

void ATpsPlayerController::BeginPlay()
{
	Super::BeginPlay();

	TpsHUD = Cast<ATpsHUD>(GetHUD());

	ServerCheckMatchState();
}

void ATpsPlayerController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ATpsPlayerController::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATpsPlayerController, MatchState);
}

void ATpsPlayerController::PollInit()
{
	if (!CharacterOverlay)
	{
		if (TpsHUD && TpsHUD->CharacterOverlay)
		{
			CharacterOverlay = TpsHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);

				if (bInitializeCarryingAmmo)
				{	
					SetHUDCarryingAmmo(HUDCarryingAmmo);
				}

				if (bInitializeWeaponAmmo)
				{
					SetHUDWeaponAmmo(HUDWeaponAmmo);
				}
			}
		}
	}	
}


void ATpsPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ATPSCharacterBase* TpsCharacter = Cast<ATPSCharacterBase>(InPawn))
	{
		SetHUDHealth(TpsCharacter->GetHealth(), TpsCharacter->GetMaxHealth());
		TpsCharacter->UpdateHUDAmmo();
		TpsCharacter->SpawnDefaultWeapon();
	}
}

void ATpsPlayerController::SetHUDHealth(const float& Health, const float& MaxHealth)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;
	
	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->HealthBar &&
		TpsHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercentage = Health / MaxHealth;
		TpsHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercentage);

		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		TpsHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	} else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ATpsPlayerController::SetHUDScore(const float& Score)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->ScoreAmount)
	{
		const FString ScoreText = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
		TpsHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	} else
	{
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ATpsPlayerController::SetHUDDefeats(const int32& Defeats)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->DefeatsAmount)
	{
		const FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		TpsHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	} else
	{
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ATpsPlayerController::SetHUDWeaponAmmo(const int32& Ammo)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->WeaponAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TpsHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	} else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void ATpsPlayerController::SetHUDCarryingAmmo(const int32& Ammo)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->CarryingAmmoAmount)
	{
		const FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		TpsHUD->CharacterOverlay->CarryingAmmoAmount->SetText(FText::FromString(AmmoText));
	} else
	{
		bInitializeCarryingAmmo = true;
		HUDCarryingAmmo = Ammo;
	}
}

void ATpsPlayerController::SetHUDMatchCountdown(const float& CountdownTime)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->CharacterOverlay &&
		TpsHUD->CharacterOverlay->MatchCountdownText)
	{
		if (CountdownTime < 0.f)
		{
			TpsHUD->CharacterOverlay->MatchCountdownText->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TpsHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ATpsPlayerController::SetHUDAnnouncementCountdown(const float& CountdownTime)
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;

	if (TpsHUD &&
		TpsHUD->AnnouncementOverlay &&
		TpsHUD->AnnouncementOverlay->WarmupTime)
	{
		if (CountdownTime < 0.f)
		{
			TpsHUD->AnnouncementOverlay->WarmupTime->SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		
		const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		const int32 Seconds = CountdownTime - Minutes * 60;
		
		const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		TpsHUD->AnnouncementOverlay->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ATpsPlayerController::SetHUDTime()
{
	const float ServerTime = GetServerTime();
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - ServerTime + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - ServerTime + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - ServerTime + LevelStartingTime;
	
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		MainGameMode = !MainGameMode ? Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this)) : MainGameMode;
		if (MainGameMode)
		{
			SecondsLeft = FMath::CeilToInt(MainGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}
	
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(SecondsLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	CountdownInt = SecondsLeft;
}



void ATpsPlayerController::ServerRequestServerTime_Implementation(const float& TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ATpsPlayerController::ClientReportServerTime_Implementation(const float& TimeOfClientRequest,
	const float& TimeServerReceivedClientRequest)
{
	const float WorldTimeSeconds = GetWorld()->GetTimeSeconds();
	
	const float RoundTripTime = WorldTimeSeconds - TimeOfClientRequest;
	const float CurrentServerTime = TimeServerReceivedClientRequest + (0.5f * RoundTripTime);

	ClientServerDelta = CurrentServerTime - WorldTimeSeconds;
}

float ATpsPlayerController::GetServerTime() const
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ATpsPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}



void ATpsPlayerController::CheckTimeSync(const float& DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;;
	}
}

void ATpsPlayerController::OnMatchStateSet(const FName State)
{
	MatchState = State;
	
	if (MatchState == MatchState::InProgress)
	{
		OnMatchStart();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		OnMatchEnd();
	}
}

void ATpsPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		OnMatchStart();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		OnMatchEnd();
	}
}

void ATpsPlayerController::OnMatchStart()
{
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;
	if (TpsHUD)
	{
		if (TpsHUD->AnnouncementOverlay)
		{
			TpsHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Hidden);
		}
			
		TpsHUD->AddCharacterOverlay();
	}
}

void ATpsPlayerController::OnMatchEnd()
{
	if (ATPSCharacterBase* CharacterBase = Cast<ATPSCharacterBase>( GetPawn()))
	{
		CharacterBase->bDisableGameplay = true;

		if (CharacterBase->GetCombatComponent())
		{
			CharacterBase->GetCombatComponent()->FireButtonPressed(false);
		}
	}
	
	TpsHUD = !TpsHUD ? Cast<ATpsHUD>(GetHUD()) : TpsHUD;
	if (TpsHUD)
	{
		TpsHUD->CharacterOverlay->RemoveFromParent();
		if (TpsHUD->AnnouncementOverlay && TpsHUD->AnnouncementOverlay->AnnouncementText && TpsHUD->AnnouncementOverlay->InfoText)
		{
			TpsHUD->AnnouncementOverlay->SetVisibility(ESlateVisibility::Visible);
			const FString AnnouncementText("New Match Starts In: ");
			TpsHUD->AnnouncementOverlay->AnnouncementText->SetText(FText::FromString(AnnouncementText));


			const AMainGameState* MainGameState = Cast<AMainGameState>(UGameplayStatics::GetGameState(this));
			const AMainPlayerState* MainPlayerState = GetPlayerState<AMainPlayerState>();

			if (MainGameState && MainPlayerState)
			{
				
				FString InfoTextString;
				
				TArray<AMainPlayerState*> TopPlayers = MainGameState->TopScoringPlayers;
				if (TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == MainPlayerState)
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				} else
				{
					InfoTextString = FString("Players tied for the win: \n");
					for (auto const TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s"), *TiedPlayer->GetPlayerName()));
					}
				}
				

				TpsHUD->AnnouncementOverlay->InfoText->SetText(FText::FromString(InfoTextString));
			}
			
		}
	}
}

void ATpsPlayerController::ServerCheckMatchState_Implementation()
{
	if (const AMainGameMode* GameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void ATpsPlayerController::ClientJoinMidGame_Implementation(const FName& StateOfMatch, const float& Warmup, const float& Match, const float& StartingTime, const float& Cooldown)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	
	OnMatchStateSet(MatchState);

	if (TpsHUD && MatchState == MatchState::WaitingToStart)
	{
		TpsHUD->AddAnnouncementOverlay();
	}
}

