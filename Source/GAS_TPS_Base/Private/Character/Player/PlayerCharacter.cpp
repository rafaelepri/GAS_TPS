

#include "Character/Player/PlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Camera/CameraComponent.h"

#include "Components/CapsuleComponent.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "GAS_TPS_Base.h"

APlayerCharacter::APlayerCharacter()
{
	bReplicates = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	SetNetUpdateFrequency(66.f);
	SetMinNetUpdateFrequency(33.f);

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
	CharMovComponent->MaxAcceleration = 700.0f; // default 700.f;
	CharMovComponent->BrakingFrictionFactor = 1.0f; // default 1.0f;
	CharMovComponent->SetCrouchedHalfHeight(60.0f);
	CharMovComponent->bUseSeparateBrakingFriction = true;
	CharMovComponent->GroundFriction = 5.0f; // default 5.0
	CharMovComponent->MaxWalkSpeed = 380.0f; // default 380
	CharMovComponent->MaxWalkSpeedCrouched = 210.0f; // default 400
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
}

void APlayerCharacter::BeginPlay() {
	Super::BeginPlay();

	CurrentGait = EGait::Run;
}

void APlayerCharacter::Tick(const float DeltaTime) {
	Super::Tick(DeltaTime);
	

}

/////////////////////////////////////////////////////////////////////////////////// INPUT SETUP

void APlayerCharacter::NotifyControllerChanged() {
	Super::NotifyControllerChanged();
	// Add Input Mapping Context
	if (const APlayerController* PlayerController = Cast<APlayerController>(Controller)) {
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {
			Subsystem->AddMappingContext(IMC_Default, 0);
		}
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) {
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Traversal
		// EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &APlayerCharacter::StartTraversalAction);
		
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);

		// Crouching
		// EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ToggleCrouch);

		// Walking
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &APlayerCharacter::ToggleWalk);

		// Sprinting
		// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSprint);
		// EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndSprint);

		// Aim
		// EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APlayerCharacter::StartAiming);
		// EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndAiming);

		// Pickup
		// EnhancedInputComponent->BindAction(PickupAction, ETriggerEvent::Triggered, this, &APlayerCharacter::PickUpAction);

		// Reload
		// EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Reload_Action);

		// Fire/Attack
		// EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &APlayerCharacter::StartFiring);
		// EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &APlayerCharacter::EndFiring);
	}
}



void APlayerCharacter::Move(const FInputActionValue& Value) {
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

void APlayerCharacter::Look(const FInputActionValue& Value) {
	const FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::ToggleWalk(const FInputActionValue& Value)
{
	if (CurrentGait == EGait::Walk)
	{
		CurrentGait = EGait::Run;
		GetCharacterMovement()->MaxWalkSpeed = 375.0f;
		return;
	}
	
	CurrentGait = EGait::Walk;
	GetCharacterMovement()->MaxWalkSpeed = 180.0f;
}