// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chooser.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "GAS_TPS_Base.h"
#include "Interfaces/InteractionInterface.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchTrajectoryTypes.h"
#include "PoseSearch/PoseSearchTrajectoryLibrary.h"
#include "Animation/AnimExecutionContext.h"
#include "Animation/AnimNodeReference.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimInstance.h"
#include "AnimInstanceBase.generated.h"

/**
 * 
 */

USTRUCT()
struct FASAnimInstanceProxy : public FAnimInstanceProxy
{
	GENERATED_BODY()

protected:

	virtual void InitializeObjects(UAnimInstance* InAnimInstance) override;
	virtual void PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds) override;
	virtual void Update(float DeltaSeconds) override;

public:

	UPROPERTY(Transient)
	APawn* Owner;
	UPROPERTY(Transient)
	class ATPSCharacterBase* Character;
	UPROPERTY(Transient)
	class UCharacterMovementComponent* MovementComponent;

};


UCLASS()
class GAS_TPS_BASE_API UAnimInstanceBase : public UAnimInstance, public IInteractionInterface {
	GENERATED_BODY()

protected:

	UPROPERTY(Transient)
	FASAnimInstanceProxy Proxy;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &Proxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	// virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Locomotion", meta = (BlueprintThreadSafe))
	void GenerateTrajectory(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Locomotion", meta = (BlueprintThreadSafe))
	void UpdateEssentialValues(const float& DeltaSeconds);
	void CalculateAccelerationLean(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "Locomotion", meta = (BlueprintThreadSafe))
	void UpdateStates();

	UFUNCTION(BlueprintCallable, Category = "System", meta = (BlueprintThreadSafe))
	void UpdateCVars();

	UFUNCTION(BlueprintCallable, Category = "StateMachine Locomotion", meta = (BlueprintThreadSafe))
	void UpdateMovementDirection();
	UFUNCTION(BlueprintCallable, Category = "StateMachine Locomotion", meta = (BlueprintThreadSafe))
	FMovementDirectionThreshold GetMovementDirectionThreshold() const;
	UFUNCTION(BlueprintCallable, Category = "StateMachine Locomotion", meta = (BlueprintThreadSafe))
	void UpdateTargetRotation();
	UFUNCTION(BlueprintCallable, Category = "StateMachine Locomotion", meta = (BlueprintThreadSafe))
	float GetStrafeYawRotationOffset() const;


	// Root Offset

	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationRadius() const;

	UFUNCTION(Category = "RootOffset", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);


	// State Node Functions

	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	float Get_MMNotifyRecencyTimeOut() const;
	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	float GetMMBlendTime() const;
	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMMInterupMode() const;

	UFUNCTION(Category = "MotionMatching", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(Category = "MotionMatching", BlueprintCallable, meta = (BlueprintThreadSafe))
	void MotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);



	// AimOffset

	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool EnableAO() const;
	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool AO_Threshold(const float& Threshold) const;
	
	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	FVector GetAOValue() const;

	// Movement Analysis

	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsMoving() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsStarting() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsPivoting() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsPivoting_SM() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsPivoting_MM() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool ShouldSpinTransition() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustTraversed() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustLanded_Light() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool JustLanded_Heavy() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	float GetLandVelocity() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool PlayLand() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool PlayMovingLand() const;




	// BlensStack Graph

	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;
	UFUNCTION(Category = "BlendStack", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	bool EnableSteering() const;
	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	bool IsTurningInPlace() const;



public:

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	FTransform InteractionTransform;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	const UChooserTable* LocomotionTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_PastVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_CurrentVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_FutureVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FPoseSearchQueryTrajectory Trajectory;

	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	TWeakObjectPtr<const UPoseSearchDatabase> SelectedDatabase;
	UPROPERTY(BlueprintReadWrite, Category = "MotionMatching")
	TArray<FName> DatabaseTags;

	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float OrientationAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float TimeToLand = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float AnimTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	UAnimationAsset* AnimAsset = nullptr;




	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	ATPSCharacterBase* Character = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UCharacterMovementComponent* CharacterMovementComponent = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform CharacterTransformLastFrame;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform CharacterTransform;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform RootTransform;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector AccelerationLastFrame = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector Acceleration = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	float AccelerationAmount = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	bool HasAcceleration = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector VelocityLastFrame = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	float Speed2D = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	bool HasVelocity = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector VelocityAcceleration = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector LastNonZeroVelocity = FVector::ZeroVector;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float MaxTurnAngle = 50.f; // set this to 50.f if using devop overlay system 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	float HeavyLandSppedThreshold = 700.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Settings")
	EMovementDirectionBias MovementDirectionBias;
	
	FMovementDirectionThreshold OutThreshold1;
	FMovementDirectionThreshold OutThreshold2;
	FMovementDirectionThreshold OutThreshold3;

	float PreviousDesiredControllYaw = 0.f;


	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementType MovementModeLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementType MovementMode;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	ERotationMode RotationModeLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	ERotationMode RotationMode;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementState MovementStateLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementState MovementState;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EGait GaitLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EGait Gait;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EStance StanceLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EStance Stance;


	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	float OffsetRootTranslationRadius = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	bool OffsetRootBoneEnabled = false;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	int32 MMDatabaseLOD = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	bool UseExperimentalStateMachine = false;


	FTransform RootOffsetTransform;



	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	EMovementDirection MovementDirection;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	EMovementDirection MovementDirectionLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	float Direction = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	FBlendStackInputs BlendStackInputs;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	FMovementDirectionThreshold MovementDirectionThresholds;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	FRotator TargetRotation = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	float TargetRotationDelta = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	UAnimSequence* CurveContainer = nullptr;






	// Interface

	UFUNCTION()
	virtual void SetInteractionTransform(FTransform InTransform) override;
	UFUNCTION()
	virtual FTransform GetInteractionTransform() override;
	UFUNCTION()
	virtual void PlayTraversalMontage(UAnimMontage* MontageToPlay, float PlayRate, float StartTime) override;


	// Montage And Delegates

	void FunctionToExecuteOnAnimationBlendOut(UAnimMontage* animMontage, bool bInterrupted);
	void FunctionToExecuteOnAnimationEnd(UAnimMontage* animMontage, bool bInterrupted);
	void PlayAnAnimationMontage(UAnimMontage* montageToPlay, float PlayRate, float StartTime);



	// AIMING FUNCTIONALITY

	UPROPERTY(BlueprintReadOnly)
	bool bCanAim;
	UFUNCTION()
	void CanAim();

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overlay")
	float AdditiveOverlayWeight = 1.0f;
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Overlay")
	float AdditiveOverlayStrength = 0.75f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overlay")
	float AdditiveOverlayInterpSpeed = 7.0f;
	UFUNCTION()
	bool CanOverlayAim() const;


	

	UPROPERTY(BlueprintReadOnly)
	FRotator SpineRotation = FRotator::ZeroRotator;
	UFUNCTION()
	void UpdateSpineRotation(const float DeltaSeconds);

	UPROPERTY(BlueprintReadOnly, Category = "Spine Rotation")
	float TargetSpineRoll = 0.f; // Raw clamped value (not smoothed)
	UPROPERTY(BlueprintReadOnly, Category = "Spine Rotation")
	float SmoothedSpineRoll = 0.f; // Interpolated value
	UPROPERTY(EditDefaultsOnly, Category = "Spine Rotation")
	float SpineInterpSpeed = 20.f; // Adjust for smoothness (higher = faster)


	UPROPERTY(BlueprintReadOnly)
	bool bIsReloading;
};
