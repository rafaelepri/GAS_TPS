#include "GameMode/MainGameMode.h"

#include "Character/TpsCharacterBase.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerState/MainPlayerState.h"
#include "PlayerController/TpsPlayerController.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AMainGameMode::AMainGameMode()
{
	DefaultPawnClass = ATPSCharacterBase::StaticClass();

	bDelayedStart = true;
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMainGameMode::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	} else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	} else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void AMainGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ATpsPlayerController* PlayerController = Cast<ATpsPlayerController>(*It))
		{
			PlayerController->OnMatchStateSet(MatchState);
		}
	}
}


void AMainGameMode::PlayerEliminated(ATPSCharacterBase* EliminatedCharacter,
                                     ATpsPlayerController* VictimController, ATpsPlayerController* KillerController)
{
	AMainPlayerState* KillerPlayerState = KillerController ? Cast<AMainPlayerState>(KillerController->PlayerState) : nullptr;
	AMainPlayerState* VictimPlayerState = VictimController ? Cast<AMainPlayerState>(VictimController->PlayerState) : nullptr;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->UpdateScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->UpdateDefeats(1);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Eliminated();
	}
}

void AMainGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		// since we want to respawn call this Reset();
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		TArray<AActor*> PlayerStartList;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartList); // this populates the Actors array
		const int32 Selection = FMath::RandRange(0, PlayerStartList.Num() - 1); // random spawn point
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStartList[Selection]);
	}
}


