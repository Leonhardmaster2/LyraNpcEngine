// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "LyraNPCBTTask_FindTask.generated.h"

/**
 * BT Task: Finds the best available task for the NPC based on needs and schedule.
 * Sets the found task in the blackboard.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_FindTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_FindTask();

	// Filter by specific task type (optional)
	UPROPERTY(EditAnywhere, Category = "Task")
	FGameplayTag TaskTypeFilter;

	// Blackboard key to store found task
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetTaskKey;

	// Blackboard key to store task location
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TaskLocationKey;

	// Maximum search radius
	UPROPERTY(EditAnywhere, Category = "Task")
	float SearchRadius = 10000.0f;

	// Use schedule to determine task type
	UPROPERTY(EditAnywhere, Category = "Task")
	bool bUseScheduleForTaskType = true;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual FString GetStaticDescription() const override;
};
