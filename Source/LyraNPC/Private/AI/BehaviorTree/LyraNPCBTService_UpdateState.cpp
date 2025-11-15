// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTService_UpdateState.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/Controllers/LyraNPCAIController.h"
#include "LyraNPCModule.h"

ULyraNPCBTService_UpdateState::ULyraNPCBTService_UpdateState()
{
	NodeName = "Update NPC State";

	// Run every 0.5 seconds by default
	Interval = 0.5f;
	RandomDeviation = 0.1f;
}

void ULyraNPCBTService_UpdateState::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ALyraNPCAIController* AIController = Cast<ALyraNPCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return;
	}

	// Let the AI controller handle the update
	AIController->UpdateBlackboardFromComponents();
}

FString ULyraNPCBTService_UpdateState::GetStaticDescription() const
{
	TArray<FString> Updates;

	if (bUpdateNeeds) Updates.Add(TEXT("Needs"));
	if (bUpdateSchedule) Updates.Add(TEXT("Schedule"));
	if (bUpdateAlertness) Updates.Add(TEXT("Alertness"));
	if (bUpdateCognitive) Updates.Add(TEXT("Cognitive"));

	return FString::Printf(TEXT("Updates: %s"), *FString::Join(Updates, TEXT(", ")));
}
