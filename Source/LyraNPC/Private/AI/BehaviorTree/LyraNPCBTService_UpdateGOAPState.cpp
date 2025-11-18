// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTService_UpdateGOAPState.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGOAPComponent.h"
#include "LyraNPCModule.h"

ULyraNPCBTService_UpdateGOAPState::ULyraNPCBTService_UpdateGOAPState()
{
	NodeName = "Update GOAP State";
	Interval = UpdateInterval;
	RandomDeviation = 0.1f;
}

void ULyraNPCBTService_UpdateGOAPState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return;
	}

	ALyraNPCCharacter* NPCCharacter = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPCCharacter)
	{
		return;
	}

	// Find GOAP component
	ULyraNPCGOAPComponent* GOAPComp = NPCCharacter->FindComponentByClass<ULyraNPCGOAPComponent>();
	if (!GOAPComp)
	{
		return;
	}

	// Update world state (this is already done automatically by the GOAP component)
	// But we can trigger goal priority updates here
	GOAPComp->UpdateGoalPriorities();

	UE_LOG(LogLyraNPC, VeryVerbose, TEXT("Updated GOAP state for %s"), *NPCCharacter->GetName());
}
