#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GAS_TPS_Base.generated.h"

#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1

UENUM(BlueprintType)
enum class EGait : uint8 {
    Walk UMETA(DisplayName = "Walk"),
    Run UMETA(DisplayName = "Run"),
    Sprint UMETA(DisplayName = "Sprint"),
};

UENUM(BlueprintType)
enum class ETraversalActionType : uint8
{
    None = 0,
    // Traverse over a thin object and end on the ground at a similar level (Low fence)
    Hurdle = 1,
    // Traverse over a thin object and end in a falling state (Tall fence, or elevated obstacle with no floor on the other side)
    Vault = 2,
    // Traverse up and onto an object without passing over it
    Mantle = 3
};

class FGAS_TPS_BaseModule : public IModuleInterface {
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};

USTRUCT(BlueprintType)
struct FTraversalParams : public FTableRowBase {
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    ETraversalActionType ActionType;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    bool HasFrontLedge = false;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    FVector FrontLedgeLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    FVector FrontLedgeNormal = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    bool HasBackLedge = false;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    FVector BackLedgeLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    FVector BackLedgeNormal = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    bool HasBackFloor = false;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    FVector BackFloorLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float ObstacleHeight = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float ObstacleDepth = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float BackLedgeHeight = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    UPrimitiveComponent* HitComponent = nullptr;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    const UAnimMontage* ChosenMontage = nullptr;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float StartTime = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float PlayRate = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Check Result")
    float Speed = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Chooser Inputs")
    TEnumAsByte<EMovementMode> NewMovementMode;
};

USTRUCT(BlueprintType)
struct FTraversalCheckInputs : public FTableRowBase {
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    FVector TraceForwardDirection = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    float TraceForwardDistence = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    FVector TraceOriginOffset = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    FVector TraceEndOffset = FVector::ZeroVector;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    float TraceRadius = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "Traversal Input")
    float TraceHalfHeight = 0.f;
};

UENUM(BlueprintType)
enum class EMovementDirection : uint8
{
    F        UMETA(DisplayName = "F"),
    B        UMETA(DisplayName = "B"),
    LL       UMETA(DisplayName = "LL"),
    LR       UMETA(DisplayName = "LR"),
    RL       UMETA(DisplayName = "RL"),
    RR       UMETA(DisplayName = "RR")
};

USTRUCT(BlueprintType)
struct FMovementDirectionThreshold : public FTableRowBase {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "MovementDirectionThreshold")
    float FL = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "MovementDirectionThreshold")
    float FR = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "MovementDirectionThreshold")
    float BL = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "MovementDirectionThreshold")
    float BR = 0.f;
};

UENUM(BlueprintType)
enum class EMovementDirectionBias : uint8
{
    LeftFootForward      UMETA(DisplayName = "LeftFootForward"),
    RightFootForward     UMETA(DisplayName = "RightFootForward")
};

UENUM(BlueprintType)
enum class EMovementType : uint8
{
    OnGround     UMETA(DisplayName = "OnGround"),
    InAir        UMETA(DisplayName = "InAir"),
};

USTRUCT(BlueprintType)
struct FBlendStackInputs : public FTableRowBase {
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    UAnimationAsset* Anim;
    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    bool Loop = false;
    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    float StartTime = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    float BlendTime = 0.f;
    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    UBlendProfile* BlendProfile;
    UPROPERTY(BlueprintReadWrite, Category = "BlendStackInputs")
    TArray<FName> Tags;
};

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Idle         UMETA(DisplayName = "Idle"),
    Moving       UMETA(DisplayName = "Moving")
};

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
    OrientToRotation     UMETA(DisplayName = "OrientToRotation"),
    Strafe               UMETA(DisplayName = "Strafe")
};

UENUM(BlueprintType)
enum class EStance : uint8
{
    Stand         UMETA(DisplayName = "Stand"),
    Crouch        UMETA(DisplayName = "Crouch")
};

