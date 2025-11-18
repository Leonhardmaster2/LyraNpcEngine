// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTDecorator_CheckGOAPGoal.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGOAPComponent.h"

ULyraNPCBTDecorator_CheckGOAPGoal::ULyraNPCBTDecorator_CheckGOAPGoal()
{
	NodeName = "Check GOAP Goal";
}

bool ULyraNPCBTDecorator_CheckGOAPGoal::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	ALyraNPCCharacter* NPCCharacter = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPCCharacter)
	{
		return false;
	}

	ULyraNPCGOAPComponent* GOAPComp = NPCCharacter->FindComponentByClass<ULyraNPCGOAPComponent>();
	if (!GOAPComp)
	{
		return false;
	}

	if (!GoalTag.IsValid())
	{
		return false;
	}

	// Check if the specified goal is the current best goal
	FLyraNPCGoal BestGoal = GOAPComp->SelectBestGoal();

	if (BestGoal.GoalTag == GoalTag && BestGoal.CurrentPriority >= MinimumPriority)
	{
		return true;
	}

	return false;
}
