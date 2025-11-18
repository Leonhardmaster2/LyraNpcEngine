// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "GameplayTagContainer.h"
#include "LyraNPCBTDecorator_CheckGOAPGoal.generated.h"

/**
 * Decorator that checks if a specific GOAP goal is active or should be pursued.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTDecorator_CheckGOAPGoal : public UBTDecorator
{
	GENERATED_BODY()

public:
	ULyraNPCBTDecorator_CheckGOAPGoal();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	// Goal tag to check for
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	FGameplayTag GoalTag;

	// Minimum priority threshold to consider this goal active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float MinimumPriority = 0.5f;
};
