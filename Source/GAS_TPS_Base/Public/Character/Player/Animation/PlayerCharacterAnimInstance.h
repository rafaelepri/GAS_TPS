// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GAS_TPS_Base.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"


#include "BoneControllers/AnimNode_OffsetRootBone.h"
#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "PoseSearch/PoseSearchLibrary.h"

#include "PlayerCharacterAnimInstance.generated.h"

class UPoseSearchDatabase;
class UChooserTable;
struct FAnimUpdateContext;
struct FAnimNodeReference;

USTRUCT()
struct FASPlayerAnimInstanceProxy : public FAnimInstanceProxy
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
	class APlayerCharacter* Character;
	UPROPERTY(Transient)
	class UCharacterMovementComponent* MovementComponent;
};

UCLASS()
class GAS_TPS_BASE_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)
	FASPlayerAnimInstanceProxy Proxy;
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override { return &Proxy; }
	virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* InProxy) override {}

	virtual void NativeInitializeAnimation() override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	void UpdateEssentialValues(float DeltaSeconds);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	APlayerCharacter* Character = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	UCharacterMovementComponent* CharacterMovementComponent = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_PastVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_CurrentVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FVector Trj_FutureVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MotionMatching")
	FPoseSearchQueryTrajectory Trajectory;

	UPROPERTY(BlueprintReadOnly, Category = "States")
	EGait CurrentGaitLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EGait CurrentGait;

	UPROPERTY(BlueprintReadOnly, Category = "States")
	EStance StanceLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EStance Stance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector ParentVelocityLastFrame = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector ParentVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector VelocityAcceleration = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform CharacterTransformLastFrame;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform CharacterTransform;

	FTransform RootOffsetTransform;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FTransform RootTransform;

	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementState MovementStateLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementState MovementState;

	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementType MovementModeLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EMovementType MovementMode;

	ERotationMode RotationModeLastFrame;
	UPROPERTY(BlueprintReadOnly, Category = "States")
	ERotationMode RotationMode;

	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	float OffsetRootTranslationRadius = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	bool OffsetRootBoneEnabled = false;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	int32 MMDatabaseLOD = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector VelocityLastFrame = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	float Speed2D = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Data")
	bool HasVelocity = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector LastNonZeroVelocity = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector Acceleration = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	const UChooserTable* LocomotionTable;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	FPoseSearchTrajectoryData TrajectoryGenerationData_Idle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MotionMatching")
	FPoseSearchTrajectoryData TrajectoryGenerationData_Moving;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	TWeakObjectPtr<const UPoseSearchDatabase> SelectedDatabase;
	UPROPERTY(BlueprintReadWrite, Category = "MotionMatching")
	TArray<FName> DatabaseTags;
	UPROPERTY(BlueprintReadOnly, Category = "StateMachine")
	FBlendStackInputs BlendStackInputs;

	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float OrientationAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float TimeToLand = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	float AnimTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "MotionMatching")
	UAnimationAsset* AnimAsset = nullptr;

	float PreviousDesiredControlYaw = 0.f;

	UFUNCTION(Category = "MotionMatching", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(Category = "MotionMatching", BlueprintCallable, meta = (BlueprintThreadSafe))
	void MotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	EPoseSearchInterruptMode GetMMInteruptMode() const;
	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	float Get_MMNotifyRecencyTimeOut() const;
	UFUNCTION(BlueprintCallable, Category = "MotionMatching", meta = (BlueprintThreadSafe))
	float GetMMBlendTime() const;

	UFUNCTION(BlueprintCallable, Category = "Locomotion", meta = (BlueprintThreadSafe))
	void GenerateTrajectory(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	float GetTrajectoryTurnAngle() const;
	

	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsMoving() const;
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsPivoting() const;
	
	void CalculateAccelerationLean(float DeltaSeconds);
	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool ShouldTurnInPlace() const;

	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool EnableAO() const;
	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	bool AO_Threshold(const float& Threshold) const;
	UFUNCTION(BlueprintCallable, Category = "AimOffset", meta = (BlueprintThreadSafe))
	FVector GetAOValue() const;

	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootRotationMode() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOffsetRootBoneMode GetOffsetRootTranslationMode() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	EOrientationWarpingSpace GetOrientationWarpingSpace() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationRadius() const;
	UFUNCTION(Category = "RootOffset", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);

	// BlensStack Graph
	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	FQuat GetDesiredFacing() const;
	UFUNCTION(Category = "BlendStack", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	bool EnableSteering() const;
	UFUNCTION(BlueprintCallable, Category = "BlendStack", meta = (BlueprintThreadSafe))
	bool IsTurningInPlace() const;
};
