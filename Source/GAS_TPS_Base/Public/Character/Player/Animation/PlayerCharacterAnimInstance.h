// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GAS_TPS_Base.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"

#include "BoneControllers/AnimNode_OrientationWarping.h"
#include "BoneControllers/AnimNode_OffsetRootBone.h"

#include "PlayerCharacterAnimInstance.generated.h"

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
	
	UPROPERTY(BlueprintReadOnly, Category = "States")
	EGait CurrentGait;

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

	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	float OffsetRootTranslationRadius = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "CVar")
	bool OffsetRootBoneEnabled = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	FVector Acceleration = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "MovementAnalysis", meta = (BlueprintThreadSafe))
	bool IsMoving() const;
	
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
	float GetOffsetRootTranslationHalfLife() const;
	UFUNCTION(BlueprintCallable, Category = "RootOffset", meta = (BlueprintThreadSafe))
	float GetOffsetRootTranslationRadius() const;
	UFUNCTION(Category = "RootOffset", BlueprintCallable, meta = (BlueprintThreadSafe))
	void UpdateOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node);
};
