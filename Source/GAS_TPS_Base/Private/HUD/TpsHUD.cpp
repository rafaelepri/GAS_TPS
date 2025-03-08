// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/TpsHUD.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "HUD/CharacterOverlay.h"
#include "HUD/Widget/Announcement.h"



void ATpsHUD::BeginPlay() {
	Super::BeginPlay();
}

void ATpsHUD::AddCharacterOverlay() {
	APlayerController* PlayerController = GetOwningPlayerController();  // this cant be const
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}

void ATpsHUD::AddAnnouncementOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController(); // this cant be const
	if (PlayerController && AnnouncementOverlayClass)
	{
		AnnouncementOverlay = CreateWidget<UAnnouncement>(PlayerController, AnnouncementOverlayClass);
		AnnouncementOverlay->AddToViewport();
	}
}

void ATpsHUD::DrawHUD() {
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;

			
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter = ViewportSize * 0.5f;

		const float SpreadScaled = CrossHairSpreadMax * HUDPackage.CrosshairSpread;
		

		if (HUDPackage.CrosshairCenter)
		{
			const FVector2D Spread(0.f, 0.f);
			DrawHUDCrosshairs(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairLeft)
		{
			const FVector2D Spread(-SpreadScaled, 0.f);
			DrawHUDCrosshairs(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairRight)
		{
			const FVector2D Spread(SpreadScaled, 0.f);
			DrawHUDCrosshairs(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairTop)
		{
			const FVector2D Spread( 0.f,-SpreadScaled);
			DrawHUDCrosshairs(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairBottom)
		{
			const FVector2D Spread( 0.f,SpreadScaled);
			DrawHUDCrosshairs(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}


void ATpsHUD::DrawHUDCrosshairs(UTexture2D* Texture, const FVector2D& ViewportCenter, const FVector2D& Spread, const FLinearColor& Color) {
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y 
	);

	DrawTexture(
		Texture,
		TextureDrawPoint.X,
		TextureDrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		Color
	);
}
