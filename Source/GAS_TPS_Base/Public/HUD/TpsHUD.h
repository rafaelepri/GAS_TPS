// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TpsHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UTexture2D* CrosshairCenter;
	UTexture2D* CrosshairLeft;
	UTexture2D* CrosshairRight;
	UTexture2D* CrosshairTop;
	UTexture2D* CrosshairBottom;
	float CrosshairSpread;
	FLinearColor CrosshairColor = FLinearColor::White;
};

/**
 * 
 */
UCLASS()
class GAS_TPS_BASE_API ATpsHUD : public AHUD {
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

	UPROPERTY(EditAnywhere, Category = "PlayerStats")
	TSubclassOf<UUserWidget> CharacterOverlayClass;
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	void AddCharacterOverlay();

	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementOverlayClass;
	UPROPERTY()
	class UAnnouncement* AnnouncementOverlay;
	void AddAnnouncementOverlay();
protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	void DrawHUDCrosshairs(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& Color);

	UPROPERTY(EditAnywhere)
	float CrossHairSpreadMax = 16.f;
public:
	FORCEINLINE void setHUDPackage(const FHUDPackage& Value) { HUDPackage = Value; }
};
