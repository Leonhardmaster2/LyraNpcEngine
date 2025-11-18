// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/Actions/LyraNPCGOAPAction_Sleep.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"

ULyraNPCGOAPAction_Sleep::ULyraNPCGOAPAction_Sleep()
{
	ActionName = FName("Sleep");
	ActionDescription = FText::FromString("Find a bed and sleep to restore energy");

	// Preconditions: Need to be tired and have access to bed
	Preconditions.StateValues.Add(FName("IsTired"), 1.0f);
	Preconditions.StateFlags.Add(FName("HasBed"), true);

	// Effects: No longer tired, rested
	Effects.StateValues.Add(FName("IsTired"), 0.0f);
	Effects.StateFlags.Add(FName("IsRested"), true);

	// Base cost
	Cost.BaseCost = 8.0f;
	Cost.bRequiresMovement = true;
}

bool ULyraNPCGOAPAction_Sleep::CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return false;

	// Check if actually tired
	ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent;
	if (!Needs) return false;

	float EnergyValue = Needs->GetNeedValue(ELyraNPCNeedType::Energy);
	if (EnergyValue < 30.0f) // Only sleep if energy is low
	{
		return false;
	}

	// Check if bed is available in the world
	UWorld* World = NPC->GetWorld();
	if (!World) return false;

	ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
	if (!WorldSubsystem) return false;

	// Look for sleep tasks
	TArray<ULyraNPCTaskActor*> AllTasks = WorldSubsystem->GetAllTasks();
	for (ULyraNPCTaskActor* Task : AllTasks)
	{
		if (Task && Task->TaskType.MatchesTag(FGameplayTag::RequestGameplayTag(FName("LyraNPC.Task.Sleep"))))
		{
			if (Task->CanBeUsedBy(NPC))
			{
				return true; // Bed is available
			}
		}
	}

	return false;
}

FLyraNPCWorldState ULyraNPCGOAPAction_Sleep::ApplyEffects(const FLyraNPCWorldState& CurrentState) const
{
	FLyraNPCWorldState NewState = CurrentState;
	NewState.ApplyState(Effects);
	return NewState;
}

float ULyraNPCGOAPAction_Sleep::CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return 9999.0f;

	float TotalCost = Cost.BaseCost;

	// Find nearest bed
	UWorld* World = NPC->GetWorld();
	if (World)
	{
		ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
		if (WorldSubsystem)
		{
			ULyraNPCTaskActor* NearestBed = WorldSubsystem->FindBestTaskForNPC(
				NPC,
				FGameplayTag::RequestGameplayTag(FName("LyraNPC.Task.Sleep"))
			);

			if (NearestBed)
			{
				float Distance = FVector::Dist(NPC->GetActorLocation(), NearestBed->GetActorLocation());
				TotalCost += (Distance / 100.0f) * Cost.DistanceCostMultiplier;
			}
			else
			{
				return 9999.0f; // No bed available
			}
		}
	}

	return TotalCost;
}

void ULyraNPCGOAPAction_Sleep::OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor)
{
	TimeSpentSleeping = 0.0f;
	Bed = Cast<ULyraNPCTaskActor>(TargetActor);

	if (Bed.IsValid())
	{
		Bed->StartUsing(NPC);
	}
}

bool ULyraNPCGOAPAction_Sleep::TickAction(ALyraNPCCharacter* NPC, float DeltaTime)
{
	if (!NPC) return false;

	TimeSpentSleeping += DeltaTime;

	// Gradually restore energy while sleeping
	if (ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent)
	{
		float RestorationPerSecond = EnergyRestoration / SleepDuration;
		float CurrentEnergy = Needs->GetNeedValue(ELyraNPCNeedType::Energy);
		Needs->SetNeedValue(ELyraNPCNeedType::Energy, FMath::Min(100.0f, CurrentEnergy + RestorationPerSecond * DeltaTime));
	}

	// Check if done sleeping
	if (TimeSpentSleeping >= SleepDuration)
	{
		return false; // Action complete
	}

	return true; // Still sleeping
}

void ULyraNPCGOAPAction_Sleep::OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess)
{
	if (Bed.IsValid())
	{
		Bed->StopUsing(NPC);
	}

	TimeSpentSleeping = 0.0f;
	Bed = nullptr;
}
