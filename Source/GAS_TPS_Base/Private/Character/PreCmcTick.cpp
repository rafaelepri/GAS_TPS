// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PreCmcTick.h"
#include "GAS_TPS_Base/Public/Character/TpsCharacterBase.h"

UPreCmcTick::UPreCmcTick() {
	PrimaryComponentTick.bCanEverTick = true;
}

void UPreCmcTick::BeginPlay() {
	Super::BeginPlay();

	CharacterBase = Cast<ATPSCharacterBase>(GetOwner());
}

void UPreCmcTick::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) {

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CharacterBase) {
		CharacterBase->UpdateRotation_PreCmc();
		CharacterBase->UpdateMovement_PreCmc();
		CharacterBase->UpdateCamera_PreCmc(true);
	}
}