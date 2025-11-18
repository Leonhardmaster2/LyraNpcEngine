// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/Actions/LyraNPCGOAPAction_UseTask.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"

ULyraNPCGOAPAction_UseTask::ULyraNPCGOAPAction_UseTask()
{
	ActionName = FName("UseTask");
	ActionDescription = FText::FromString("Find and use a task actor");

	// Generic preconditions - will be customized based on task type
	Preconditions.StateFlags.Add(FName("HasTaskAvailable"), true);

	// Generic effects
	Effects.StateFlags.Add(FName("TaskCompleted"), true);

	// Base cost
	Cost.BaseCost = 10.0f;
	Cost.bRequiresMovement = true;
}

void ULyraNPCGOAPAction_UseTask::SetTaskType(FGameplayTag InTaskType)
{
	TaskType = InTaskType;
	ActionName = FName(*FString::Printf(TEXT("UseTask_%s"), *TaskType.ToString()));
}

bool ULyraNPCGOAPAction_UseTask::CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC || !TaskType.IsValid()) return false;

	// Check if the associated need requires attention
	if (TargetNeed != ELyraNPCNeedType::Hunger) // Hunger is default, check if it's set
	{
		ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent;
		if (Needs)
		{
			float NeedValue = Needs->GetNeedValue(TargetNeed);
			if (NeedValue < MinimumNeedValue)
			{
				return false; // Need is not urgent enough
			}
		}
	}

	// Check if task is available in the world
	UWorld* World = NPC->GetWorld();
	if (!World) return false;

	ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
	if (!WorldSubsystem) return false;

	// Look for matching tasks
	TArray<ULyraNPCTaskActor*> AllTasks = WorldSubsystem->GetAllTasks();
	for (ULyraNPCTaskActor* Task : AllTasks)
	{
		if (Task && Task->TaskType.MatchesTag(TaskType))
		{
			if (Task->CanBeUsedBy(NPC))
			{
				return true; // Task is available
			}
		}
	}

	return false;
}

FLyraNPCWorldState ULyraNPCGOAPAction_UseTask::ApplyEffects(const FLyraNPCWorldState& CurrentState) const
{
	FLyraNPCWorldState NewState = CurrentState;
	NewState.ApplyState(Effects);
	return NewState;
}

float ULyraNPCGOAPAction_UseTask::CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC || !TaskType.IsValid()) return 9999.0f;

	float TotalCost = Cost.BaseCost;

	// Find nearest matching task
	UWorld* World = NPC->GetWorld();
	if (World)
	{
		ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
		if (WorldSubsystem)
		{
			ULyraNPCTaskActor* BestTask = WorldSubsystem->FindBestTaskForNPC(NPC, TaskType);

			if (BestTask)
			{
				float Distance = FVector::Dist(NPC->GetActorLocation(), BestTask->GetActorLocation());
				TotalCost += (Distance / 100.0f) * Cost.DistanceCostMultiplier;

				// Factor in need urgency (lower need = higher cost, making it less likely)
				if (ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent)
				{
					float NeedValue = Needs->GetNeedValue(TargetNeed);
					float NeedUrgency = 100.0f - NeedValue; // Inverse: low value = high urgency
					TotalCost *= (1.0f - (NeedUrgency / 200.0f)); // Reduce cost for urgent needs
				}
			}
			else
			{
				return 9999.0f; // No task available
			}
		}
	}

	return TotalCost;
}

void ULyraNPCGOAPAction_UseTask::OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor)
{
	TimeSpentOnTask = 0.0f;
	CurrentTask = Cast<ULyraNPCTaskActor>(TargetActor);

	if (CurrentTask.IsValid())
	{
		CurrentTask->StartUsing(NPC);
	}
}

bool ULyraNPCGOAPAction_UseTask::TickAction(ALyraNPCCharacter* NPC, float DeltaTime)
{
	if (!NPC) return false;

	TimeSpentOnTask += DeltaTime;

	// Apply task benefits
	if (CurrentTask.IsValid() && NPC->NeedsComponent)
	{
		// Get need satisfaction from task
		const TMap<ELyraNPCNeedType, float>& Satisfaction = CurrentTask->NeedsSatisfaction;

		for (const auto& Pair : Satisfaction)
		{
			float RestorationPerSecond = Pair.Value / TaskDuration;
			float CurrentValue = NPC->NeedsComponent->GetNeedValue(Pair.Key);
			NPC->NeedsComponent->SetNeedValue(
				Pair.Key,
				FMath::Min(100.0f, CurrentValue + RestorationPerSecond * DeltaTime)
			);
		}
	}

	// Check if done
	if (TimeSpentOnTask >= TaskDuration)
	{
		return false; // Action complete
	}

	return true; // Still working
}

void ULyraNPCGOAPAction_UseTask::OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess)
{
	if (CurrentTask.IsValid())
	{
		CurrentTask->StopUsing(NPC);
	}

	TimeSpentOnTask = 0.0f;
	CurrentTask = nullptr;
}
