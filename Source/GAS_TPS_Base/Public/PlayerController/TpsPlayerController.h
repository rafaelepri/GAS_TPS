// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TpsPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class GAS_TPS_BASE_API ATpsPlayerController : public APlayerController {
	GENERATED_BODY()

public:
	void SetHUDHealth(const float& Health, const float& MaxHealth);
	void SetHUDScore(const float& Score);
	void SetHUDDefeats(const int32& Defeats);
	void SetHUDWeaponAmmo(const int32& Ammo);
	void SetHUDCarryingAmmo(const int32& Ammo);
	void SetHUDMatchCountdown(const float& CountdownTime);
	void SetHUDAnnouncementCountdown(const float& CountdownTime);
	
	virtual void OnPossess(APawn* InPawn) override;

	void OnMatchStateSet(FName State);
	void OnMatchStart();
	void OnMatchEnd();
protected:	
	virtual void BeginPlay() override;
	virtual void Tick(const float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void SetHUDTime();
	void PollInit();

	/*
	 *	Sync time between client and server
	 */
	UFUNCTION(Server, Reliable) // Requests current server time, passing clients time when request was sent
	void ServerRequestServerTime(const float& TimeOfClientRequest);

	UFUNCTION(Client, Reliable) // Reports the current server time to the client in response to ServerRequestServerTime
	void ClientReportServerTime(const float& TimeOfClientRequest, const float& TimeServerReceivedClientRequest);

	float ClientServerDelta = 0.f; // difference between client and server time
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(const float& DeltaTime);
	
	virtual float GetServerTime() const; // synced with server world clock
	virtual void ReceivedPlayer() override; // sync with server clock as soon as possible

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	UFUNCTION(Client, Reliable)
	void ClientJoinMidGame(const FName& StateOfMatch, const float& Warmup, const float& Match, const float& StartingTime, const float& Cooldown);
private:
	UPROPERTY()
	class ATpsHUD* TpsHUD;
	UPROPERTY()
	class AMainGameMode* MainGameMode;

	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	float CooldownTime = 0.f;
	uint32 CountdownInt = 0;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

	float HUDHealth;
	float HUDMaxHealth;
	float HUDScore;
	int32 HUDDefeats;
	float HUDCarryingAmmo;
	bool bInitializeCarryingAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
};

