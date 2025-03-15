// Lalinha 2025

#include "Character/TpsCharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "KismetAnimationLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionWarpingComponent.h"
#include "Character/PreCmcTick.h"
#include "ChooserFunctionLibrary.h"
#include "PoseSearch/PoseSearchLibrary.h"
#include "AnimationWarpingLibrary.h"
#include "Components/CombatComponent.h"
#include "Traversal/BlockBase.h"
#include "Interfaces/InteractionInterface.h"
#include "Weapons/Weapon.h"
#include "GAS_TPS_Base/Public/GAS_TPS_Base.h"
#include "PlayerController/TpsPlayerController.h"
#include "GameMode/MainGameMode.h"
#include "TimerManager.h"
#include "PlayerState/MainPlayerState.h"
#include "Components/BuffComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"


ATPSCharacterBase::ATPSCharacterBase() {
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Mesh config
	USkeletalMeshComponent* CharMeshComponent = GetMesh();
	CharMeshComponent->SetRelativeTransform(FTransform(
		FRotator(0.0f, -90.0f, 0.0f).Quaternion(),
		FVector(0.0f, 0.0f, -88.0f),
		FVector::OneVector));
	CharMeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CharMeshComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CharMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	CharMeshComponent->SetCollisionObjectType(ECC_SkeletalMesh);
	
	// Set size for collision capsule
	UCapsuleComponent* CharCapsuleComponent = GetCapsuleComponent();
	CharCapsuleComponent->SetCapsuleHalfHeight(86.0f);
	CharCapsuleComponent->SetCapsuleRadius(30.0f);
	CharCapsuleComponent->SetLineThickness(0.5f);
	CharCapsuleComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CharCapsuleComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// Configure character movement
	UCharacterMovementComponent* CharMovComponent = GetCharacterMovement();
	CharMovComponent->bOrientRotationToMovement = false;
	CharMovComponent->MaxAcceleration = 800.0f;
	CharMovComponent->BrakingFrictionFactor = 1.0f;
	CharMovComponent->SetCrouchedHalfHeight(60.0f);
	CharMovComponent->bUseSeparateBrakingFriction = true;
	CharMovComponent->GroundFriction = 5.0f;
	CharMovComponent->MaxWalkSpeed = 500.0f;
	CharMovComponent->MaxWalkSpeedCrouched = 300.0f;
	CharMovComponent->MinAnalogWalkSpeed = 150.0f;
	CharMovComponent->bCanWalkOffLedgesWhenCrouching = true;
	CharMovComponent->PerchRadiusThreshold = 20.0f;
	CharMovComponent->bUseFlatBaseForFloorChecks = true;
	CharMovComponent->JumpZVelocity = 500.f;
	CharMovComponent->AirControl = 0.25f;
	CharMovComponent->RotationRate = FRotator(0.0f, -1.0f, 0.0f);
	CharMovComponent->bUseControllerDesiredRotation = true;
	CharMovComponent->BrakingDecelerationWalking = 1500.0f;
	CharMovComponent->GetNavAgentPropertiesRef().bCanCrouch = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetRelativeTransform(FTransform(
		FRotator(0.0f, 0.0f, 0.0f).Quaternion(),
		FVector(0.0f, 30.0f, 25.0f),
		FVector::OneVector));
	SpringArm->ProbeSize = 0.0f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagMaxDistance = 200.0f;
	SpringArm->SetupAttachment(RootComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("Motion Warping"));
	PreCmcTick = CreateDefaultSubobject<UPreCmcTick>(TEXT("PreCMCTick"));

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true);

	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);

	CameraStyleThirdPersonFar.SpringArmLength = 300.0f;
	CameraStyleThirdPersonFar.SocketOffset = FVector(0.0f, 20.0f, 20.0f);
	CameraStyleThirdPersonFar.TranslationLagSpeed = 6.0f;
	CameraStyleThirdPersonFar.FieldOfView = 90.0f;
	CameraStyleThirdPersonFar.TransitionSpeed = 2.0f;

	CameraStyleThirdPersonClose.SpringArmLength = 150.0f;
	CameraStyleThirdPersonClose.SocketOffset = FVector(0.0f, 50.0f, 20.0f);
	CameraStyleThirdPersonClose.TranslationLagSpeed = 20.0f;
	CameraStyleThirdPersonClose.FieldOfView = 50.0f;
	CameraStyleThirdPersonClose.TransitionSpeed = 2.0f;
	
	CameraStyleThirdPersonAim.SpringArmLength = 0.0f;
	CameraStyleThirdPersonAim.SocketOffset = FVector(0.0f, -180.f, 0.0f);
	CameraStyleThirdPersonAim.TranslationLagSpeed = 30.0f;
	CameraStyleThirdPersonAim.FieldOfView = 10.0f;
	CameraStyleThirdPersonAim.TransitionSpeed = 17.0f;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
}

void ATPSCharacterBase::BeginPlay() {
	Super::BeginPlay();

	if (UCharacterMovementComponent* CharMovComponent = GetCharacterMovement()) {
		if (PreCmcTick) {
			CharMovComponent->PrimaryComponentTick.AddPrerequisite(PreCmcTick, PreCmcTick->PrimaryComponentTick);
		}
	}

	HandleHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ATPSCharacterBase::ReceiveDamage);
	}
}

void ATPSCharacterBase::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);
	
	HideCameraIfCharacterClose();
	PollInit();
}

void ATPSCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ATPSCharacterBase, CharacterInputState, COND_SkipOwner);
	DOREPLIFETIME(ATPSCharacterBase, CharacterWeaponState);
	DOREPLIFETIME_CONDITION(ATPSCharacterBase, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ATPSCharacterBase, TraversalResult, COND_SkipOwner);
	DOREPLIFETIME(ATPSCharacterBase, Health);
	DOREPLIFETIME(ATPSCharacterBase, bDisableGameplay);

}

void ATPSCharacterBase::PostInitializeComponents() {
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
	}
}

void ATPSCharacterBase::Server_SetCharacterWeaponState_Implementation(const FCharacterWeaponState NewState) {
	CharacterWeaponState = NewState;
}

void ATPSCharacterBase::Server_PickupAction_Implementation() {
	// Combat is being checked for this function call
	Combat->EquipWeapon(OverlappingWeapon);
}

void ATPSCharacterBase::Server_SetCharacterInputState_Implementation(const FCharacterInputState NewState) {
	CharacterInputState = NewState;
}

void ATPSCharacterBase::Server_Traversal_Implementation(const FTraversalParams InTraversalParams) {
	TraversalResult = InTraversalParams;
	PerformTraversalAction();
}

/////////////////////////////////////////////////////////////////////////////////// INPUT SETUP



void ATPSCharacterBase::NotifyControllerChanged() {
	Super::NotifyControllerChanged();
	// Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(IMC_Default, 0);
		}
	}
}

void ATPSCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Traversal
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::StartTraversalAction);
		
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::Look);

		// Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::ToggleCrouch);

		// Walking
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::ToggleWalk);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATPSCharacterBase::StartSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATPSCharacterBase::EndSprint);

		// Aim
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPSCharacterBase::StartAiming);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPSCharacterBase::EndAiming);

		// Pickup
		EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::PickUpAction);

		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &ATPSCharacterBase::Reload_Action);

		// Fire/Attack
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATPSCharacterBase::StartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ATPSCharacterBase::EndFiring);
	}
}



void ATPSCharacterBase::Move(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	const FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr) {
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATPSCharacterBase::Look(const FInputActionValue& Value) {
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATPSCharacterBase::ToggleWalk(const FInputActionValue& Value) {
	if (CharacterInputState.bWantsToSprint) return;

	if (CharacterInputState.bWantsToWalk) {
		CharacterInputState.bWantsToWalk = false;
	}
	else {
		CharacterInputState.bWantsToWalk = true;
	}

	Server_SetCharacterInputState(CharacterInputState);
}

void ATPSCharacterBase::ToggleCrouch(const FInputActionValue& Value) {
	if (bDisableGameplay) return;
	if (!bIsCrouched && !CharacterInputState.bWantsToSprint) {
		Crouch();
	}
	else {
		UnCrouch();
	}
}

void ATPSCharacterBase::StartSprint() {
	if (bDisableGameplay) return;
	
	if (bIsCrouched) {
		UnCrouch();
	}

	CharacterInputState.bWantsToWalk = false;
	CharacterInputState.bWantsToAim = false;
	CharacterInputState.bWantsToStrafe = false;
	CharacterInputState.bWantsToSprint = true;
	Server_SetCharacterInputState(CharacterInputState);
}

void ATPSCharacterBase::EndSprint() {
	CharacterInputState.bWantsToWalk = false;
	CharacterInputState.bWantsToAim = false;
	CharacterInputState.bWantsToStrafe = true;
	CharacterInputState.bWantsToSprint = false;
	Server_SetCharacterInputState(CharacterInputState);
}

void ATPSCharacterBase::StartAiming() {
	if (bDisableGameplay) return;
	
	CharacterInputState.bWantsToWalk = true;
	CharacterInputState.bWantsToAim = true;
	CharacterInputState.bWantsToStrafe = false;
	CharacterInputState.bWantsToSprint = false;
	Server_SetCharacterInputState(CharacterInputState);

}

void ATPSCharacterBase::EndAiming() {
	CharacterInputState.bWantsToWalk = false;
	CharacterInputState.bWantsToAim = false;
	CharacterInputState.bWantsToStrafe = true;
	CharacterInputState.bWantsToSprint = false;
	Server_SetCharacterInputState(CharacterInputState);

}

void ATPSCharacterBase::PickUpAction() {
	if (bDisableGameplay) return;
	
	if (Combat) {
		if (HasAuthority()) {
			Combat->EquipWeapon(OverlappingWeapon);
		}

		Server_PickupAction();
	}
}

void ATPSCharacterBase::StartFiring() {
	if (bDisableGameplay) return;	
	
	if (Combat) {
		Combat->FireButtonPressed(true);
	}
}

void ATPSCharacterBase::EndFiring() {
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
}

void ATPSCharacterBase::Reload_Action() {
	if (bDisableGameplay) return;
	
	if (Combat)
	{
		Combat->Reload();
	}
}

//////////////////////////////////////////////////////////////////// MOVEMENT HELPER FUNCTIONS
///

double ATPSCharacterBase::CalculateMaxAcceleration(const FVector& CharacterVelocity) const {
	switch (CurrentGait) {
		case EGait::Walk:  return 800.0f;
		case EGait::Run:   return 800.0f;
		case EGait::Sprint: return UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::VSizeXY(CharacterVelocity), 300.0, 700.0, 800.0, 300.0);
		default: return 0.0f;
	}
}

double ATPSCharacterBase::CalculateBrakingDeceleration() const {
	if (GetPendingMovementInputVector() != FVector::ZeroVector) {
		return 500.0f;
	}

	return 2000.0f;
}

double ATPSCharacterBase::CalculateMaxSpeed(const FVector& CharacterVelocity) const {
	if (StrafeSpeedMapCurve) {
		const float StrafeSpeedMap = StrafeSpeedMapCurve.LoadSynchronous()->GetFloatValue(
			FMath::Abs(UKismetAnimationLibrary::CalculateDirection(CharacterVelocity, GetActorRotation())));
		const bool bStrafe = StrafeSpeedMap < 1.0f;
		const FVector& Speeds = bIsCrouched ? CrouchSpeeds :
			CurrentGait == EGait::Walk ? WalkSpeeds :
			CurrentGait == EGait::Run ? RunSpeeds :
			CurrentGait == EGait::Sprint ? SprintSpeeds :
			FVector::ZeroVector;
		static const FVector2D RangeA1 = FVector2D(0.0f, 1.0f);
		static const FVector2D RangeA2 = FVector2D(1.0f, 2.0f);
		return FMath::GetMappedRangeValueClamped(bStrafe ? RangeA1 : RangeA2,
			FVector2D(bStrafe ? Speeds.X : Speeds.Y, bStrafe ? Speeds.Y : Speeds.Z), StrafeSpeedMap);
	}

	return INDEX_NONE;
}

EGait ATPSCharacterBase::GetDesiredGait(const FVector& CurrentAcceleration) const {
	if (CharacterInputState.bWantsToAim || CharacterInputState.bWantsToWalk && !CharacterInputState.bWantsToSprint) {
		return EGait::Walk;
	}

	if (CharacterInputState.bWantsToSprint && CanSprint(CurrentAcceleration) && !CharacterInputState.bWantsToWalk) {
		return EGait::Sprint;
	}

	return EGait::Run;
}


bool ATPSCharacterBase::CanSprint(const FVector& CurrentAcceleration) const {
	if (IsLocallyControlled()) {
		return UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizedDeltaRotator(
			GetActorRotation(), UKismetMathLibrary::MakeRotFromX(GetPendingMovementInputVector())).Yaw
		) < 50.f;
	}

	return UKismetMathLibrary::Abs(UKismetMathLibrary::NormalizedDeltaRotator(
		GetActorRotation(), UKismetMathLibrary::MakeRotFromX(CurrentAcceleration)).Yaw
	) < 50.f;
}



//////////////////////////////////////////////////////////////////// MOVEMENT FUNCTIONS


void ATPSCharacterBase::UpdateRotation_PreCmc() const {
	if (UCharacterMovementComponent* CharMovComponent = GetCharacterMovement()) {
		// CharMovComponent->bUseControllerDesiredRotation = CharacterInputState.bWantsToStrafe || CharacterInputState.bWantsToAim;
		// CharMovComponent->bOrientRotationToMovement = !CharacterInputState.bWantsToStrafe && !CharacterInputState.bWantsToAim;
		CharMovComponent->RotationRate = CharMovComponent->IsFalling() ? FRotator(0.0f, 200.0f, 0.0f) : FRotator(0.0f, -1.0f, 0.0f);
	}
}

void ATPSCharacterBase::UpdateMovement_PreCmc() {
	if(UCharacterMovementComponent* CharMovComponent = GetCharacterMovement()) {
		CurrentGait = GetDesiredGait(CharMovComponent->GetCurrentAcceleration());

		CharMovComponent->MaxAcceleration = CalculateMaxAcceleration(CharMovComponent->Velocity);
		CharMovComponent->BrakingDecelerationWalking = CalculateBrakingDeceleration();
		CharMovComponent->GroundFriction = CurrentGait == EGait::Walk || CurrentGait == EGait::Run ? 5.0 :
			UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::VSizeXY(CharMovComponent->Velocity), 0.0, 500.0, 5.0, 3.0);
			
		CharMovComponent->MaxWalkSpeed = CharMovComponent->MaxWalkSpeedCrouched =
			CalculateMaxSpeed(CharMovComponent->Velocity);
	}
}

void ATPSCharacterBase::UpdateMovementSimulated(const FVector& OldVelocity) {
	IsMovingOnGround = GetCharacterMovement()->IsMovingOnGround();
	if (IsMovingOnGround != WasMovingOnGroundSimulated)
	{
		(IsMovingOnGround) ? FinishJump(OldVelocity) : PlayAudio(UKismetMathLibrary::VSizeXY(OldVelocity), true);
	}
	WasMovingOnGroundSimulated = IsMovingOnGround;
}

void ATPSCharacterBase::UpdateCamera_PreCmc(const bool bInterpolate) {
	const FCameraParameters& TargetCameraParameters = CharacterInputState.bWantsToSprint ? CameraStyleThirdPersonFar : 
		CharacterInputState.bWantsToAim ? CameraStyleThirdPersonAim : CameraStyleThirdPersonClose;


	const float TranslationLagSpeed = bInterpolate ? TargetCameraParameters.TranslationLagSpeed : -1.0f;
	const float TransitionSpeed = bInterpolate ? TargetCameraParameters.TransitionSpeed : -1.0f;

	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetCameraParameters.FieldOfView,
		GetWorld()->DeltaTimeSeconds, TransitionSpeed));

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength,
		TargetCameraParameters.SpringArmLength, GetWorld()->DeltaTimeSeconds, TransitionSpeed);
	SpringArm->CameraLagSpeed = FMath::FInterpTo(SpringArm->CameraLagSpeed, TranslationLagSpeed,
		GetWorld()->DeltaTimeSeconds, TransitionSpeed);
	SpringArm->SocketOffset = FMath::VInterpTo(SpringArm->SocketOffset, TargetCameraParameters.SocketOffset,
		GetWorld()->DeltaTimeSeconds, TransitionSpeed);
}

void ATPSCharacterBase::Landed(const FHitResult& Hit) {
	FinishJump(GetCharacterMovement()->Velocity);
}

void ATPSCharacterBase::FinishJump(const FVector& InVel) {
	PlayAudio(UKismetMathLibrary::MapRangeClamped(InVel.Z, -500.f, -900.f, 0.5f, 1.5f), false);
	JustLanded = true;

	FLatentActionInfo LatentInfo;
	LatentInfo.CallbackTarget = this;
	LatentInfo.ExecutionFunction = "OnDelayCompleted";
	LatentInfo.Linkage = 0;
	LatentInfo.UUID = __LINE__;

	CustomDelay(this, 0.3f, LatentInfo);
}

void ATPSCharacterBase::CustomDelay(const UObject* WorldContextObject, const float Duration, 
	const FLatentActionInfo& LatentInfo) {
	
	if (UWorld* World = GEngine->GetWorldFromContextObjectChecked(WorldContextObject)) {
		FLatentActionManager& LatentManager = World->GetLatentActionManager();
		if (LatentManager.FindExistingAction<FDelayLatentAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr) {
			LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, new FDelayLatentAction(Duration, LatentInfo));
		}
	}
}

void ATPSCharacterBase::OnDelayCompleted() {
	JustLanded = false;
}

////////////////////////////////////////////////////////////////////////////// ANIMATION
///

void ATPSCharacterBase::OnMontageCompleted(const bool Value) {
	DoingTraversalAction = Value;
	OnTraversal(Value);
	GetCapsuleComponent()->IgnoreComponentWhenMoving(TraversalResult.HitComponent, Value);

	GetCharacterMovement()->SetMovementMode((Value) ? EMovementMode::MOVE_Flying : EMovementMode::MOVE_Walking, 0);
}




////////////////////////////////////////////////////////////////////////////// TRAVERSAL ACTION FUNCTIONALITY
///

void ATPSCharacterBase::StartTraversalAction() {
	if (DoingTraversalAction || bDisableGameplay) return;
	if (!GetCharacterMovement()->IsMovingOnGround()) return;


	if (GetCharacterMovement()->IsMovingOnGround())
	{
		TempTraversalParams.NewMovementMode = MOVE_Walking;
	} else
	{
		TempTraversalParams.NewMovementMode = MOVE_Falling;
	}
	
	TryTraversalAction();
}

bool ATPSCharacterBase::TryTraversalAction() {
	bool TestBool = false;

	CacheValues();
	TestBool = StoreHitComponent();
	if (TestBool) return true;

	if (!TempTraversalParams.HasFrontLedge) return true;

	TestBool = HasRoom();
	if (!TestBool) return true;

	SetObstacleHeight();
	TestBool = TopSweep();
	SetObstacleDepth(TestBool);
	DownwardTrace();
	GetAllMatchingMontages();

	if (ValidMontages.IsEmpty()) return true;

	SetInteractionTransform();
	SelectTraversalMontage();
	if (!TempTraversalParams.ChosenMontage) return true;

	TraversalResult = TempTraversalParams;
	PerformTraversalAction();
	Server_Traversal(TraversalResult);

	return false;
}

FTraversalCheckInputs ATPSCharacterBase::GetTraversalInputs() const {
	FTraversalCheckInputs NewInputs;
	NewInputs.TraceForwardDirection = GetActorForwardVector();
	NewInputs.TraceRadius = 30.f;
	switch (GetCharacterMovement()->MovementMode)
	{
	case MOVE_None:
		NewInputs.TraceForwardDistence = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Quat_UnrotateVector(GetActorRotation().Quaternion(), GetCharacterMovement()->Velocity).X, 0.f, 500.f, 75.f, 350.f);
		NewInputs.TraceHalfHeight = 60.f;
		break;
	case MOVE_Walking:
		NewInputs.TraceForwardDistence = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Quat_UnrotateVector(GetActorRotation().Quaternion(), GetCharacterMovement()->Velocity).X, 0.f, 500.f, 75.f, 350.f);
		NewInputs.TraceHalfHeight = 60.f;
		break;
	case MOVE_NavWalking:
		NewInputs.TraceForwardDistence = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Quat_UnrotateVector(GetActorRotation().Quaternion(), GetCharacterMovement()->Velocity).X, 0.f, 500.f, 75.f, 350.f);
		NewInputs.TraceHalfHeight = 60.f;
		break;
	case MOVE_Falling:
		NewInputs.TraceForwardDistence = 75.f;
		NewInputs.TraceHalfHeight = 86.f;
		break;
	case MOVE_Swimming:
		NewInputs.TraceForwardDistence = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Quat_UnrotateVector(GetActorRotation().Quaternion(), GetCharacterMovement()->Velocity).X, 0.f, 500.f, 75.f, 350.f);
		NewInputs.TraceHalfHeight = 60.f;
		break;
	case MOVE_Flying:
		NewInputs.TraceForwardDistence = 75.f;
		NewInputs.TraceHalfHeight = 86.f;
		break;
	case MOVE_Custom:
		NewInputs.TraceForwardDistence = UKismetMathLibrary::MapRangeClamped(UKismetMathLibrary::Quat_UnrotateVector(GetActorRotation().Quaternion(), GetCharacterMovement()->Velocity).X, 0.f, 500.f, 75.f, 350.f);
		NewInputs.TraceHalfHeight = 60.f;
		break;
	case MOVE_MAX:
		break;
	default:
		break;
	}
	return NewInputs;
}

void ATPSCharacterBase::CacheValues() {
	ActorLocation = GetActorLocation();

	CapsuleRadius = GetCapsuleComponent()->GetScaledCapsuleRadius();
	CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

FHitResult ATPSCharacterBase::TraversalTrace(const FVector& Start, const FVector& End, const float Radius, const float HalfHeight) const {
	FHitResult HitResult;
	const TArray<AActor*> IgnoredActors;
	const ETraceTypeQuery TraversalQuery = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	UKismetSystemLibrary::CapsuleTraceSingle(this, Start, End, Radius, HalfHeight, TraversalQuery, false, IgnoredActors, EDrawDebugTrace::None, HitResult, true);

	return HitResult;
}

bool ATPSCharacterBase::StoreHitComponent() {
	const FTraversalCheckInputs Inputs = GetTraversalInputs();
	const FVector Start = ActorLocation + Inputs.TraceOriginOffset;
	const FVector End = Start + (Inputs.TraceForwardDirection * Inputs.TraceForwardDistence) + Inputs.TraceEndOffset;
	FHitResult NewHit = TraversalTrace(Start, End, Inputs.TraceRadius, Inputs.TraceHalfHeight);
	if (!NewHit.bBlockingHit) return true;

	ABlockBase* Block = Cast<ABlockBase>(NewHit.GetActor());
	if (!Block) return true;
	TempTraversalParams.HitComponent = NewHit.GetComponent();

	Block->GetLedgeTransform(NewHit.ImpactPoint, ActorLocation, this);
	return false;
}

bool ATPSCharacterBase::HasRoom() {
	HasRoomCheckFrontLedgeLocation = TempTraversalParams.FrontLedgeLocation + (TempTraversalParams.FrontLedgeNormal * CapsuleRadius + 2.f) + FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	const FHitResult NewResult = TraversalTrace(ActorLocation, HasRoomCheckFrontLedgeLocation, CapsuleRadius * 0.6f, CapsuleHalfHeight);
	const bool NewBool = UKismetMathLibrary::BooleanNOR(NewResult.bBlockingHit, NewResult.IsValidBlockingHit());
	if (!NewBool) TempTraversalParams.HasFrontLedge = false;

	return NewBool;
}

void ATPSCharacterBase::SetObstacleHeight() {
	TempTraversalParams.ObstacleHeight = UKismetMathLibrary::Abs(((ActorLocation - FVector(0.f, 0.f, CapsuleHalfHeight)) - TempTraversalParams.FrontLedgeLocation).Z);
}

bool ATPSCharacterBase::TopSweep() {
	const FVector HeightZ = FVector(0.f, 0.f, CapsuleHalfHeight + 2.f);
	HasRoomCheckBackLedgeLocation = TempTraversalParams.BackLedgeLocation + TempTraversalParams.BackLedgeNormal * (CapsuleRadius + 2.f) + HeightZ;
	TopSweepResult = TraversalTrace(HasRoomCheckFrontLedgeLocation, HasRoomCheckBackLedgeLocation, CapsuleRadius, CapsuleHalfHeight);
	return !TopSweepResult.bBlockingHit;
}

void ATPSCharacterBase::SetObstacleDepth(const bool HasRoom) {
	if (!HasRoom) 	{
		TempTraversalParams.ObstacleDepth = (TopSweepResult.ImpactPoint - TempTraversalParams.FrontLedgeLocation).Size2D();
		TempTraversalParams.HasBackLedge = false;
		return;
	}

	TempTraversalParams.ObstacleDepth = (TempTraversalParams.FrontLedgeLocation - TempTraversalParams.BackLedgeLocation).Size2D();
}

void ATPSCharacterBase::DownwardTrace() {
	const FVector HeightZ = FVector(0.f, 0.f, (TempTraversalParams.ObstacleHeight - CapsuleHalfHeight) + 50.f);
	const FVector End = (TempTraversalParams.BackLedgeLocation + TempTraversalParams.BackLedgeNormal * (CapsuleRadius + 2.f)) - HeightZ;
	FHitResult DownHit = TraversalTrace(HasRoomCheckBackLedgeLocation, End, CapsuleRadius, CapsuleHalfHeight);

	if (DownHit.bBlockingHit)
	{
		TempTraversalParams.HasBackFloor = true;
		TempTraversalParams.BackFloorLocation = DownHit.ImpactPoint;
		TempTraversalParams.BackLedgeHeight = UKismetMathLibrary::Abs((DownHit.ImpactPoint - TempTraversalParams.BackLedgeLocation).Z);
	}
	else TempTraversalParams.HasBackFloor = false;
}

void ATPSCharacterBase::GetAllMatchingMontages() {
	if (!TraversalTable) return;

	ValidMontages = UChooserFunctionLibrary::EvaluateChooserMulti(this, TraversalTable, UAnimMontage::StaticClass());
}

void ATPSCharacterBase::SetInteractionTransform()
{
	if (IInteractionInterface* AnimInterface = Cast<IInteractionInterface>(GetMesh()->GetAnimInstance()))
		AnimInterface->SetInteractionTransform(FTransform(UKismetMathLibrary::MakeRotFromZ(TempTraversalParams.FrontLedgeNormal),
			                                                    TempTraversalParams.FrontLedgeLocation, FVector(1.f, 1.f, 1.f)));
}

void ATPSCharacterBase::SelectTraversalMontage()
{
	FPoseSearchBlueprintResult Result;
	UPoseSearchLibrary::MotionMatch(GetMesh()->GetAnimInstance(), ValidMontages, "PoseHistory", FPoseSearchContinuingProperties(), FPoseSearchFutureProperties(), Result);

	TempTraversalParams.ChosenMontage = Cast<UAnimMontage>(Result.SelectedAnimation);
	if (!TempTraversalParams.ChosenMontage) return;
	TempTraversalParams.StartTime = Result.SelectedTime;
	TempTraversalParams.PlayRate = Result.WantedPlayRate;
}

void ATPSCharacterBase::UpdateWarpTargets()
{
	float DistanceFromFrontLedgeToBackLedge = 0.f;
	float DistanceFromFrontLedgeToBackFloor = 0.f;

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation("FrontLedge", TraversalResult.FrontLedgeLocation,
		               UKismetMathLibrary::MakeRotFromX(UKismetMathLibrary::NegateVector(TraversalResult.FrontLedgeNormal)));

	if (TraversalResult.ActionType == ETraversalActionType::Hurdle || TraversalResult.ActionType == ETraversalActionType::Vault)
	{
		TArray<FMotionWarpingWindowData> OutData;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, "BackLedge", OutData);
		if (OutData.IsEmpty()) MotionWarping->RemoveWarpTarget("BackLedge");
		else
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(TraversalResult.ChosenMontage, "Distance_From_Ledge", OutData[0].EndTime,
				                                                                                       DistanceFromFrontLedgeToBackLedge);
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation("BackLedge", TraversalResult.BackLedgeLocation, FRotator::ZeroRotator);
		}
	}
	else MotionWarping->RemoveWarpTarget("BackLedge");

	if (TempTraversalParams.ActionType == ETraversalActionType::Hurdle)
	{
		TArray<FMotionWarpingWindowData> OutData2;
		UMotionWarpingUtilities::GetMotionWarpingWindowsForWarpTargetFromAnimation(TraversalResult.ChosenMontage, "BackFloor", OutData2);
		if (OutData2.IsEmpty()) MotionWarping->RemoveWarpTarget("BackFloor");
		else
		{
			UAnimationWarpingLibrary::GetCurveValueFromAnimation(TraversalResult.ChosenMontage, "Distance_From_Ledge", OutData2[0].EndTime,
				DistanceFromFrontLedgeToBackFloor);

			FVector TempLoc(FVector(TraversalResult.BackLedgeLocation + (TraversalResult.BackLedgeNormal * UKismetMathLibrary::Abs
			                                                (DistanceFromFrontLedgeToBackLedge - DistanceFromFrontLedgeToBackFloor))));
			FVector NewLoc(TempLoc.X, TempLoc.Y, TraversalResult.BackFloorLocation.Z);
			MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation("BackFloor", NewLoc, FRotator::ZeroRotator);
		}

	}
	else MotionWarping->RemoveWarpTarget("BackFloor");
}

void ATPSCharacterBase::PerformTraversalAction()
{
	UpdateWarpTargets();

	UAnimMontage* MontageToPlay = const_cast<UAnimMontage*>(TraversalResult.ChosenMontage);
	if (!MontageToPlay) return;
	if (IInteractionInterface* AnimInterface = Cast<IInteractionInterface>(GetMesh()->GetAnimInstance()))
		AnimInterface->PlayTraversalMontage(MontageToPlay, TraversalResult.PlayRate, TraversalResult.StartTime);

	OnMontageCompleted(true);
}

void ATPSCharacterBase::OnRep_TraversalResult() {
	PerformTraversalAction();
}

void ATPSCharacterBase::OnTraversal(const bool NewBool) const {
	GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = NewBool;
	GetCharacterMovement()->bServerAcceptClientAuthoritativePosition = NewBool;
}

void ATPSCharacterBase::SetAnimParam(UAnimMontage* InMontage) {
	TempTraversalParams.ChosenMontage = InMontage;
}

////////////////////////////////////////////////////////////////////////////// WEAPONS FUNCTIONALITY
///

void ATPSCharacterBase::SetOverlappingWeapon(AWeapon* Weapon) {
	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(false);
		}	
	}

	OverlappingWeapon = Weapon;
	
	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ATPSCharacterBase::OnRep_OverlappingWeapon(const AWeapon* LastWeapon) const {
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}
	
	if (LastWeapon)  {
		LastWeapon->ShowPickupWidget(false);
	}
}

void ATPSCharacterBase::HideCameraIfCharacterClose() const {
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < HideCameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	} else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon)
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ATPSCharacterBase::SpawnDefaultWeapon() const
{
	const AMainGameMode* MainGameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (MainGameMode && World && !bIsEliminated && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		if (StartingWeapon)
		{
			StartingWeapon->bDestroyWeapon = true;
		}
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

////////////////////////////////////////////////////////////////////////////// HEALTH
///

void ATPSCharacterBase::OnRep_Health(const float LastHealthValue) {
	// can play animations here
	HandleHUDHealth();

	if (Health < LastHealthValue)
	{
		// play hit reaction montage for instance
	} else
	{
		// we are healing
	}
}

void ATPSCharacterBase::HandleHUDHealth()
{
	TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Controller) : TpsPlayerController;
	if (TpsPlayerController)
	{
		TpsPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}


void ATPSCharacterBase::UpdateHUDAmmo()
{
	TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Controller) : TpsPlayerController;
	if (TpsPlayerController && Combat && Combat->EquippedWeapon)
	{
		TpsPlayerController->SetHUDCarryingAmmo(Combat->CarryingAmmo);
		TpsPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

void ATPSCharacterBase::ReceiveDamage(AActor* DamagedActor, const float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	HandleHUDHealth();
	// can play animations here
	// spawn blood here;

	if (Health == 0.f)
	{
		if (AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
		{
			TpsPlayerController = !TpsPlayerController ? Cast<ATpsPlayerController>(Controller) : TpsPlayerController;
			ATpsPlayerController* KillerController = Cast<ATpsPlayerController>(InstigatorController);
			GameMode->PlayerEliminated(this, TpsPlayerController, KillerController);
		}
	}

}

void ATPSCharacterBase::Eliminated()
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->bDestroyWeapon)
		{
			Combat->EquippedWeapon->Destroy();
		} else
		{
			Combat->EquippedWeapon->Dropped();
			Combat->EquippedWeapon = nullptr;
		}
	}
	MulticastEliminated_Implementation();
	GetWorldTimerManager().SetTimer(
		EliminatedTimer,
		this,
		&ATPSCharacterBase::EliminatedTimerFinisher,
		EliminatedDelay
	);
}

void ATPSCharacterBase::MulticastEliminated_Implementation() 
{
	if (TpsPlayerController)
	{
		TpsPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bIsEliminated = true;
	// handle animation here or toggle ragdoll

	// Disable Character Movement
	bDisableGameplay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
	// Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATPSCharacterBase::EliminatedTimerFinisher()
{
	if (AMainGameMode* GameMode = GetWorld()->GetAuthGameMode<AMainGameMode>())
	{
		GameMode->RequestRespawn(this, Controller);
	}
}

void ATPSCharacterBase::PollInit()
{
	if (!MainPlayerState)
	{
		MainPlayerState = GetPlayerState<AMainPlayerState>();
		if (MainPlayerState)
		{
			MainPlayerState->UpdateScore(0.f);
			MainPlayerState->UpdateDefeats(0);
		}
	}
}


ECombatState ATPSCharacterBase::GetCombatState() const
{
	if (!Combat) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}

