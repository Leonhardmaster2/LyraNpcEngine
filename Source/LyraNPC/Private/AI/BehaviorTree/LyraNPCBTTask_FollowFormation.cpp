// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_FollowFormation.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGroupCoordinationComponent.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_FollowFormation::ULyraNPCBTTask_FollowFormation()
{
	NodeName = "Follow Formation";
	bNotifyTick = true;
	bCreateNodeInstance = true;
}

uint16 ULyraNPCBTTask_FollowFormation::GetInstanceMemorySize() const
{
	return sizeof(FBTFollowFormationMemory);
}

EBTNodeResult::Type ULyraNPCBTTask_FollowFormation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTFollowFormationMemory* Memory = CastInstanceNodeMemory<FBTFollowFormationMemory>(NodeMemory);
	if (!Memory)
	{
		return EBTNodeResult::Failed;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ALyraNPCCharacter* NPCCharacter = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPCCharacter)
	{
		return EBTNodeResult::Failed;
	}

	ULyraNPCIdentityComponent* Identity = NPCCharacter->IdentityComponent;
	if (!Identity)
	{
		return EBTNodeResult::Failed;
	}

	// Find our group
	ULyraNPCGroupCoordinationComponent* MyGroup = nullptr;
	UWorld* World = NPCCharacter->GetWorld();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ULyraNPCGroupCoordinationComponent* GroupComp = It->FindComponentByClass<ULyraNPCGroupCoordinationComponent>();
		if (GroupComp && GroupComp->IsMember(Identity->GetUniqueId()))
		{
			MyGroup = GroupComp;
			break;
		}
	}

	if (!MyGroup)
	{
		UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s is not in any group"), *NPCCharacter->GetNPCName());
		return EBTNodeResult::Failed;
	}

	// Get formation position
	FVector FormationPosition = MyGroup->GetFormationPositionForMember(Identity->GetUniqueId());
	Memory->LastTargetPosition = FormationPosition;

	// Check if already at formation position
	float Distance = FVector::Dist(NPCCharacter->GetActorLocation(), FormationPosition);
	if (Distance <= AcceptableRadius)
	{
		if (!bContinuousFollow)
		{
			return EBTNodeResult::Succeeded; // Already there
		}
		// In continuous mode, stay in progress to maintain position
	}

	// Move to formation position
	Memory->bMovementStarted = false;
	return EBTNodeResult::InProgress;
}

void ULyraNPCBTTask_FollowFormation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTFollowFormationMemory* Memory = CastInstanceNodeMemory<FBTFollowFormationMemory>(NodeMemory);
	if (!Memory)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ALyraNPCCharacter* NPCCharacter = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPCCharacter)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	ULyraNPCIdentityComponent* Identity = NPCCharacter->IdentityComponent;
	if (!Identity)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Find our group
	ULyraNPCGroupCoordinationComponent* MyGroup = nullptr;
	UWorld* World = NPCCharacter->GetWorld();

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		ULyraNPCGroupCoordinationComponent* GroupComp = It->FindComponentByClass<ULyraNPCGroupCoordinationComponent>();
		if (GroupComp && GroupComp->IsMember(Identity->GetUniqueId()))
		{
			MyGroup = GroupComp;
			break;
		}
	}

	if (!MyGroup)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Get updated formation position
	FVector FormationPosition = MyGroup->GetFormationPositionForMember(Identity->GetUniqueId());

	// Check if formation position has changed significantly
	if (FVector::Dist(FormationPosition, Memory->LastTargetPosition) > 50.0f || !Memory->bMovementStarted)
	{
		Memory->LastTargetPosition = FormationPosition;
		Memory->bMovementStarted = true;

		// Move to new formation position
		AIController->MoveToLocation(FormationPosition, AcceptableRadius);
	}

	// Check if we've reached formation position
	float Distance = FVector::Dist(NPCCharacter->GetActorLocation(), FormationPosition);
	if (Distance <= AcceptableRadius)
	{
		if (!bContinuousFollow)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		}
		// In continuous mode, keep running
	}
}

EBTNodeResult::Type ULyraNPCBTTask_FollowFormation::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		AIController->StopMovement();
	}

	return EBTNodeResult::Aborted;
}
