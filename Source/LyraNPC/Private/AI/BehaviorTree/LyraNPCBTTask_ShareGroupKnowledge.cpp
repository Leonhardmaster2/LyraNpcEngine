// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_ShareGroupKnowledge.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGroupCoordinationComponent.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_ShareGroupKnowledge::ULyraNPCBTTask_ShareGroupKnowledge()
{
	NodeName = "Share Group Knowledge";
}

EBTNodeResult::Type ULyraNPCBTTask_ShareGroupKnowledge::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
		UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s is not in any group, cannot share knowledge"), *NPCCharacter->GetNPCName());
		return EBTNodeResult::Failed;
	}

	// Get blackboard
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	// Create knowledge to share
	FLyraNPCGroupKnowledge Knowledge;
	Knowledge.Type = KnowledgeType;
	Knowledge.ReporterId = Identity->GetUniqueId();
	Knowledge.Timestamp = World->GetTimeSeconds();
	Knowledge.Urgency = Urgency;
	Knowledge.Accuracy = 0.9f;

	// Get location from blackboard
	if (LocationKey.SelectedKeyType == UBlackboardKeyType_Vector::StaticClass())
	{
		Knowledge.Location = BlackboardComp->GetValueAsVector(LocationKey.SelectedKeyName);
	}
	else
	{
		Knowledge.Location = NPCCharacter->GetActorLocation();
	}

	// Get related actor from blackboard
	if (TargetActorKey.SelectedKeyType == UBlackboardKeyType_Object::StaticClass())
	{
		AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (TargetActor)
		{
			Knowledge.RelatedActor = TargetActor;
			Knowledge.Location = TargetActor->GetActorLocation();
		}
	}

	// Share the knowledge with the group
	MyGroup->ShareKnowledge(Knowledge);

	UE_LOG(LogLyraNPC, Log, TEXT("NPC %s shared knowledge type %d with group %s"),
		*NPCCharacter->GetNPCName(), (int32)KnowledgeType, *MyGroup->GroupId.ToString());

	return EBTNodeResult::Succeeded;
}
