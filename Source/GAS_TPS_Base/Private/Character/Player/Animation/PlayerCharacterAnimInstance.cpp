// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/Animation/PlayerCharacterAnimInstance.h"
#include "Character/Player/PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"

#include "AnimationWarpingLibrary.h"

#include "ChooserFunctionLibrary.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchDatabase.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"

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

	OffsetRootBoneEnabled = UKismetSystemLibrary::GetConsoleVariableBoolValue("a.animnode.offsetrootbone.enable");
	OffsetRootTranslationRadius = UKismetSystemLibrary::GetConsoleVariableBoolValue("DDCvar.OffsetRootBone.TranslationRadius");

	CurrentGaitLastFrame = CurrentGait;
	CurrentGait = Character->GetCurrentGait();
	
	GenerateTrajectory(DeltaSeconds);
	UpdateEssentialValues(DeltaSeconds);

	CurrentGaitLastFrame = CurrentGait;
	CurrentGait = Character->GetCurrentGait();

}

void UPlayerCharacterAnimInstance::UpdateEssentialValues(const float DeltaSeconds)
{
	if (!Proxy.MovementComponent) return;

	CharacterTransformLastFrame = CharacterTransform;
	CharacterTransform = Proxy.Character->GetActorTransform();

	RootTransform = FTransform(FRotator(RootOffsetTransform.Rotator().Pitch, RootOffsetTransform.Rotator().Yaw + 90.f,
	RootOffsetTransform.Rotator().Roll), RootOffsetTransform.GetLocation(), FVector(1.f, 1.f, 1.f));

	Acceleration = Proxy.MovementComponent->GetCurrentAcceleration();

	VelocityLastFrame = Velocity;
	Velocity = Proxy.MovementComponent->Velocity;
	Speed2D = UKismetMathLibrary::VSizeXY(Velocity);

	HasVelocity = Speed2D > 5.f;
	if (HasVelocity) LastNonZeroVelocity = Velocity;
	

	ParentVelocityLastFrame = ParentVelocity;
	ParentVelocity = Proxy.MovementComponent->Velocity;

	MovementStateLastFrame = MovementState;
	MovementState = IsMoving() ? EMovementState::Moving : EMovementState::Idle;

	StanceLastFrame = Stance;
	Stance = Proxy.MovementComponent->IsCrouching() ? EStance::Crouch : EStance::Stand;

	RotationModeLastFrame = RotationMode;
	RotationMode = (Proxy.MovementComponent->bOrientRotationToMovement) ? ERotationMode::OrientToRotation : ERotationMode::Strafe;

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

void UPlayerCharacterAnimInstance::GenerateTrajectory(const float DeltaSeconds)
{
	FPoseSearchQueryTrajectory OutTrajectory;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTrajectory(this, (Speed2D > 0.f) ? TrajectoryGenerationData_Moving : TrajectoryGenerationData_Idle,
															  DeltaSeconds, Trajectory, PreviousDesiredControlYaw, OutTrajectory, -1.f, 30, 0.1f, 15);

	FPoseSearchTrajectory_WorldCollisionResults CollisionResult;
	const TArray<AActor*> IgnoredActors;
	UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions(Proxy.Character, this, OutTrajectory, true, 0.01f, Trajectory, CollisionResult,
													 ETraceTypeQuery::TraceTypeQuery1, false, IgnoredActors, EDrawDebugTrace::None, true, 150.f);

	TimeToLand = CollisionResult.TimeToLand;

	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, -0.3f, -0.2f, Trj_PastVelocity);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, 0.f, 0.2f, Trj_CurrentVelocity);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, 0.4f, 0.5f, Trj_FutureVelocity);
}

float UPlayerCharacterAnimInstance::GetTrajectoryTurnAngle() const
{
	return UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(Trj_FutureVelocity), UKismetMathLibrary::MakeRotFromX(Velocity)).Yaw;
}

bool UPlayerCharacterAnimInstance::IsMoving() const
{
	return Acceleration != FVector::ZeroVector && Trj_FutureVelocity != FVector::ZeroVector;
}

void UPlayerCharacterAnimInstance::CalculateAccelerationLean(const float DeltaSeconds)
{
	const FVector TempVector = (Velocity - VelocityLastFrame) / DeltaSeconds;
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
	return IsSlotActive("DefaultSlot") ? EOffsetRootBoneMode::LockOffsetAndIgnoreAnimation : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UPlayerCharacterAnimInstance::GetOffsetRootTranslationMode() const
{
	if (IsSlotActive("DefaultSlot")) return EOffsetRootBoneMode::Release;
	switch (MovementMode)
	{
	case EMovementType::OnGround:
		return IsMoving() ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;
	case EMovementType::InAir:
		return EOffsetRootBoneMode::Release;
	default:
		return EOffsetRootBoneMode();
	}
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

void UPlayerCharacterAnimInstance::UpdateMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node) {
	if (!LocomotionTable) return;

	EAnimNodeReferenceConversionResult Result{};
	const FMotionMatchingAnimNodeReference MM = UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Failed) return;

	TArray<UPoseSearchDatabase*> PoseSearchArray;
	const TArray<UObject*> ObjectArray = UChooserFunctionLibrary::EvaluateChooserMulti(this, LocomotionTable, UPoseSearchDatabase::StaticClass());
	PoseSearchArray.Reserve(ObjectArray.Num());  // Preallocate memory
	Algo::TransformIf(ObjectArray, PoseSearchArray,
		[](UObject* Obj) { return Cast<UPoseSearchDatabase>(Obj) != nullptr; },  // Condition to filter
		[](UObject* Obj) { return Cast<UPoseSearchDatabase>(Obj); }); // Transform function

	UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(MM, PoseSearchArray, GetMMInteruptMode());
}

void UPlayerCharacterAnimInstance::MotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	EAnimNodeReferenceConversionResult Result{};
	const FMotionMatchingAnimNodeReference MM = UMotionMatchingAnimNodeLibrary::ConvertToMotionMatchingNode(Node, Result);
	if (Result == EAnimNodeReferenceConversionResult::Failed) return;

	FPoseSearchBlueprintResult OutResult;
	bool IsValidResult;
	UMotionMatchingAnimNodeLibrary::GetMotionMatchingSearchResult(MM, OutResult, IsValidResult);
	SelectedDatabase = OutResult.SelectedDatabase;

	if (SelectedDatabase != nullptr)
	{
		DatabaseTags = SelectedDatabase->Tags;
	}
}

EPoseSearchInterruptMode UPlayerCharacterAnimInstance::GetMMInteruptMode() const
{
	return (MovementMode != MovementModeLastFrame ||
		   (MovementMode == EMovementType::OnGround && (MovementState != MovementStateLastFrame ||
		   (CurrentGait != CurrentGaitLastFrame && MovementState == EMovementState::Moving) || Stance != StanceLastFrame))) ?
			EPoseSearchInterruptMode::InterruptOnDatabaseChange : EPoseSearchInterruptMode::DoNotInterrupt;
}

float UPlayerCharacterAnimInstance::Get_MMNotifyRecencyTimeOut() const
{
	switch (CurrentGait)
	{
	case EGait::Walk: return 0.2f;
	case EGait::Run: return 0.2f;
	case EGait::Sprint: return 0.16f;
	default:
		return 0.0f;
	}
}

EOrientationWarpingSpace UPlayerCharacterAnimInstance::GetOrientationWarpingSpace() const
{
	return OffsetRootBoneEnabled ? EOrientationWarpingSpace::RootBoneTransform : EOrientationWarpingSpace::ComponentTransform;
}

// BlendStack

void UPlayerCharacterAnimInstance::UpdateBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	AnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	const UAnimSequence* NewAnimSequence = Cast<UAnimSequence>(AnimAsset);
	if (!NewAnimSequence) return;
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(NewAnimSequence, "Enable_OrientationWarping",
																		  AnimTime, OrientationAlpha);
}

bool UPlayerCharacterAnimInstance::EnableSteering() const
{
	return MovementState == EMovementState::Moving || MovementMode == EMovementType::InAir;
}

FQuat UPlayerCharacterAnimInstance::GetDesiredFacing() const
{
	return Trajectory.GetSampleAtTime(0.5f, false).Facing;
}

bool UPlayerCharacterAnimInstance::IsTurningInPlace() const
{
	return DatabaseTags.Contains("TurnInPlace");
}

bool UPlayerCharacterAnimInstance::IsPivoting() const
{
	const float TestFloat = (RotationMode == ERotationMode::OrientToRotation) ? 45.f : 30.f;
	return UKismetMathLibrary::Abs(GetTrajectoryTurnAngle()) >= TestFloat;
}

float UPlayerCharacterAnimInstance::GetMMBlendTime() const
{
	switch (MovementMode)
	{
	case EMovementType::OnGround:
		return (MovementModeLastFrame == EMovementType::OnGround) ? 0.5f : 0.2f;
	case EMovementType::InAir:
		return Velocity.Z > 100.f ? 0.15 : 0.5;
	default:
		return 0.f;
	}
}