// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/Group/LyraNPCGroupTypes.h"
#include "LyraNPCBTTask_ShareGroupKnowledge.generated.h"

/**
 * Behavior Tree task to share knowledge with group members.
 * Reports enemies, threats, and other important information.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_ShareGroupKnowledge : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_ShareGroupKnowledge();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Type of knowledge to share
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	ELyraNPCGroupKnowledgeType KnowledgeType = ELyraNPCGroupKnowledgeType::EnemySighting;

	// Blackboard key for target actor (for enemy sightings, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FBlackboardKeySelector TargetActorKey;

	// Blackboard key for location (for threat locations, etc.)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FBlackboardKeySelector LocationKey;

	// Urgency level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	float Urgency = 0.8f;
};
