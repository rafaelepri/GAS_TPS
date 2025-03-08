// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimInstanceBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Character/TpsCharacterBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimationWarpingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PoseSearch/MotionMatchingAnimNodeLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "ChooserFunctionLibrary.h"
#include "BlendStack/BlendStackAnimNodeLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Components/Enums/CombatState.h"


void FASAnimInstanceProxy::InitializeObjects(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::InitializeObjects(InAnimInstance);

	Owner = InAnimInstance->TryGetPawnOwner();
	if (!Owner) return;

	Character = Cast<ATPSCharacterBase>(Owner);
	MovementComponent = Cast<UCharacterMovementComponent>(Owner->GetMovementComponent());
}

void FASAnimInstanceProxy::PreUpdate(UAnimInstance* InAnimInstance, float DeltaSeconds)
{
	FAnimInstanceProxy::PreUpdate(InAnimInstance, DeltaSeconds);
}

void FASAnimInstanceProxy::Update(float DeltaSeconds)
{
	FAnimInstanceProxy::Update(DeltaSeconds);
}




void UAnimInstanceBase::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (!Proxy.Character) return;
	Character = Proxy.Character;
	CharacterMovementComponent = Proxy.MovementComponent;

	OutThreshold1.FL = -60.f;
	OutThreshold1.FR = 60.f;
	OutThreshold1.BL = -120.f;
	OutThreshold1.BR = 120.f;

	OutThreshold2.FL = -60.f;
	OutThreshold2.FR = 60.f;
	OutThreshold2.BL = -140.f;
	OutThreshold2.BR = 140.f;

	OutThreshold3.FL = -40.f;
	OutThreshold3.FR = 40.f;
	OutThreshold3.BL = -140.f;
	OutThreshold3.BR = 140.f;
}

void UAnimInstanceBase::NativeThreadSafeUpdateAnimation(const float DeltaSeconds) {
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	if (!Proxy.Owner) return;

	UpdateCVars();

	GenerateTrajectory(DeltaSeconds);
	UpdateEssentialValues(DeltaSeconds);
	UpdateStates();

	const bool bIsPivoting = IsPivoting();
	const bool bIsWithinAO_Threshold90 = AO_Threshold(90.f);
	
	if (Proxy.Character) {
		bCanAim = CanOverlayAim();
		if (bCanAim) UpdateSpineRotation(DeltaSeconds);	
	}

	AdditiveOverlayWeight = UKismetMathLibrary::FInterpTo(AdditiveOverlayWeight, bCanAim  ? 1.f : 0.55f , DeltaSeconds, AdditiveOverlayInterpSpeed);

	float OverlayStrengthTarget;
	// float OverlayStrengthInterpSpeed;
	
	if (!bIsWithinAO_Threshold90 && bIsPivoting) {
		OverlayStrengthTarget = 0.5f;
	} else {
		if (MovementState == EMovementState::Idle) {
			OverlayStrengthTarget = 0.92f;
		} else {
			OverlayStrengthTarget = bCanAim ? 0.85f : 0.3f;
		}
	}
	
	AdditiveOverlayStrength = UKismetMathLibrary::FInterpTo(AdditiveOverlayStrength,
		 OverlayStrengthTarget,
		DeltaSeconds,
		 bIsPivoting && bIsWithinAO_Threshold90 ? 10.f :	AdditiveOverlayInterpSpeed
	);

	bIsReloading = Proxy.Character->GetCombatState() == ECombatState::ECS_Reloading;
	
	if (!UseExperimentalStateMachine) return;
	UpdateMovementDirection();
	UpdateTargetRotation();
}

void UAnimInstanceBase::GenerateTrajectory(float DeltaSeconds)
{
	FPoseSearchQueryTrajectory OutTrajectory;
	UPoseSearchTrajectoryLibrary::PoseSearchGenerateTrajectory(this, (Speed2D > 0.f) ? TrajectoryGenerationData_Moving : TrajectoryGenerationData_Idle,
		                                                      DeltaSeconds, Trajectory, PreviousDesiredControllYaw, OutTrajectory, -1.f, 30, 0.1f, 15);

	FPoseSearchTrajectory_WorldCollisionResults CollisionResult;
	TArray<AActor*> IgnoredActors;
	UPoseSearchTrajectoryLibrary::HandleTrajectoryWorldCollisions(Proxy.Character, this, OutTrajectory, true, 0.01f, Trajectory, CollisionResult,
		                                             ETraceTypeQuery::TraceTypeQuery1, false, IgnoredActors, EDrawDebugTrace::None, true, 150.f);

	TimeToLand = CollisionResult.TimeToLand;

	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, -0.3f, -0.2f, Trj_PastVelocity);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, 0.f, 0.2f, Trj_CurrentVelocity);
	UPoseSearchTrajectoryLibrary::GetTrajectoryVelocity(OutTrajectory, 0.4f, 0.5f, Trj_FutureVelocity);
}

void UAnimInstanceBase::UpdateEssentialValues(const float& DeltaSeconds) {
	CharacterTransformLastFrame = CharacterTransform;
	CharacterTransform = Proxy.Character->GetActorTransform();

	RootTransform = FTransform(FRotator(RootOffsetTransform.Rotator().Pitch, RootOffsetTransform.Rotator().Yaw + 90.f,
		              RootOffsetTransform.Rotator().Roll), RootOffsetTransform.GetLocation(), FVector(1.f, 1.f, 1.f));

	AccelerationLastFrame = Acceleration;
	Acceleration = Proxy.MovementComponent->GetCurrentAcceleration();
	AccelerationAmount = Acceleration.Size() / Proxy.MovementComponent->GetMaxAcceleration();
	HasAcceleration = AccelerationAmount > 0.f;

	VelocityLastFrame = Velocity;
	Velocity = Proxy.MovementComponent->Velocity;
	Speed2D = UKismetMathLibrary::VSizeXY(Velocity);

	HasVelocity = Speed2D > 5.f;
	CalculateAccelerationLean(DeltaSeconds);
	if (HasVelocity) LastNonZeroVelocity = Velocity;
}


void UAnimInstanceBase::CalculateAccelerationLean(float DeltaSeconds)
{
	FVector TempVector = (Velocity - VelocityLastFrame) / DeltaSeconds;
	if (HasAcceleration)
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


void UAnimInstanceBase::UpdateStates()
{
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
		break;
	}

	RotationModeLastFrame = RotationMode;
	RotationMode = (Proxy.MovementComponent->bOrientRotationToMovement) ? ERotationMode::OrientToRotation : ERotationMode::Strafe;

	MovementStateLastFrame = MovementState;
	MovementState = IsMoving() ? EMovementState::Moving : EMovementState::Idle;

	GaitLastFrame = Gait;
	Gait = Proxy.Character->CurrentGait;

	StanceLastFrame = Stance;
	Stance = Proxy.MovementComponent->IsCrouching() ? EStance::Crouch : EStance::Stand;
}

void UAnimInstanceBase::UpdateCVars()
{
	OffsetRootBoneEnabled = UKismetSystemLibrary::GetConsoleVariableBoolValue("a.animnode.offsetrootbone.enable");
	MMDatabaseLOD = UKismetSystemLibrary::GetConsoleVariableBoolValue("DDCvar.MMDatabaseLOD");
	OffsetRootTranslationRadius = UKismetSystemLibrary::GetConsoleVariableBoolValue("DDCvar.OffsetRootBone.TranslationRadius");
	UseExperimentalStateMachine = !GetOwningComponent()->ComponentHasTag("Force MM Setup") && UKismetSystemLibrary::GetConsoleVariableBoolValue("DDCVar.ExperimentalStateMachine.Enable") || GetOwningComponent()->ComponentHasTag("Force SM Setup");
}






// State Machine


void UAnimInstanceBase::UpdateMovementDirection() {
	MovementDirectionLastFrame = MovementDirection;
	if (MovementState != EMovementState::Moving) return;

	Direction = UKismetAnimationLibrary::CalculateDirection(UKismetMathLibrary::Vector_NormalUnsafe(Trj_FutureVelocity), CharacterTransform.GetRotation().Rotator());
	MovementDirectionThresholds = GetMovementDirectionThreshold();

	if (RotationMode == ERotationMode::OrientToRotation || Gait == EGait::Sprint) MovementDirection = EMovementDirection::F;
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, MovementDirectionThresholds.FL,
		     MovementDirectionThresholds.FR, true, true)) MovementDirection = EMovementDirection::F;
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, MovementDirectionThresholds.BL,
		     MovementDirectionThresholds.FL, true, true)) MovementDirection = (MovementDirectionBias == EMovementDirectionBias::LeftFootForward) ?
		     EMovementDirection::LL : EMovementDirection::LR;
	else if (UKismetMathLibrary::InRange_FloatFloat(Direction, MovementDirectionThresholds.FR,
		     MovementDirectionThresholds.BR, true, true)) MovementDirection = (MovementDirectionBias == EMovementDirectionBias::LeftFootForward) ?
		     EMovementDirection::RL : EMovementDirection::RR;
	else MovementDirection = EMovementDirection::B;


}

FMovementDirectionThreshold UAnimInstanceBase::GetMovementDirectionThreshold() const
{
	if (MovementDirection == EMovementDirection::F || MovementDirection == EMovementDirection::B) return OutThreshold1;
	return IsPivoting() ? OutThreshold1 : (BlendStackInputs.Loop && !Proxy.Character->CharacterInputState.bWantsToAim) ? OutThreshold2 : OutThreshold3;
}

void UAnimInstanceBase::UpdateTargetRotation()
{
	if (IsMoving())
	{
		switch (RotationMode)
		{
		case ERotationMode::OrientToRotation: TargetRotation = CharacterTransform.Rotator();
			break;
		case ERotationMode::Strafe: TargetRotation = FRotator(CharacterTransform.Rotator().Pitch, CharacterTransform.Rotator().Yaw +
			GetStrafeYawRotationOffset(), CharacterTransform.Rotator().Roll);
			break;
		default:
			break;
		}
	}
	else TargetRotation = CharacterTransform.Rotator();
	TargetRotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, RootTransform.Rotator()).Yaw;
}

float UAnimInstanceBase::GetStrafeYawRotationOffset() const
{
	if (!CurveContainer) return 0.f;
	const float MappedDirection = UKismetMathLibrary::MapRangeClamped(UKismetAnimationLibrary::CalculateDirection(UKismetMathLibrary::Vector_NormalUnsafe(Trj_FutureVelocity), 
		                          CharacterTransform.Rotator()), -180.f, 180.f, 0.f, 8.f) / 30.f;

	FName CurveName;
	switch (MovementDirection)
	{
	case EMovementDirection::F: CurveName = "StrafeOffset_F";
		break;
	case EMovementDirection::B: CurveName = "StrafeOffset_B";
		break;
	case EMovementDirection::LL: CurveName = "StrafeOffset_LL";
		break;
	case EMovementDirection::LR: CurveName = "StrafeOffset_LR";
		break;
	case EMovementDirection::RL: CurveName = "StrafeOffset_RL";
		break;
	case EMovementDirection::RR: CurveName = "StrafeOffset_RR";
		break;
	default:
		break;
	}

	float Result = 0.f;
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(CurveContainer, CurveName, MappedDirection, Result);
	return Result;
}






// Offset Root

EOffsetRootBoneMode UAnimInstanceBase::GetOffsetRootRotationMode() const
{
	return IsSlotActive("DefaultSlot") ? EOffsetRootBoneMode::LockOffsetAndIgnoreAnimation : EOffsetRootBoneMode::Accumulate;
}

EOffsetRootBoneMode UAnimInstanceBase::GetOffsetRootTranslationMode() const
{
	if (IsSlotActive("DefaultSlot")) return EOffsetRootBoneMode::Release;
	switch (MovementMode)
	{
	case EMovementType::OnGround:
		return IsMoving() ? EOffsetRootBoneMode::Interpolate : EOffsetRootBoneMode::Release;
		break;
	case EMovementType::InAir:
		return EOffsetRootBoneMode::Release;
		break;
	default:
		return EOffsetRootBoneMode();
		break;
	}
}

float UAnimInstanceBase::GetOffsetRootTranslationHalfLife() const
{
	switch (MovementState)
	{
	case EMovementState::Idle: return 0.1f;
		break;
	case EMovementState::Moving: return 0.3f;
		break;
	default:
		return 0.f;
		break;
	}
}

EOrientationWarpingSpace UAnimInstanceBase::GetOrientationWarpingSpace() const
{
	return OffsetRootBoneEnabled ? EOrientationWarpingSpace::RootBoneTransform : EOrientationWarpingSpace::ComponentTransform;
}

float UAnimInstanceBase::GetOffsetRootTranslationRadius() const
{
	return OffsetRootTranslationRadius;
}


void UAnimInstanceBase::UpdateOffsetRoot(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	RootOffsetTransform = UAnimationWarpingLibrary::GetOffsetRootTransform(Node);
}




// AimOffset

bool UAnimInstanceBase::EnableAO() const
{
	if (!Proxy.Owner) return false;
	const float TestFloat = (MovementState == EMovementState::Idle) ? 115.f : 180.f;
	return AO_Threshold(TestFloat) && RotationMode == ERotationMode::Strafe && GetSlotMontageLocalWeight("DefaultSlot") < 0.5f;
}

bool UAnimInstanceBase::AO_Threshold(const float& Threshold) const {
	return UKismetMathLibrary::Abs(GetAOValue().X) <= Threshold;
}

FVector UAnimInstanceBase::GetAOValue() const {
	if (!Proxy.Owner) return FVector::ZeroVector;
	
	const FRotator NewRotator = UKismetMathLibrary::NormalizedDeltaRotator((Proxy.Character->IsLocallyControlled()) ?
		Proxy.Character->GetControlRotation() : Proxy.Character->GetBaseAimRotation(), RootTransform.Rotator());
	
	return UKismetMathLibrary::VLerp(FVector(NewRotator.Yaw, NewRotator.Pitch, 0.f), FVector::ZeroVector, GetCurveValue("Disable_AO"));
}

//Movement Analysis


bool UAnimInstanceBase::IsMoving() const
{
	return Acceleration != FVector::ZeroVector && Trj_FutureVelocity != FVector::ZeroVector;
}

bool UAnimInstanceBase::IsStarting() const
{
	return IsMoving() && (UKismetMathLibrary::VSizeXY(Trj_FutureVelocity) >= UKismetMathLibrary::VSizeXY(Velocity) + 100.f) && !DatabaseTags.Contains("Pivots");
}

float UAnimInstanceBase::GetTrajectoryTurnAngle() const
{
	return UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::MakeRotFromX(Trj_FutureVelocity), UKismetMathLibrary::MakeRotFromX(Velocity)).Yaw;
}

bool UAnimInstanceBase::IsPivoting() const
{
	return !UseExperimentalStateMachine ? IsPivoting_MM() : IsPivoting_SM();
}

bool UAnimInstanceBase::IsPivoting_SM() const
{
	float TestAngle = 0.f;
	switch (Stance)
	{
	case EStance::Stand:

		switch (Gait)
		{
		case EGait::Walk:
			TestAngle = UKismetMathLibrary::MapRangeClamped(Speed2D, 150.f, 200.f, 70.f, 60.f);
			break;
		case EGait::Run:
			TestAngle = UKismetMathLibrary::MapRangeClamped(Speed2D, 300.f, 500.f, 70.f, 60.f);
			break;
		case EGait::Sprint:
			TestAngle = UKismetMathLibrary::MapRangeClamped(Speed2D, 300.f, 700.f, 60.f, 50.f);
			break;
		default:
			break;
		}
		break;
	case EStance::Crouch: TestAngle = 65.f;
		break;
	default:
		break;
	}
	const bool AngleCondition = UKismetMathLibrary::Abs(GetTrajectoryTurnAngle()) >= TestAngle;

	bool InRange = false;
	switch (Stance)
	{
	case EStance::Stand:
		switch (Gait)
		{
		case EGait::Walk:
			InRange = UKismetMathLibrary::InRange_FloatFloat(Speed2D, 50.f, 200.f, true, true);
			break;
		case EGait::Run:
			InRange = UKismetMathLibrary::InRange_FloatFloat(Speed2D, 200.f, 550.f, true, true);
			break;
		case EGait::Sprint:
			InRange = UKismetMathLibrary::InRange_FloatFloat(Speed2D, 200.f, 700.f, true, true);
			break;
		default:
			break;
		}
		break;
	case EStance::Crouch:
		InRange = UKismetMathLibrary::InRange_FloatFloat(Speed2D, 50.f, 200.f, true, true);
		break;
	default:
		break;
	}

	return AngleCondition && InRange;
}

bool UAnimInstanceBase::IsPivoting_MM() const
{
	const float TestFloat = (RotationMode == ERotationMode::OrientToRotation) ? 45.f : 30.f;
	return UKismetMathLibrary::Abs(GetTrajectoryTurnAngle()) >= TestFloat;
}

bool UAnimInstanceBase::ShouldTurnInPlace() const
{
	if (!Proxy.Character) return false;

	const double AbsoluteRotationYaw = UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.Rotator(),
	RootTransform.Rotator()).Yaw);
	
	// const FString FloatString = FString::Printf(TEXT("ABS Rotation Yaw value: %.2f"), variable );
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FloatString);
	
	if (Proxy.Character->CharacterInputState.bWantsToAim) {
		if (AbsoluteRotationYaw >= MaxTurnAngle) {
			return true;
		}
	}
	
	return AbsoluteRotationYaw >= MaxTurnAngle && MovementState == EMovementState::Idle &&
		MovementStateLastFrame == EMovementState::Moving;
}

bool UAnimInstanceBase::ShouldSpinTransition() const
{
	return UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizedDeltaRotator(CharacterTransform.Rotator(),
		    RootTransform.Rotator()).Yaw) >= 130.f && Speed2D >= 150.f && !DatabaseTags.Contains("Pivots");
}

bool UAnimInstanceBase::JustTraversed() const
{
	return !IsSlotActive("DefaultSlot") && GetCurveValue("MovingTraversal") > 0.f && UKismetMathLibrary::Abs(GetTrajectoryTurnAngle()) <= 50.f;
}

bool UAnimInstanceBase::JustLanded_Light() const
{
	if (!Proxy.Character) return false;
	return Proxy.Character->JustLanded && UKismetMathLibrary::Abs(Proxy.Character->LandVelocity.Z) < UKismetMathLibrary::Abs(HeavyLandSppedThreshold);
}

bool UAnimInstanceBase::JustLanded_Heavy() const
{
	if (!Proxy.Character) return false;
	return Proxy.Character->JustLanded && UKismetMathLibrary::Abs(Proxy.Character->LandVelocity.Z) >= UKismetMathLibrary::Abs(HeavyLandSppedThreshold);
}

float UAnimInstanceBase::GetLandVelocity() const
{
	return Proxy.Character->LandVelocity.Z;
}

bool UAnimInstanceBase::PlayLand() const
{
	return MovementMode == EMovementType::OnGround && MovementModeLastFrame == EMovementType::InAir;
}

bool UAnimInstanceBase::PlayMovingLand() const
{
	return MovementMode == EMovementType::OnGround && MovementModeLastFrame == EMovementType::InAir && UKismetMathLibrary::Abs(GetTrajectoryTurnAngle()) <= 120.f;
}




// State Node Functions

float UAnimInstanceBase::Get_MMNotifyRecencyTimeOut() const
{
	switch (Gait)
	{
	case EGait::Walk: return 0.2f;
		break;
	case EGait::Run: return 0.2f;
		break;
	case EGait::Sprint: return 0.16f;
		break;
	default:
		break;
	}

	return 0.0f;
}

float UAnimInstanceBase::GetMMBlendTime() const
{
	switch (MovementMode)
	{
	case EMovementType::OnGround:
		return (MovementModeLastFrame == EMovementType::OnGround) ? 0.5f : 0.2f;
		break;
	case EMovementType::InAir:
		return Velocity.Z > 100.f ? 0.15 : 0.5;
		break;
	default:
		return 0.f;
		break;
	}
}

EPoseSearchInterruptMode UAnimInstanceBase::GetMMInterupMode() const
{
	return (MovementMode != MovementModeLastFrame ||
		   (MovementMode == EMovementType::OnGround && (MovementState != MovementStateLastFrame ||
		   (Gait != GaitLastFrame && MovementState == EMovementState::Moving) || Stance != StanceLastFrame))) ?
		    EPoseSearchInterruptMode::InterruptOnDatabaseChange : EPoseSearchInterruptMode::DoNotInterrupt;
}


void UAnimInstanceBase::UpdateMotionMatchingMovement(const FAnimUpdateContext& Context, const FAnimNodeReference& Node) {
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

	UMotionMatchingAnimNodeLibrary::SetDatabasesToSearch(MM, PoseSearchArray, GetMMInterupMode());
}

void UAnimInstanceBase::MotionMatchingPostSelection(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
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




// BlendStack

void UAnimInstanceBase::UpdateBlendStack(const FAnimUpdateContext& Context, const FAnimNodeReference& Node)
{
	AnimTime = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAssetTime(Node);
	AnimAsset = UBlendStackAnimNodeLibrary::GetCurrentBlendStackAnimAsset(Node);
	const UAnimSequence* NewAnimSequence = Cast<UAnimSequence>(AnimAsset);
	if (!NewAnimSequence) return;
	UAnimationWarpingLibrary::GetCurveValueFromAnimation(NewAnimSequence, "Enable_OrientationWarping",
		                                                                  AnimTime, OrientationAlpha);
}

bool UAnimInstanceBase::EnableSteering() const
{
	return MovementState == EMovementState::Moving || MovementMode == EMovementType::InAir;
}

FQuat UAnimInstanceBase::GetDesiredFacing() const
{
	return Trajectory.GetSampleAtTime(0.5f, false).Facing; // Add Condition For ExperimantalStateMachine
}

bool UAnimInstanceBase::IsTurningInPlace() const
{
	return DatabaseTags.Contains("TurnInPlace");
}




// Interface

void UAnimInstanceBase::SetInteractionTransform(FTransform InTransform)
{
	InteractionTransform = InTransform;
}

FTransform UAnimInstanceBase::GetInteractionTransform()
{
	return InteractionTransform;
}

void UAnimInstanceBase::PlayTraversalMontage(UAnimMontage* MontageToPlay, float PlayRate, float StartTime)
{
	PlayAnAnimationMontage(MontageToPlay, PlayRate, StartTime);
}





// Montages And Delegates

void UAnimInstanceBase::FunctionToExecuteOnAnimationBlendOut(UAnimMontage* animMontage, bool bInterrupted)
{
	Proxy.Character->OnMontageCompleted(false);
}

void UAnimInstanceBase::FunctionToExecuteOnAnimationEnd(UAnimMontage* animMontage, bool bInterrupted)
{
	UE_LOG(LogTemp, Warning, TEXT("MY ANIMATION HAS COMPLETED!"));
}

void UAnimInstanceBase::PlayAnAnimationMontage(UAnimMontage* montageToPlay, float PlayRate, float StartTime)
{
	if (montageToPlay)
	{
		Montage_Play(montageToPlay, PlayRate, EMontagePlayReturnType::MontageLength, StartTime, true);
		Proxy.Character->OnMontageCompleted(true);

		FOnMontageEnded BlendOutDelegate;
		BlendOutDelegate.BindUObject(this, &UAnimInstanceBase::FunctionToExecuteOnAnimationBlendOut);
		Montage_SetBlendingOutDelegate(BlendOutDelegate, montageToPlay);

		FOnMontageEnded CompleteDelegate;
		CompleteDelegate.BindUObject(this, &UAnimInstanceBase::FunctionToExecuteOnAnimationEnd);
		Montage_SetEndDelegate(CompleteDelegate, montageToPlay);
	}
}


// OVERLAY


void UAnimInstanceBase::CanAim() {
	if (AO_Threshold(73.f) && Proxy.Character->CharacterInputState.bWantsToAim && !Proxy.Character->DoingTraversalAction)
	{
		bCanAim = true;
	}

	bCanAim = false;
}

bool UAnimInstanceBase::CanOverlayAim() const {
	if (MovementMode == EMovementType::OnGround && Proxy.Character->CharacterInputState.bWantsToAim && !JustLanded_Heavy() && !Proxy.Character->DoingTraversalAction && AO_Threshold(73.f)) {
		return true;
	}

	return false;
}

void UAnimInstanceBase::UpdateSpineRotation(const float DeltaSeconds) {
	
	// 1. Calculate the target roll
	TargetSpineRoll = UKismetMathLibrary::ClampAngle(GetAOValue().Y * -1.f, -90.f, 90.f) / 5.f;

	if (Proxy.Character->IsLocallyControlled()) {
		SpineRotation.Roll = TargetSpineRoll;
		return;
	}

	// 2. Interpolate toward the target roll
	SmoothedSpineRoll = FMath::FInterpTo(
		SmoothedSpineRoll,
		TargetSpineRoll,
		DeltaSeconds,
		SpineInterpSpeed
	);

	// 3. Apply the smoothed value to the bone
	SpineRotation.Roll = SmoothedSpineRoll;
}
