// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_FollowPath.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "Core/LyraNPCCharacter.h"
#include "Navigation/LyraNPCPathFollowingComponent.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_FollowPath::ULyraNPCBTTask_FollowPath()
{
	NodeName = "Follow Predetermined Path";
	bNotifyTick = true;
}

uint16 ULyraNPCBTTask_FollowPath::GetInstanceMemorySize() const
{
	return sizeof(FPathMemory);
}

EBTNodeResult::Type ULyraNPCBTTask_FollowPath::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FPathMemory* Memory = reinterpret_cast<FPathMemory*>(NodeMemory);
	Memory->PointsVisited = 0;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ALyraNPCCharacter* NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPC || !NPC->PathFollowingComponent)
	{
		return EBTNodeResult::Failed;
	}

	ULyraNPCPathFollowingComponent* PathComp = NPC->PathFollowingComponent;
	if (PathComp->PathPoints.Num() == 0)
	{
		UE_LOG(LogLyraNPC, Warning, TEXT("FollowPath: No path points defined"));
		return EBTNodeResult::Failed;
	}

	Memory->StartingPointIndex = PathComp->CurrentPointIndex;
	Memory->bWasFollowingPath = PathComp->bIsFollowingPath;

	PathComp->StartFollowingPath();

	// Update life state
	if (NPC->IdentityComponent)
	{
		NPC->IdentityComponent->SetLifeState(ELyraNPCLifeState::Traveling);
	}

	// Set movement style from current path point
	NPC->SetMovementStyle(PathComp->GetCurrentMovementStyle());

	UE_LOG(LogLyraNPC, Verbose, TEXT("Started following path"));
	return EBTNodeResult::InProgress;
}

void ULyraNPCBTTask_FollowPath::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FPathMemory* Memory = reinterpret_cast<FPathMemory*>(NodeMemory);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ALyraNPCCharacter* NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPC || !NPC->PathFollowingComponent)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ULyraNPCPathFollowingComponent* PathComp = NPC->PathFollowingComponent;

	// Check if we've reached the current point
	if (PathComp->HasReachedCurrentPoint())
	{
		if (!PathComp->bIsWaitingAtPoint)
		{
			PathComp->OnReachedPathPoint();
			Memory->PointsVisited++;

			// Update movement style for next point
			NPC->SetMovementStyle(PathComp->GetCurrentMovementStyle());

			UE_LOG(LogLyraNPC, Verbose, TEXT("Reached path point %d"), Memory->PointsVisited);

			// Check if we've visited enough points
			if (PointsToVisit > 0 && Memory->PointsVisited >= PointsToVisit)
			{
				PathComp->StopFollowingPath();
				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return;
			}
		}
	}
	else
	{
		// Move towards current target
		FVector TargetLocation = PathComp->GetCurrentTargetLocation();

		float AcceptRadius = OverrideAcceptanceRadius > 0.0f
			? OverrideAcceptanceRadius
			: PathComp->GetCurrentPathPoint().AcceptanceRadius;

		AIController->MoveToLocation(TargetLocation, AcceptRadius, true, true, false, true);
	}

	// Check for path completion
	if (PathComp->IsPathComplete())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type ULyraNPCBTTask_FollowPath::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ALyraNPCCharacter* NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
		if (NPC && NPC->PathFollowingComponent)
		{
			NPC->PathFollowingComponent->StopFollowingPath();
		}

		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}

FString ULyraNPCBTTask_FollowPath::GetStaticDescription() const
{
	if (PointsToVisit > 0)
	{
		return FString::Printf(TEXT("Follow path for %d points"), PointsToVisit);
	}
	return TEXT("Follow entire predetermined path");
}
