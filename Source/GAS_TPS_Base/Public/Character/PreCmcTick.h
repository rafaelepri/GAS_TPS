
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PreCmcTick.generated.h"

class ATPSCharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAS_TPS_BASE_API UPreCmcTick : public UActorComponent  {
	GENERATED_BODY()

public:	
	UPreCmcTick();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	ATPSCharacterBase* CharacterBase;

public:	
	virtual void TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
	

