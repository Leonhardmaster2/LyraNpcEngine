// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LyraNPCGOAPTypes.generated.h"

class ALyraNPCCharacter;
class ULyraNPCGOAPAction;

/**
 * World state key-value pair for GOAP planning
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCWorldState
{
	GENERATED_BODY()

	// State as key-value pairs (e.g., "HasFood" -> true, "Gold" -> 50)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, float> StateValues;

	// State as boolean flags
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, bool> StateFlags;

	void SetValue(FName Key, float Value) { StateValues.Add(Key, Value); }
	void SetFlag(FName Key, bool bValue) { StateFlags.Add(Key, bValue); }

	float GetValue(FName Key, float Default = 0.0f) const
	{
		const float* Found = StateValues.Find(Key);
		return Found ? *Found : Default;
	}

	bool GetFlag(FName Key, bool bDefault = false) const
	{
		const bool* Found = StateFlags.Find(Key);
		return Found ? *Found : bDefault;
	}

	bool MeetsConditions(const FLyraNPCWorldState& Conditions) const;
	void ApplyState(const FLyraNPCWorldState& Other);
	float GetStateDifference(const FLyraNPCWorldState& Other) const;
};

/**
 * A goal that an NPC wants to achieve
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCGoal
{
	GENERATED_BODY()

	// Unique identifier for this goal type
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag GoalTag;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GoalName;

	// Desired world state to achieve
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLyraNPCWorldState DesiredState;

	// Base priority (modified by needs/personality)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BasePriority = 1.0f;

	// Current calculated priority
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CurrentPriority = 1.0f;

	// Is this goal currently active?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsActive = false;

	// Timestamp when goal was started
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float StartTime = 0.0f;

	// Maximum time to spend on this goal before re-evaluating
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 600.0f;
};

/**
 * Procedural cost for GOAP actions
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCActionCost
{
	GENERATED_BODY()

	// Base cost of performing action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseCost = 1.0f;

	// Additional cost based on distance
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceCostMultiplier = 0.001f;

	// Additional cost based on needs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float NeedPenalty = 0.0f;

	// Additional cost based on danger
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DangerPenalty = 0.0f;

	float CalculateTotalCost(float Distance = 0.0f) const
	{
		return BaseCost + (Distance * DistanceCostMultiplier) + NeedPenalty + DangerPenalty;
	}
};

/**
 * A step in a GOAP plan
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCPlanStep
{
	GENERATED_BODY()

	// The action to perform
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<ULyraNPCGOAPAction> Action;

	// Cost of this step
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float Cost = 0.0f;

	// Target location (if applicable)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FVector TargetLocation = FVector::ZeroVector;

	// Target actor (if applicable)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TWeakObjectPtr<AActor> TargetActor;
};

/**
 * A complete action plan to achieve a goal
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCActionPlan
{
	GENERATED_BODY()

	// The goal this plan achieves
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FLyraNPCGoal Goal;

	// Steps to execute
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLyraNPCPlanStep> Steps;

	// Total cost of the plan
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float TotalCost = 0.0f;

	// Current step index
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 CurrentStepIndex = 0;

	// Is plan currently executing?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsExecuting = false;

	// Plan creation time
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	float CreationTime = 0.0f;

	bool IsComplete() const { return CurrentStepIndex >= Steps.Num(); }
	FLyraNPCPlanStep* GetCurrentStep() { return Steps.IsValidIndex(CurrentStepIndex) ? &Steps[CurrentStepIndex] : nullptr; }
	void AdvanceStep() { CurrentStepIndex++; }
};

/**
 * Node for A* pathfinding in GOAP planning
 */
struct FLyraNPCGOAPNode
{
	FLyraNPCWorldState State;
	TSharedPtr<FLyraNPCGOAPNode> Parent;
	ULyraNPCGOAPAction* Action;
	float GCost;  // Cost from start
	float HCost;  // Heuristic cost to goal
	float FCost;  // Total cost (G + H)

	FLyraNPCGOAPNode() : Action(nullptr), GCost(0.0f), HCost(0.0f), FCost(0.0f) {}

	FLyraNPCGOAPNode(const FLyraNPCWorldState& InState, TSharedPtr<FLyraNPCGOAPNode> InParent, ULyraNPCGOAPAction* InAction, float InGCost, float InHCost)
		: State(InState)
		, Parent(InParent)
		, Action(InAction)
		, GCost(InGCost)
		, HCost(InHCost)
		, FCost(InGCost + InHCost)
	{}
};

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGOAPPlanCreated, ALyraNPCCharacter*, NPC, const FLyraNPCActionPlan&, Plan);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGOAPPlanFailed, ALyraNPCCharacter*, NPC, const FLyraNPCGoal&, Goal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGOAPActionStarted, ALyraNPCCharacter*, NPC, ULyraNPCGOAPAction*, Action, int32, StepIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnGOAPActionCompleted, ALyraNPCCharacter*, NPC, ULyraNPCGOAPAction*, Action, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGOAPGoalAchieved, ALyraNPCCharacter*, NPC, const FLyraNPCGoal&, Goal);
