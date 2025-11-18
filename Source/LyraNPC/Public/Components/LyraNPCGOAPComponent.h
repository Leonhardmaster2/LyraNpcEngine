// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/GOAP/LyraNPCGOAPTypes.h"
#include "LyraNPCGOAPComponent.generated.h"

class ULyraNPCGOAPAction;
class ALyraNPCCharacter;

/**
 * Component that implements Goal-Oriented Action Planning for NPCs.
 * NPCs formulate multi-step plans to achieve their goals dynamically.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC GOAP Planner"))
class LYRANPC_API ULyraNPCGOAPComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPComponent();

	// ===== CONFIGURATION =====

	// Available actions this NPC can perform
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "GOAP|Actions")
	TArray<TObjectPtr<ULyraNPCGOAPAction>> AvailableActions;

	// Available goals this NPC can pursue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Goals")
	TArray<FLyraNPCGoal> AvailableGoals;

	// Maximum planning depth (prevents infinite loops)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Planning")
	int32 MaxPlanningDepth = 10;

	// Maximum nodes to explore during planning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Planning")
	int32 MaxNodesExplored = 1000;

	// How often to replan (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Planning")
	float ReplanInterval = 5.0f;

	// Allow replanning during execution if goal changes?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Planning")
	bool bAllowDynamicReplanning = true;

	// Modify planning based on cognitive skill
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Intelligence")
	bool bCognitiveSkillAffectsPlanning = true;

	// ===== STATE =====

	// Current active goal
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|State")
	FLyraNPCGoal CurrentGoal;

	// Current action plan
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|State")
	FLyraNPCActionPlan CurrentPlan;

	// Current world state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|State")
	FLyraNPCWorldState CurrentWorldState;

	// Currently executing action
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GOAP|State")
	TObjectPtr<ULyraNPCGOAPAction> CurrentAction;

	// ===== EVENTS =====

	UPROPERTY(BlueprintAssignable, Category = "GOAP|Events")
	FOnGOAPPlanCreated OnPlanCreated;

	UPROPERTY(BlueprintAssignable, Category = "GOAP|Events")
	FOnGOAPPlanFailed OnPlanFailed;

	UPROPERTY(BlueprintAssignable, Category = "GOAP|Events")
	FOnGOAPActionStarted OnActionStarted;

	UPROPERTY(BlueprintAssignable, Category = "GOAP|Events")
	FOnGOAPActionCompleted OnActionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "GOAP|Events")
	FOnGOAPGoalAchieved OnGoalAchieved;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== PLANNING =====

	/**
	 * Create a plan to achieve the given goal using A* pathfinding
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Planning")
	bool CreatePlan(const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan);

	/**
	 * Select the highest priority goal based on current state
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Planning")
	FLyraNPCGoal SelectBestGoal();

	/**
	 * Update goal priorities based on needs, schedule, etc.
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Planning")
	void UpdateGoalPriorities();

	/**
	 * Force replanning (discards current plan)
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Planning")
	void ForceReplan();

	// ===== EXECUTION =====

	/**
	 * Start executing the current plan
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Execution")
	bool StartPlanExecution();

	/**
	 * Stop execution and clear plan
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Execution")
	void StopExecution();

	/**
	 * Execute current action (called from tick)
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Execution")
	void ExecuteCurrentAction(float DeltaTime);

	// ===== WORLD STATE =====

	/**
	 * Update world state from NPC components
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|State")
	void UpdateWorldState();

	/**
	 * Get current world state value
	 */
	UFUNCTION(BlueprintPure, Category = "GOAP|State")
	float GetWorldStateValue(FName Key, float Default = 0.0f) const;

	/**
	 * Get current world state flag
	 */
	UFUNCTION(BlueprintPure, Category = "GOAP|State")
	bool GetWorldStateFlag(FName Key, bool bDefault = false) const;

	/**
	 * Set world state value manually
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|State")
	void SetWorldStateValue(FName Key, float Value);

	/**
	 * Set world state flag manually
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|State")
	void SetWorldStateFlag(FName Key, bool bValue);

	// ===== ACTIONS =====

	/**
	 * Add an action to the available actions list
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Actions")
	void AddAction(ULyraNPCGOAPAction* Action);

	/**
	 * Remove an action from available actions
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Actions")
	void RemoveAction(ULyraNPCGOAPAction* Action);

	/**
	 * Get all valid actions for current state
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Actions")
	TArray<ULyraNPCGOAPAction*> GetValidActions() const;

	// ===== GOALS =====

	/**
	 * Add a goal to pursue
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Goals")
	void AddGoal(const FLyraNPCGoal& Goal);

	/**
	 * Remove a goal
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Goals")
	void RemoveGoal(FGameplayTag GoalTag);

	/**
	 * Set goal active/inactive
	 */
	UFUNCTION(BlueprintCallable, Category = "GOAP|Goals")
	void SetGoalActive(FGameplayTag GoalTag, bool bActive);

	// ===== QUERIES =====

	UFUNCTION(BlueprintPure, Category = "GOAP|Query")
	bool IsExecutingPlan() const { return CurrentPlan.bIsExecuting; }

	UFUNCTION(BlueprintPure, Category = "GOAP|Query")
	bool HasActivePlan() const { return CurrentPlan.Steps.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "GOAP|Query")
	int32 GetPlanStepsRemaining() const;

	UFUNCTION(BlueprintPure, Category = "GOAP|Query")
	float GetPlanProgress() const;

protected:
	virtual void BeginPlay() override;

private:
	// A* planning algorithm
	bool PlanAStar(const FLyraNPCWorldState& StartState, const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan);

	// Heuristic for A* (estimates cost to goal)
	float CalculateHeuristic(const FLyraNPCWorldState& State, const FLyraNPCGoal& Goal) const;

	// Check if goal is achieved
	bool IsGoalAchieved(const FLyraNPCWorldState& State, const FLyraNPCGoal& Goal) const;

	// Get neighbors (possible next states) for A* search
	void GetNeighbors(const FLyraNPCWorldState& State, TArray<TPair<ULyraNPCGOAPAction*, FLyraNPCWorldState>>& OutNeighbors);

	// Reconstruct plan from A* nodes
	void ReconstructPlan(TSharedPtr<FLyraNPCGOAPNode> GoalNode, const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan);

	// Advance to next action in plan
	void AdvancePlan();

	// Check if plan is still valid
	bool IsPlanValid() const;

	// Timers
	float TimeSinceLastReplan = 0.0f;

	// Owner NPC
	UPROPERTY(Transient)
	TWeakObjectPtr<ALyraNPCCharacter> OwnerNPC;

	// Planning statistics
	int32 LastPlanNodesExplored = 0;
	float LastPlanTime = 0.0f;
};
