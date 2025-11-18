// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "LyraNPCBTService_UpdateGOAPState.generated.h"

/**
 * Behavior Tree service that periodically updates the GOAP world state
 * based on the NPC's current situation (needs, location, combat status, etc.)
 */
UCLASS()
class LYRANPC_API ULyraNPCBTService_UpdateGOAPState : public UBTService
{
	GENERATED_BODY()

public:
	ULyraNPCBTService_UpdateGOAPState();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	// Update frequency (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float UpdateInterval = 1.0f;
};
