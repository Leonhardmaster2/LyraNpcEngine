// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "LyraNPCBTTask_FollowPath.generated.h"

/**
 * BT Task: Follows the predetermined path set in the PathFollowingComponent.
 * Great for patrol routes, travel paths, or wandering.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTTask_FollowPath : public UBTTaskNode
{
	GENERATED_BODY()

public:
	ULyraNPCBTTask_FollowPath();

	// How many path points to visit before completing (-1 = entire path)
	UPROPERTY(EditAnywhere, Category = "Path")
	int32 PointsToVisit = -1;

	// Should automatically move to next point after reaching one?
	UPROPERTY(EditAnywhere, Category = "Path")
	bool bAutoAdvance = true;

	// Accept radius for path points (overrides individual point settings if > 0)
	UPROPERTY(EditAnywhere, Category = "Path")
	float OverrideAcceptanceRadius = 0.0f;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual FString GetStaticDescription() const override;

protected:
	struct FPathMemory
	{
		int32 PointsVisited;
		int32 StartingPointIndex;
		bool bWasFollowingPath;
	};
};
