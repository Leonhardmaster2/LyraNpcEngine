// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_JoinGroup.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGroupCoordinationComponent.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "EngineUtils.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_JoinGroup::ULyraNPCBTTask_JoinGroup()
{
	NodeName = "Join Group";
}

EBTNodeResult::Type ULyraNPCBTTask_JoinGroup::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
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

	UWorld* World = NPCCharacter->GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	// Find group coordination components in the world
	ULyraNPCGroupCoordinationComponent* TargetGroup = nullptr;

	if (!TargetGroupId.IsNone())
	{
		// Look for specific group
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			ULyraNPCGroupCoordinationComponent* GroupComp = It->FindComponentByClass<ULyraNPCGroupCoordinationComponent>();
			if (GroupComp && GroupComp->GroupId == TargetGroupId)
			{
				TargetGroup = GroupComp;
				break;
			}
		}
	}
	else
	{
		// Find nearest compatible group
		float ClosestDistance = SearchRadius;
		FVector NPCLocation = NPCCharacter->GetActorLocation();

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			ULyraNPCGroupCoordinationComponent* GroupComp = It->FindComponentByClass<ULyraNPCGroupCoordinationComponent>();
			if (GroupComp && GroupComp->GetMemberCount() < GroupComp->MaxGroupSize)
			{
				// Check if we're already in this group
				ULyraNPCIdentityComponent* Identity = NPCCharacter->IdentityComponent;
				if (Identity && GroupComp->IsMember(Identity->GetUniqueId()))
				{
					continue; // Already in this group
				}

				// Find distance to group center
				float Distance = FVector::Dist(NPCLocation, GroupComp->FormationCenter);
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					TargetGroup = GroupComp;
				}
			}
		}
	}

	// Join the group
	if (TargetGroup)
	{
		if (TargetGroup->AddMember(NPCCharacter, PreferredRole))
		{
			UE_LOG(LogLyraNPC, Log, TEXT("NPC %s joined group %s"),
				*NPCCharacter->GetNPCName(), *TargetGroup->GroupId.ToString());
			return EBTNodeResult::Succeeded;
		}
	}

	UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s could not find group to join"), *NPCCharacter->GetNPCName());
	return EBTNodeResult::Failed;
}
