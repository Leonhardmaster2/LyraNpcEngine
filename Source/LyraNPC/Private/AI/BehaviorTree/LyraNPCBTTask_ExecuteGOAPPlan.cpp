// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_ExecuteGOAPPlan.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCGOAPComponent.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_ExecuteGOAPPlan::ULyraNPCBTTask_ExecuteGOAPPlan()
{
	NodeName = "Execute GOAP Plan";
	bNotifyTick = true;
	bCreateNodeInstance = true; // Each BT instance gets its own memory
}

uint16 ULyraNPCBTTask_ExecuteGOAPPlan::GetInstanceMemorySize() const
{
	return sizeof(FBTExecuteGOAPPlanMemory);
}

EBTNodeResult::Type ULyraNPCBTTask_ExecuteGOAPPlan::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FBTExecuteGOAPPlanMemory* Memory = CastInstanceNodeMemory<FBTExecuteGOAPPlanMemory>(NodeMemory);
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

	// Find GOAP component
	ULyraNPCGOAPComponent* GOAPComp = NPCCharacter->FindComponentByClass<ULyraNPCGOAPComponent>();
	if (!GOAPComp)
	{
		UE_LOG(LogLyraNPC, Warning, TEXT("NPC %s has no GOAP component!"), *NPCCharacter->GetName());
		return EBTNodeResult::Failed;
	}

	// Initialize memory
	Memory->bPlanningComplete = false;
	Memory->bExecutingPlan = false;
	Memory->PlanningTimeElapsed = 0.0f;

	// Check if we need to create a new plan
	if (bForceNewPlan || !GOAPComp->HasActivePlan())
	{
		// Select best goal and create plan
		FLyraNPCGoal BestGoal = GOAPComp->SelectBestGoal();

		if (!BestGoal.GoalTag.IsValid())
		{
			UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s has no valid goals"), *NPCCharacter->GetName());
			return EBTNodeResult::Failed;
		}

		FLyraNPCActionPlan NewPlan;
		if (GOAPComp->CreatePlan(BestGoal, NewPlan))
		{
			Memory->bPlanningComplete = true;
			UE_LOG(LogLyraNPC, Log, TEXT("NPC %s created GOAP plan with %d steps"),
				*NPCCharacter->GetName(), NewPlan.Steps.Num());
		}
		else
		{
			UE_LOG(LogLyraNPC, Warning, TEXT("NPC %s failed to create GOAP plan"), *NPCCharacter->GetName());
			return EBTNodeResult::Failed;
		}
	}
	else
	{
		Memory->bPlanningComplete = true;
	}

	// Start executing the plan
	if (Memory->bPlanningComplete)
	{
		if (GOAPComp->StartPlanExecution())
		{
			Memory->bExecutingPlan = true;
			return EBTNodeResult::InProgress; // Will tick to execute
		}
	}

	return EBTNodeResult::Failed;
}

void ULyraNPCBTTask_ExecuteGOAPPlan::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FBTExecuteGOAPPlanMemory* Memory = CastInstanceNodeMemory<FBTExecuteGOAPPlanMemory>(NodeMemory);
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

	ULyraNPCGOAPComponent* GOAPComp = NPCCharacter->FindComponentByClass<ULyraNPCGOAPComponent>();
	if (!GOAPComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// Execute current action
	if (Memory->bExecutingPlan)
	{
		GOAPComp->ExecuteCurrentAction(DeltaSeconds);

		// Check if plan is complete
		if (GOAPComp->IsPlanComplete())
		{
			UE_LOG(LogLyraNPC, Log, TEXT("NPC %s completed GOAP plan"), *NPCCharacter->GetName());
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}

		// Check if plan is still valid
		if (bReplanIfInvalid && !GOAPComp->IsPlanValid())
		{
			UE_LOG(LogLyraNPC, Log, TEXT("NPC %s plan became invalid, replanning"), *NPCCharacter->GetName());

			// Try to replan
			FLyraNPCGoal CurrentGoal = GOAPComp->GetCurrentGoal();
			FLyraNPCActionPlan NewPlan;

			if (GOAPComp->CreatePlan(CurrentGoal, NewPlan))
			{
				GOAPComp->StartPlanExecution();
			}
			else
			{
				UE_LOG(LogLyraNPC, Warning, TEXT("NPC %s failed to replan"), *NPCCharacter->GetName());
				FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
				return;
			}
		}

		// Check if plan failed
		if (GOAPComp->HasPlanFailed())
		{
			UE_LOG(LogLyraNPC, Warning, TEXT("NPC %s plan failed"), *NPCCharacter->GetName());
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}
	}
}

EBTNodeResult::Type ULyraNPCBTTask_ExecuteGOAPPlan::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (AIController)
	{
		ALyraNPCCharacter* NPCCharacter = Cast<ALyraNPCCharacter>(AIController->GetPawn());
		if (NPCCharacter)
		{
			ULyraNPCGOAPComponent* GOAPComp = NPCCharacter->FindComponentByClass<ULyraNPCGOAPComponent>();
			if (GOAPComp)
			{
				// Cancel current plan
				GOAPComp->CancelCurrentPlan();
			}
		}
	}

	return EBTNodeResult::Aborted;
}
