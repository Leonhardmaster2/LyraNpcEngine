// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LyraNPCBTTask_FollowFormation.generated.h"

/**
 * Behavior Tree task to move to and maintain a formation position within a group.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_FollowFormation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_FollowFormation();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

protected:
	// Acceptable distance from formation position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	float AcceptableRadius = 150.0f;

	// Movement speed modifier when in formation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	float FormationSpeedMultiplier = 0.8f;

	// Continuously update position (vs. move once)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Formation")
	bool bContinuousFollow = true;
};

struct FBTFollowFormationMemory
{
	FVector LastTargetPosition = FVector::ZeroVector;
	bool bMovementStarted = false;
};
