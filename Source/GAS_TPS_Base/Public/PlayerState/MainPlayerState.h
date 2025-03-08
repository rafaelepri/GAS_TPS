
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MainPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class GAS_TPS_BASE_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void OnRep_Score() override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	
	void UpdateScore(const float& ScoreToAdd);
	void UpdateDefeats(const int32& DefeatsAmount);
private:
	
	
	UPROPERTY()
	class ATPSCharacterBase* Character;
	UPROPERTY()
	class ATpsPlayerController* Controller;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;
};
