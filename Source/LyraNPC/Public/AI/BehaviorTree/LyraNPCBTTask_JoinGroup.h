// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AI/Group/LyraNPCGroupTypes.h"
#include "LyraNPCBTTask_JoinGroup.generated.h"

/**
 * Behavior Tree task to join a nearby group.
 * NPCs will seek out and join groups based on archetype and social relationships.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_JoinGroup : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_JoinGroup();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	// Group ID to join (if empty, will find nearest compatible group)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	FName TargetGroupId;

	// Preferred role in the group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	ELyraNPCGroupRole PreferredRole = ELyraNPCGroupRole::Follower;

	// Search radius for finding groups
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
	float SearchRadius = 2000.0f;
};
