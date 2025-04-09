// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/PlayerCharacterAnimInstance.h"

#include "AnimationWarpingLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Character/Player/PlayerCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void FASPlayerAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	Owner = InAnimInstance->TryGetPawnOwner();

	if (Owner)
	{
		Character = Cast<APlayerCharacter>(Owner);
		MovementComponent = Cast<UCharacterMovementComponent>(Owner->GetMovementComponent());
	}
}

void FASPlayerAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FASPlayerAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!Proxy.Character) return;
	Character = Proxy.Character;
	CharacterMovementComponent = Proxy.MovementComponent;
}

void UPlayerCharacterAnimInstance::NativeThreadSafeUpdateAnimation(const float DeltaSeconds) {
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Owner || !Character) return;

	OffsetRootTranslationRadius = UKismetSystemLibrary::GetConsoleVariableBoolValue("DDCvar.OffsetRootBone.TranslationRadius");
	
	CurrentGait = Character->GetCurrentGait();

	UpdateEssentialValues(DeltaSeconds);
}

void UPlayerCharacterAnimInstance::UpdateEssentialValues(const float DeltaSeconds)
{
	if (!Proxy.MovementComponent) return;

	Acceleration = Proxy.MovementComponent->GetCurrentAcceleration();
	
	CharacterTransformLastFrame = CharacterTransform;
	CharacterTransform = Proxy.Character->GetActorTransform();

	RootTransform = FTransform(FRotator(RootOffsetTransform.Rotator().Pitch, RootOffsetTransform.Rotator().Yaw + 90.f,
				  RootOffsetTransform.Rotator().Roll), RootOffsetTransform.GetLocation(), FVector(1.f, 1.f, 1.f));

	ParentVelocityLastFrame = ParentVelocity;
	ParentVelocity = Proxy.MovementComponent->Velocity;

	MovementStateLastFrame = MovementState;
	MovementState = IsMoving() ? EMovementState::Moving : EMovementState::Idle;

	MovementModeLastFrame = MovementMode;
	switch (Proxy.MovementComponent->MovementMode)
	{
	case MOVE_None:
		MovementMode = EMovementType::OnGround;
		break;
	case MOVE_Walking:
		MovementMode = EMovementType::OnGround;
		break;
	case MOVE_NavWalking:
		MovementMode = EMovementType::OnGround;
		break;
	case MOVE_Falling:
		MovementMode = EMovementType::InAir;
		break;
	case MOVE_Swimming:
		break;
	case MOVE_Flying:
		break;
	case MOVE_Custom:
		break;
	case MOVE_MAX:
		break;
	default:
		MovementMode = EMovementType::OnGround;
		break;
	}

	CalculateAccelerationLean(DeltaSeconds);
}

bool UPlayerCharacterAnimInstance::IsMoving() const
{
	return Acceleration != FVector::ZeroVector;
}

void UPlayerCharacterAnimInstance::CalculateAccelerationLean(const float DeltaSeconds)
{
	const FVector TempVector = (ParentVelocity - ParentVelocityLastFrame) / DeltaSeconds;
	if (Acceleration.Size() / Proxy.MovementComponent->GetMaxAcceleration() > 0.f) // has acceleration
	{
		VelocityAcceleration = UKismetMathLibrary::Quat_UnrotateVector(CharacterTransform.GetRotation(),
							  (UKismetMathLibrary::Vector_ClampSizeMax(TempVector, Proxy.MovementComponent->GetMaxAcceleration()) /
							   Proxy.MovementComponent->GetMaxAcceleration()));
		return;
	}
	VelocityAcceleration = UKismetMathLibrary::Quat_UnrotateVector(CharacterTransform.GetRotation(),
						  (UKismetMathLibrary::Vector_ClampSizeMax(TempVector, Proxy.MovementComponent->GetMaxBrakingDeceleration()) /
						   Proxy.MovementComponent->GetMaxBrakingDeceleration()));
}

bool UPlayerCharacterAnimInstance::EnableAO() const
{
	if (!Proxy.Owner) return false;
	const float TestFloat = (MovementState == EMovementState::Idle) ? 115.f : 180.f;
	return AO_Threshold(TestFloat) && GetSlotMontageLocalWeight("DefaultSlot") < 0.5f;
}

bool UPlayerCharacterAnimInstance::AO_Threshold(const float& Threshold) const
{
	return UKismetMathLibrary::Abs(GetAOValue().X) <= Threshold;
}

FVector UPlayerCharacterAnimInstance::GetAOValue() const
{
	if (!Proxy.Owner) return FVector::ZeroVector;

	// const FRotator NewRotator = UKismetMathLibrary::NormalizedDeltaRotator((Proxy.Character->IsLocallyControlled()) ?
	// Proxy.Character->GetControlRotation() : Proxy.Character->GetBaseAimRotation(), RootTransform.Rotator());
	
	const FRotator NewRotator = UKismetMathLibrary::NormalizedDeltaRotator(Proxy.Character->GetBaseAimRotation() , RootTransform.Rotator());
	const FVector AO_Value = UKismetMathLibrary::VLerp(FVector(NewRotator.Yaw, NewRotator.Pitch, 0.f), FVector::ZeroVector, GetCurveValue("Disable_AO"));

	return AO_Value;
}

EOffsetRootBoneMode UPlayerCharacterAnimInstance::GetOffsetRootRotationMode() const
{
	return IsMoving() ? EOffsetRootBoneMode::LockOffsetAndIgnoreAnimation : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UPlayerCharacterAnimInstance::GetOffsetRootTranslationMode() const
{
	// if (IsSlotActive("DefaultSlot")) return EOffsetRootBoneMode::Release;
	return IsMoving() ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;
}

float UPlayerCharacterAnimInstance::GetOffsetRootTranslationHalfLife() const
{
	switch (MovementState)
	{
	case EMovementState::Idle: return 0.1f;
	case EMovementState::Moving: return 0.3f;
	default: return 0.f;
	}
}

float UPlayerCharacterAnimInstance::GetOffsetRootTranslationRadius() const
{
	return OffsetRootTranslationRadius;
}

void UPlayerCharacterAnimInstance::UpdateOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	RootOffsetTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);
}

bool UPlayerCharacterAnimInstance::ShouldTurnInPlace() const
{
	if (!Proxy.Character) return false;

	const double AbsoluteRotationYaw = UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.Rotator(),
	RootTransform.Rotator()).Yaw);
	
	// const FString FloatString = FString::Printf(TEXT("ABS Rotation Yaw value: %.2f"), variable );
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FloatString);

	return AbsoluteRotationYaw >= 50.0f && MovementState == EMovementState::Idle &&
		MovementStateLastFrame == EMovementState::Moving;
}