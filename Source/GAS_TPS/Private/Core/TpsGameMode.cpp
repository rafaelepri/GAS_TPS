// Copyright (c) 2025 Lalinha

#include "Core/TpsGameMode.h"

#include "Character/TpsCharacter.h"
#include "Core/TpsGamePlayerState.h"
#include "Core/TpsGameState.h"

ATpsGameMode::ATpsGameMode() {
	DefaultPawnClass = ATpsCharacter::StaticClass();
	GameStateClass = ATpsGameState::StaticClass();
	PlayerStateClass = ATpsGamePlayerState::StaticClass();
}
