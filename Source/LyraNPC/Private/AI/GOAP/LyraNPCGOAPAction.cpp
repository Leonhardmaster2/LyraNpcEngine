// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "LyraNPCModule.h"

ULyraNPCGOAPAction::ULyraNPCGOAPAction()
{
	ActionName = TEXT("Base Action");
}

bool ULyraNPCGOAPAction::IsValid_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return false;

	// Check range if required
	if (bRequiresTarget && MaxRange > 0.0f)
	{
		FVector Location;
		AActor* Actor;
		if (!const_cast<ULyraNPCGOAPAction*>(this)->FindTarget_Implementation(NPC, Location, Actor))
		{
			return false;
		}

		float Distance = FVector::Dist(NPC->GetActorLocation(), Location);
		if (Distance > MaxRange)
		{
			return false;
		}
	}

	return true;
}

bool ULyraNPCGOAPAction::CheckPreconditions_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	return CurrentState.MeetsConditions(Preconditions);
}

FLyraNPCWorldState ULyraNPCGOAPAction::ApplyEffects_Implementation(const FLyraNPCWorldState& CurrentState) const
{
	FLyraNPCWorldState NewState = CurrentState;
	NewState.ApplyState(Effects);
	return NewState;
}

float ULyraNPCGOAPAction::CalculateCost_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	float TotalCost = Cost.BaseCost;

	// Add distance cost if target required
	if (bRequiresTarget)
	{
		FVector Location;
		AActor* Actor;
		if (const_cast<ULyraNPCGOAPAction*>(this)->FindTarget_Implementation(NPC, Location, Actor))
		{
			float Distance = FVector::Dist(NPC->GetActorLocation(), Location);
			TotalCost += Distance * Cost.DistanceCostMultiplier;
		}
	}

	// Intelligence affects cost calculation (smarter NPCs evaluate costs more accurately)
	if (ULyraNPCCognitiveComponent* Cognitive = NPC->CognitiveComponent)
	{
		TotalCost = Cognitive->ApplyIntelligenceVariance(TotalCost);
	}

	return TotalCost;
}

bool ULyraNPCGOAPAction::FindTarget_Implementation(ALyraNPCCharacter* NPC, FVector& OutLocation, AActor*& OutActor)
{
	// Base implementation - override in derived classes
	OutLocation = NPC->GetActorLocation();
	OutActor = nullptr;
	return false;
}

void ULyraNPCGOAPAction::OnActionStart_Implementation(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor)
{
	bIsExecuting = true;
	ExecutionStartTime = GetWorldFromNPC(NPC)->GetTimeSeconds();
	CurrentNPC = NPC;

	UE_LOG(LogLyraNPC, Verbose, TEXT("GOAP Action Started: %s"), *GetDisplayName());
}

bool ULyraNPCGOAPAction::TickAction_Implementation(ALyraNPCCharacter* NPC, float DeltaTime)
{
	// Base implementation - override in derived classes
	return true;  // Action complete
}

void ULyraNPCGOAPAction::OnActionEnd_Implementation(ALyraNPCCharacter* NPC, bool bSuccess)
{
	bIsExecuting = false;
	CurrentNPC.Reset();

	UE_LOG(LogLyraNPC, Verbose, TEXT("GOAP Action Ended: %s (Success: %s)"),
		*GetDisplayName(), bSuccess ? TEXT("Yes") : TEXT("No"));
}

bool ULyraNPCGOAPAction::IsComplete_Implementation(ALyraNPCCharacter* NPC) const
{
	// Base implementation - override in derived classes
	return true;
}

void ULyraNPCGOAPAction::AbortAction_Implementation(ALyraNPCCharacter* NPC)
{
	if (bIsExecuting)
	{
		OnActionEnd(NPC, false);
	}
}

FLyraNPCWorldState ULyraNPCGOAPAction::GetProceduralEffects_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	// Default: return static effects
	return Effects;
}

UWorld* ULyraNPCGOAPAction::GetWorldFromNPC(ALyraNPCCharacter* NPC) const
{
	return NPC ? NPC->GetWorld() : nullptr;
}
