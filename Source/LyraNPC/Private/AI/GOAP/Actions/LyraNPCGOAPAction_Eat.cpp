// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/Actions/LyraNPCGOAPAction_Eat.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"

ULyraNPCGOAPAction_Eat::ULyraNPCGOAPAction_Eat()
{
	ActionName = FName("Eat");
	ActionDescription = FText::FromString("Find food and eat to satisfy hunger");

	// Preconditions: Need to be hungry and have access to food
	Preconditions.StateValues.Add(FName("IsHungry"), 1.0f);
	Preconditions.StateFlags.Add(FName("HasFood"), true);

	// Effects: No longer hungry
	Effects.StateValues.Add(FName("IsHungry"), 0.0f);
	Effects.StateFlags.Add(FName("HasEaten"), true);

	// Base cost
	Cost.BaseCost = 5.0f;
	Cost.bRequiresMovement = true;
}

bool ULyraNPCGOAPAction_Eat::CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return false;

	// Check if actually hungry
	ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent;
	if (!Needs) return false;

	float HungerValue = Needs->GetNeedValue(ELyraNPCNeedType::Hunger);
	if (HungerValue < 30.0f) // Only eat if hunger is low
	{
		return false;
	}

	// Check if food is available in the world
	UWorld* World = NPC->GetWorld();
	if (!World) return false;

	ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
	if (!WorldSubsystem) return false;

	// Look for food tasks
	TArray<ULyraNPCTaskActor*> AllTasks = WorldSubsystem->GetAllTasks();
	for (ULyraNPCTaskActor* Task : AllTasks)
	{
		if (Task && Task->TaskType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("LyraNPC.Task.Eat"))))
		{
			if (Task->CanBeUsedBy(NPC))
			{
				return true; // Food is available
			}
		}
	}

	return false;
}

FLyraNPCWorldState ULyraNPCGOAPAction_Eat::ApplyEffects(const FLyraNPCWorldState& CurrentState) const
{
	FLyraNPCWorldState NewState = CurrentState;
	NewState.ApplyState(Effects);
	return NewState;
}

float ULyraNPCGOAPAction_Eat::CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return 9999.0f;

	float TotalCost = Cost.BaseCost;

	// Find nearest food source
	UWorld* World = NPC->GetWorld();
	if (World)
	{
		ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
		if (WorldSubsystem)
		{
			ULyraNPCTaskActor* NearestFood = WorldSubsystem->FindBestTaskForNPC(
				NPC,
				FGameplayTag::RequestGameplayTag(FName("LyraNPC.Task.Eat"))
			);

			if (NearestFood)
			{
				float Distance = FVector::Dist(NPC->GetActorLocation(), NearestFood->GetActorLocation());
				TotalCost += (Distance / 100.0f) * Cost.DistanceCostMultiplier;
			}
			else
			{
				return 9999.0f; // No food available
			}
		}
	}

	return TotalCost;
}

void ULyraNPCGOAPAction_Eat::OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor)
{
	TimeSpentEating = 0.0f;
	FoodSource = Cast<ULyraNPCTaskActor>(TargetActor);

	if (FoodSource.IsValid())
	{
		FoodSource->StartUsing(NPC);
	}
}

bool ULyraNPCGOAPAction_Eat::TickAction(ALyraNPCCharacter* NPC, float DeltaTime)
{
	if (!NPC) return false;

	TimeSpentEating += DeltaTime;

	// Gradually restore hunger while eating
	if (ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent)
	{
		float RestorationPerSecond = HungerSatisfaction / EatingDuration;
		float CurrentHunger = Needs->GetNeedValue(ELyraNPCNeedType::Hunger);
		Needs->SetNeedValue(ELyraNPCNeedType::Hunger, FMath::Min(100.0f, CurrentHunger + RestorationPerSecond * DeltaTime));
	}

	// Check if done eating
	if (TimeSpentEating >= EatingDuration)
	{
		return false; // Action complete
	}

	return true; // Still eating
}

void ULyraNPCGOAPAction_Eat::OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess)
{
	if (FoodSource.IsValid())
	{
		FoodSource->StopUsing(NPC);
	}

	TimeSpentEating = 0.0f;
	FoodSource = nullptr;
}
