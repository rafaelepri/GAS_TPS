// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GAS_TPS_Base/Public/GAS_TPS_Base.h"
#include "Interfaces/CrosshairInteractionInterface.h"
#include "Components/Enums/CombatState.h"
#include "TpsCharacterBase.generated.h"

class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class UMotionWarpingComponent;
class UPreCmcTick;
class AWeapon;
class UCombatComponent;
struct FInputActionValue;

class FDelayLatentAction : public FPendingLatentAction {
	
public:
	float TimeRemaining;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;

	FDelayLatentAction(const float Duration, const FLatentActionInfo& LatentInfo)
		: TimeRemaining(Duration)
		, ExecutionFunction(LatentInfo.ExecutionFunction)
		, OutputLink(LatentInfo.Linkage)
		, CallbackTarget(LatentInfo.CallbackTarget)
	{
	}

	virtual void UpdateOperation(FLatentResponse& Response) override {
		TimeRemaining -= Response.ElapsedTime();
		Response.FinishAndTriggerIf(TimeRemaining <= 0.0f, ExecutionFunction, OutputLink, CallbackTarget);
	}
};

USTRUCT(BlueprintType)
struct FCharacterInputState {
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Character Input State")
	bool bWantsToSprint = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Character Input State")
	bool bWantsToWalk = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Character Input State")
	bool bWantsToStrafe = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Input|Character Input State")
	bool bWantsToAim = false;
};

USTRUCT(BlueprintType)
struct FCharacterWeaponState {
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "Weapons|CharacterWeaponState")
	bool bHasMelee = false;
	UPROPERTY(BlueprintReadOnly, Category = "Weapons|CharacterWeaponState")
	bool bHasRifle = false;
	UPROPERTY(BlueprintReadOnly, Category = "Weapons|CharacterWeaponState")
	bool bHasPistol = false;
};

USTRUCT(BlueprintType)
struct FCameraParameters {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmLength = 300.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector SocketOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float TranslationLagSpeed = 10.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float FieldOfView = 90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float TransitionSpeed = 5.0f;
};

UCLASS(Blueprintable, BlueprintType)
class GAS_TPS_BASE_API ATPSCharacterBase : public ACharacter, public ICrosshairInteractionInterface {
	GENERATED_BODY()

	/** SpringArm positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> SpringArm;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* IMC_Default;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	/** Sprint Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;

	/** Walk Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	/** Aim Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* AimAction;

	/** Pickup Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PickupAction;

	/** Reload Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ReloadAction;

	/** Fire/Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FireAction;
public:
	ATPSCharacterBase();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	UFUNCTION(Server, Unreliable, Category = "Replication")
	void Server_SetCharacterInputState(const FCharacterInputState NewState);
	UFUNCTION(Server, Unreliable, Category = "Replication")
	void Server_SetCharacterWeaponState(const FCharacterWeaponState NewState);
	UFUNCTION(BlueprintCallable, Server, Unreliable, Category = "Replication")
	void Server_Traversal(const FTraversalParams InTraversalParams);
	UFUNCTION(Server, Reliable, Category = "Replication")
	void Server_PickupAction();
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
	TObjectPtr<UMotionWarpingComponent> MotionWarping;
	UPROPERTY(VisibleAnywhere, Category = "Character")
	TObjectPtr<UPreCmcTick> PreCmcTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	EGait CurrentGait = EGait::Run;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector WalkSpeeds = FVector(200.0f, 180.0f, 150.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector RunSpeeds = FVector(500.0f, 350.0f, 300.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector SprintSpeeds = FVector(700.0f, 700.0f, 700.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector CrouchSpeeds = FVector(225.0f, 200.0f, 180.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AnalogWalkRunThreshold = 0.7;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bJustLanded = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bUsingAttributeBasedRootMotion = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FCameraParameters CameraStyleThirdPersonFar;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FCameraParameters CameraStyleThirdPersonClose;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FCameraParameters CameraStyleThirdPersonAim;

	class AMainPlayerState* MainPlayerState;

	////////////////////////////////////////////////////////	INPUT
	///
	virtual void NotifyControllerChanged() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void ToggleWalk(const FInputActionValue& Value);
	void ToggleCrouch(const FInputActionValue& Value);
	void StartSprint();
	void EndSprint();
	void StartAiming();
	void EndAiming();
	void PickUpAction();
	void StartFiring();
	void EndFiring();
	void Reload_Action();

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	/////////////////////////////////////////////////////////// MOVEMENT HELPER FUNCTIONS
	///
	UFUNCTION(BlueprintPure, Category="Movement|HelperFn")
	double CalculateMaxAcceleration(const FVector& CharacterVelocity) const;
	UFUNCTION(BlueprintPure, Category="Movement|HelperFn")
	double CalculateBrakingDeceleration() const;
	UFUNCTION(BlueprintPure, Category="Movement|HelperFn")
	double CalculateMaxSpeed(const FVector& CharacterVelocity) const;
	UFUNCTION(BlueprintPure, Category="Movement|HelperFn")
	virtual EGait GetDesiredGait(const FVector& CurrentAcceleration) const;
	UFUNCTION(BlueprintPure, Category="Movement|HelperFn")
	bool CanSprint(const FVector& CurrentAcceleration) const;


	/////////////////////////////////////////////////////////// MOVEMENT FUNCTIONS
	///
	///

	UFUNCTION(BlueprintCallable, Category="Movement")
	void UpdateRotation_PreCmc() const;
	UFUNCTION(BlueprintCallable, Category="Movement")
	void UpdateMovement_PreCmc();
	void UpdateMovementSimulated(const FVector& OldVelocity);
	UFUNCTION()
	virtual void UpdateCamera_PreCmc(const bool bInterpolate);
	virtual void Landed(const FHitResult& Hit) override;
	void FinishJump(const FVector& InVel);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayAudio(float NewVolume, bool Jumped);

	// Delay

	void CustomDelay(const UObject* WorldContextObject, const float Duration, const FLatentActionInfo& LatentInfo);
	UFUNCTION()
	void OnDelayCompleted();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	TSoftObjectPtr<UCurveFloat> StrafeSpeedMapCurve;
	UPROPERTY(Replicated, EditAnyWhere, BlueprintReadOnly, Category = "Data")
	FCharacterInputState CharacterInputState;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Movement | Simulated")
	bool WasMovingOnGroundLastFrame_Simulated;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Movement | Simulated")
	FVector LastUpdateVelocity;
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Movement")
	bool UsingAttributeBasedRootMotion;

	UPROPERTY(BlueprintReadOnly, Category = "Data")
	FVector LandVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	bool JustLanded = false;
	UPROPERTY(BlueprintReadOnly, Category = "Data")
	bool IsMovingOnGround = false;
	bool WasMovingOnGroundSimulated = false;

	////////////////////////////////////////////////////////////////////////////// ANIMATION
	///

	void OnMontageCompleted(const bool Value);


	////////////////////////////////////////////////////////////////////////////// TRAVERSAL ACTION FUNCTIONALITY
	///
	///

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	class UChooserTable* TraversalTable;

	UPROPERTY(BlueprintReadOnly)
	FTraversalParams TempTraversalParams;
	UPROPERTY(BlueprintReadWrite, ReplicatedUsing = OnRep_TraversalResult)
	FTraversalParams TraversalResult;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	FVector ActorLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	float CapsuleRadius = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	float CapsuleHalfHeight = 0.f;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	FVector HasRoomCheckFrontLedgeLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	FVector HasRoomCheckBackLedgeLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	FHitResult TopSweepResult;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	bool DoingTraversalAction = false;
	UPROPERTY(BlueprintReadWrite, Category = "Traversal")
	TArray<UObject*> ValidMontages;

	void StartTraversalAction();
	UFUNCTION(BlueprintCallable)
	bool TryTraversalAction();

	FTraversalCheckInputs GetTraversalInputs() const;
	UFUNCTION(BlueprintCallable)
	void CacheValues();
	FHitResult TraversalTrace(const FVector& Start, const FVector& End, const float Radius, const float HalfHeight) const;
	UFUNCTION(BlueprintCallable)
	bool StoreHitComponent();
	UFUNCTION(BlueprintCallable)
	bool HasRoom();
	UFUNCTION(BlueprintCallable)
	void SetObstacleHeight();
	UFUNCTION(BlueprintCallable)
	bool TopSweep();
	UFUNCTION(BlueprintCallable)
	void SetObstacleDepth(const bool HasRoom);
	UFUNCTION(BlueprintCallable)
	void DownwardTrace();

	UFUNCTION(BlueprintCallable)
	void GetAllMatchingMontages();
	UFUNCTION(BlueprintCallable)
	void SetInteractionTransform();
	UFUNCTION(BlueprintCallable)
	void SelectTraversalMontage();

	UFUNCTION(BlueprintCallable)
	void UpdateWarpTargets();
	UFUNCTION(BlueprintCallable)
	void PerformTraversalAction();
	UFUNCTION()
	void OnRep_TraversalResult();
	void OnTraversal(const bool NewBool) const;
	UFUNCTION(BlueprintCallable)
	void SetAnimParam(UAnimMontage* InMontage);

	////////////////////////////////////////////////////////////////////////////// WEAPONS & COMBAT FUNCTIONALITY
	///

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
	UCombatComponent* Combat;
	
	UPROPERTY(Replicated, EditAnyWhere, BlueprintReadOnly, Category = "Data")
	FCharacterWeaponState CharacterWeaponState;
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	AWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const;
	void SetOverlappingWeapon(AWeapon* Weapon);

	void HideCameraIfCharacterClose() const;
	UPROPERTY()
	float HideCameraThreshold = 50.f;

	////////////////////////////////////////////////////////////////////////////// PLAYER HEALTH
	///

	UPROPERTY()
	class ATpsPlayerController* TpsPlayerController;

	UPROPERTY(EditDefaultsOnly, Category = "PlayerStats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "PlayerStats")
	float Health = MaxHealth;

	UFUNCTION()
	void OnRep_Health();
	void HandleHUDHealth();
	
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

	// Eliminated functionality
	bool bIsEliminated = false;
	UPROPERTY(EditDefaultsOnly)
	float EliminatedDelay = 3.f;
	FTimerHandle EliminatedTimer;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminated();
	void Eliminated();
	void EliminatedTimerFinisher();

protected:
	// Poll for any relevant classes and initialize our HUD
	void PollInit();
	
public:
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	ECombatState GetCombatState() const;
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return Combat; }
};





