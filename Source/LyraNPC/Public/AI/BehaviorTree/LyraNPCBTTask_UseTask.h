// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LyraNPCBTTask_UseTask.generated.h"

/**
 * BT Task: Uses a task actor for a duration.
 * The NPC will stay at the task location and receive need satisfaction.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_UseTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_UseTask();

	// Blackboard key containing the task to use
	UPROPERTY(EditAnywhere, Category = "Task")
	FBlackboardKeySelector TaskKey;

	// Override duration (0 = use task's default)
	UPROPERTY(EditAnywhere, Category = "Task")
	float OverrideDuration = 0.0f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	struct FTaskMemory
	{
		float RemainingTime;
		bool bTaskStarted;
	};
};
