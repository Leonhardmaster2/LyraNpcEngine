// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "LyraNPCBTService_UpdateState.generated.h"

/**
 * BT Service: Continuously updates NPC state in the blackboard.
 * This service runs periodically and keeps the blackboard synced with component data.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTService_UpdateState : public UBTService
{
	GENERATED_BODY()

public:
	ULyraNPCBTService_UpdateState();

	// Update needs state
	UPROPERTY(EditAnywhere, Category = "Update")
	bool bUpdateNeeds = true;

	// Update schedule state
	UPROPERTY(EditAnywhere, Category = "Update")
	bool bUpdateSchedule = true;

	// Update alertness state
	UPROPERTY(EditAnywhere, Category = "Update")
	bool bUpdateAlertness = true;

	// Update cognitive modifiers
	UPROPERTY(EditAnywhere, Category = "Update")
	bool bUpdateCognitive = true;

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual FString GetStaticDescription() const override;
};
