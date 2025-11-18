// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LyraNPCBTTask_ExecuteGOAPPlan.generated.h"

/**
 * Behavior Tree task that creates and executes a GOAP plan.
 * This allows NPCs to dynamically plan multi-step actions to achieve goals.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_ExecuteGOAPPlan : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_ExecuteGOAPPlan();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	// If true, will create a new plan. If false, will continue existing plan.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	bool bForceNewPlan = false;

	// Maximum time to spend planning (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float MaxPlanningTime = 0.5f;

	// Replan if current plan becomes invalid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	bool bReplanIfInvalid = true;
};

// Memory structure for this task
struct FBTExecuteGOAPPlanMemory
{
	bool bPlanningComplete = false;
	bool bExecutingPlan = false;
	float PlanningTimeElapsed = 0.0f;
};
