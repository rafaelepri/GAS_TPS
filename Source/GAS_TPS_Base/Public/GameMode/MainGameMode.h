

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

namespace MatchState
{
	extern GAS_TPS_BASE_API const FName Cooldown; // Match duration reached. Display winner and begin timer
}

UCLASS()
class GAS_TPS_BASE_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AMainGameMode();
	virtual void Tick(const float DeltaSeconds) override;
	
	virtual void PlayerEliminated(class ATPSCharacterBase* EliminatedCharacter, class ATpsPlayerController* VictimController, ATpsPlayerController* KillerController);
	virtual void RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet()override;
	

private:
	float CountdownTime = 0.f;

public:
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};


