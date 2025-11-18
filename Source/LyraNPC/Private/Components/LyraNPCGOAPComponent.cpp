// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCGOAPComponent.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Components/LyraNPCScheduleComponent.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "LyraNPCModule.h"

ULyraNPCGOAPComponent::ULyraNPCGOAPComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void ULyraNPCGOAPComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerNPC = Cast<ALyraNPCCharacter>(GetOwner());
	UpdateWorldState();
	UpdateGoalPriorities();
}

void ULyraNPCGOAPComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!OwnerNPC.IsValid()) return;

	// Update world state
	UpdateWorldState();

	// Check if we need to replan
	TimeSinceLastReplan += DeltaTime;
	if (TimeSinceLastReplan >= ReplanInterval)
	{
		TimeSinceLastReplan = 0.0f;

		// If no plan or plan invalid, create new one
		if (!HasActivePlan() || !IsPlanValid())
		{
			UpdateGoalPriorities();
			FLyraNPCGoal BestGoal = SelectBestGoal();

			if (BestGoal.GoalTag.IsValid())
			{
				FLyraNPCActionPlan NewPlan;
				if (CreatePlan(BestGoal, NewPlan))
				{
					CurrentPlan = NewPlan;
					CurrentGoal = BestGoal;
					StartPlanExecution();
				}
			}
		}
	}

	// Execute current action
	if (IsExecutingPlan())
	{
		ExecuteCurrentAction(DeltaTime);
	}
}

bool ULyraNPCGOAPComponent::CreatePlan(const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan)
{
	if (!OwnerNPC.IsValid())
	{
		return false;
	}

	UE_LOG(LogLyraNPC, Log, TEXT("%s: Creating GOAP plan for goal '%s'"),
		*OwnerNPC->GetNPCName(), *Goal.GoalName);

	float StartTime = FPlatformTime::Seconds();

	// Use A* to find plan
	bool bSuccess = PlanAStar(CurrentWorldState, Goal, OutPlan);

	LastPlanTime = FPlatformTime::Seconds() - StartTime;

	if (bSuccess)
	{
		UE_LOG(LogLyraNPC, Log, TEXT("%s: Plan created with %d steps (cost: %.1f, time: %.3fs, nodes: %d)"),
			*OwnerNPC->GetNPCName(), OutPlan.Steps.Num(), OutPlan.TotalCost,
			LastPlanTime, LastPlanNodesExplored);

		OnPlanCreated.Broadcast(OwnerNPC.Get(), OutPlan);
	}
	else
	{
		UE_LOG(LogLyraNPC, Warning, TEXT("%s: Failed to create plan for goal '%s'"),
			*OwnerNPC->GetNPCName(), *Goal.GoalName);

		OnPlanFailed.Broadcast(OwnerNPC.Get(), Goal);
	}

	return bSuccess;
}

bool ULyraNPCGOAPComponent::PlanAStar(const FLyraNPCWorldState& StartState, const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan)
{
	// Check if goal already achieved
	if (IsGoalAchieved(StartState, Goal))
	{
		UE_LOG(LogLyraNPC, Verbose, TEXT("Goal already achieved"));
		return false;
	}

	// Initialize A* structures
	TArray<TSharedPtr<FLyraNPCGOAPNode>> OpenList;
	TArray<TSharedPtr<FLyraNPCGOAPNode>> ClosedList;

	// Create start node
	TSharedPtr<FLyraNPCGOAPNode> StartNode = MakeShared<FLyraNPCGOAPNode>();
	StartNode->State = StartState;
	StartNode->Parent = nullptr;
	StartNode->Action = nullptr;
	StartNode->GCost = 0.0f;
	StartNode->HCost = CalculateHeuristic(StartState, Goal);
	StartNode->FCost = StartNode->HCost;

	OpenList.Add(StartNode);
	LastPlanNodesExplored = 0;

	// A* main loop
	while (OpenList.Num() > 0 && LastPlanNodesExplored < MaxNodesExplored)
	{
		// Find node with lowest F cost
		int32 BestIndex = 0;
		float BestFCost = OpenList[0]->FCost;

		for (int32 i = 1; i < OpenList.Num(); ++i)
		{
			if (OpenList[i]->FCost < BestFCost)
			{
				BestIndex = i;
				BestFCost = OpenList[i]->FCost;
			}
		}

		// Get current node
		TSharedPtr<FLyraNPCGOAPNode> CurrentNode = OpenList[BestIndex];
		OpenList.RemoveAt(BestIndex);
		ClosedList.Add(CurrentNode);
		LastPlanNodesExplored++;

		// Check if goal achieved
		if (IsGoalAchieved(CurrentNode->State, Goal))
		{
			// Found solution!
			ReconstructPlan(CurrentNode, Goal, OutPlan);
			return true;
		}

		// Check depth limit (based on cognitive skill)
		int32 CurrentDepth = 0;
		TSharedPtr<FLyraNPCGOAPNode> DepthCheck = CurrentNode;
		while (DepthCheck->Parent.IsValid())
		{
			CurrentDepth++;
			DepthCheck = DepthCheck->Parent;
		}

		int32 ActualMaxDepth = MaxPlanningDepth;
		if (bCognitiveSkillAffectsPlanning && OwnerNPC.IsValid() && OwnerNPC->CognitiveComponent)
		{
			ActualMaxDepth = OwnerNPC->CognitiveComponent->GetPlanningDepth();
		}

		if (CurrentDepth >= ActualMaxDepth)
		{
			continue;  // Too deep
		}

		// Get neighbors (possible next states)
		TArray<TPair<ULyraNPCGOAPAction*, FLyraNPCWorldState>> Neighbors;
		GetNeighbors(CurrentNode->State, Neighbors);

		for (const auto& Neighbor : Neighbors)
		{
			ULyraNPCGOAPAction* Action = Neighbor.Key;
			FLyraNPCWorldState NextState = Neighbor.Value;

			// Check if state already in closed list
			bool bInClosed = false;
			for (const auto& ClosedNode : ClosedList)
			{
				if (ClosedNode->State.GetStateDifference(NextState) < 0.01f)
				{
					bInClosed = true;
					break;
				}
			}

			if (bInClosed) continue;

			// Calculate costs
			float ActionCost = Action->CalculateCost(OwnerNPC.Get(), CurrentNode->State);
			float NewGCost = CurrentNode->GCost + ActionCost;
			float NewHCost = CalculateHeuristic(NextState, Goal);
			float NewFCost = NewGCost + NewHCost;

			// Check if already in open list
			TSharedPtr<FLyraNPCGOAPNode> ExistingNode;
			for (const auto& OpenNode : OpenList)
			{
				if (OpenNode->State.GetStateDifference(NextState) < 0.01f)
				{
					ExistingNode = OpenNode;
					break;
				}
			}

			if (ExistingNode.IsValid())
			{
				// Update if this path is better
				if (NewGCost < ExistingNode->GCost)
				{
					ExistingNode->Parent = CurrentNode;
					ExistingNode->Action = Action;
					ExistingNode->GCost = NewGCost;
					ExistingNode->FCost = NewFCost;
				}
			}
			else
			{
				// Add new node to open list
				TSharedPtr<FLyraNPCGOAPNode> NewNode = MakeShared<FLyraNPCGOAPNode>(
					NextState, CurrentNode, Action, NewGCost, NewHCost
				);
				OpenList.Add(NewNode);
			}
		}
	}

	// No plan found
	UE_LOG(LogLyraNPC, Warning, TEXT("GOAP: No plan found after exploring %d nodes"), LastPlanNodesExplored);
	return false;
}

void ULyraNPCGOAPComponent::ReconstructPlan(TSharedPtr<FLyraNPCGOAPNode> GoalNode, const FLyraNPCGoal& Goal, FLyraNPCActionPlan& OutPlan)
{
	OutPlan.Goal = Goal;
	OutPlan.Steps.Empty();
	OutPlan.TotalCost = GoalNode->GCost;
	OutPlan.CurrentStepIndex = 0;
	OutPlan.CreationTime = GetWorld()->GetTimeSeconds();

	// Trace back from goal to start
	TArray<TSharedPtr<FLyraNPCGOAPNode>> Path;
	TSharedPtr<FLyraNPCGOAPNode> CurrentNode = GoalNode;

	while (CurrentNode->Parent.IsValid())
	{
		Path.Insert(CurrentNode, 0);
		CurrentNode = CurrentNode->Parent;
	}

	// Convert to plan steps
	for (const auto& Node : Path)
	{
		FLyraNPCPlanStep Step;
		Step.Action = Node->Action;
		Step.Cost = Node->GCost - (Node->Parent.IsValid() ? Node->Parent->GCost : 0.0f);

		// Find target for this action
		if (Node->Action && OwnerNPC.IsValid())
		{
			Node->Action->FindTarget(OwnerNPC.Get(), Step.TargetLocation, Step.TargetActor.Get());
		}

		OutPlan.Steps.Add(Step);
	}
}

float ULyraNPCGOAPComponent::CalculateHeuristic(const FLyraNPCWorldState& State, const FLyraNPCGoal& Goal) const
{
	// Simple heuristic: count differences between current state and goal state
	return State.GetStateDifference(Goal.DesiredState);
}

bool ULyraNPCGOAPComponent::IsGoalAchieved(const FLyraNPCWorldState& State, const FLyraNPCGoal& Goal) const
{
	return State.MeetsConditions(Goal.DesiredState);
}

void ULyraNPCGOAPComponent::GetNeighbors(const FLyraNPCWorldState& State, TArray<TPair<ULyraNPCGOAPAction*, FLyraNPCWorldState>>& OutNeighbors)
{
	if (!OwnerNPC.IsValid()) return;

	for (ULyraNPCGOAPAction* Action : AvailableActions)
	{
		if (!Action) continue;

		// Check if action is valid
		if (!Action->IsValid(OwnerNPC.Get(), State)) continue;

		// Check preconditions
		if (!Action->CheckPreconditions(OwnerNPC.Get(), State)) continue;

		// Apply effects
		FLyraNPCWorldState NextState = Action->ApplyEffects(State);

		OutNeighbors.Add(TPair<ULyraNPCGOAPAction*, FLyraNPCWorldState>(Action, NextState));
	}
}

FLyraNPCGoal ULyraNPCGOAPComponent::SelectBestGoal()
{
	UpdateGoalPriorities();

	FLyraNPCGoal BestGoal;
	float HighestPriority = -1.0f;

	for (FLyraNPCGoal& Goal : AvailableGoals)
	{
		if (!Goal.bIsActive) continue;

		if (Goal.CurrentPriority > HighestPriority)
		{
			HighestPriority = Goal.CurrentPriority;
			BestGoal = Goal;
		}
	}

	return BestGoal;
}

void ULyraNPCGOAPComponent::UpdateGoalPriorities()
{
	if (!OwnerNPC.IsValid()) return;

	for (FLyraNPCGoal& Goal : AvailableGoals)
	{
		float Priority = Goal.BasePriority;

		// Modify based on needs
		if (OwnerNPC->NeedsComponent)
		{
			// Check if goal relates to needs
			// This is game-specific logic - would be extended by game code
			Priority *= (1.0f + (100.0f - OwnerNPC->NeedsComponent->GetOverallWellbeing()) / 100.0f);
		}

		// Modify based on schedule
		if (OwnerNPC->ScheduleComponent)
		{
			// Check if goal matches current scheduled activity
			// This would be extended by game code
		}

		// Apply cognitive skill (smarter NPCs better at prioritizing)
		if (OwnerNPC->CognitiveComponent)
		{
			Priority *= OwnerNPC->CognitiveComponent->GetDecisionQuality();
		}

		Goal.CurrentPriority = Priority;
	}
}

bool ULyraNPCGOAPComponent::StartPlanExecution()
{
	if (!HasActivePlan() || !OwnerNPC.IsValid())
	{
		return false;
	}

	CurrentPlan.bIsExecuting = true;
	CurrentPlan.CurrentStepIndex = 0;

	UE_LOG(LogLyraNPC, Log, TEXT("%s: Starting plan execution (%d steps)"),
		*OwnerNPC->GetNPCName(), CurrentPlan.Steps.Num());

	return true;
}

void ULyraNPCGOAPComponent::StopExecution()
{
	if (CurrentAction)
	{
		CurrentAction->AbortAction(OwnerNPC.Get());
		CurrentAction = nullptr;
	}

	CurrentPlan.bIsExecuting = false;
}

void ULyraNPCGOAPComponent::ExecuteCurrentAction(float DeltaTime)
{
	if (!OwnerNPC.IsValid() || !HasActivePlan()) return;

	FLyraNPCPlanStep* CurrentStep = CurrentPlan.GetCurrentStep();
	if (!CurrentStep || !CurrentStep->Action)
	{
		// Plan complete
		UE_LOG(LogLyraNPC, Log, TEXT("%s: Plan execution complete"), *OwnerNPC->GetNPCName());
		OnGoalAchieved.Broadcast(OwnerNPC.Get(), CurrentGoal);
		StopExecution();
		return;
	}

	// Start action if not already started
	if (CurrentAction != CurrentStep->Action)
	{
		if (CurrentAction)
		{
			CurrentAction->OnActionEnd(OwnerNPC.Get(), true);
		}

		CurrentAction = CurrentStep->Action;
		CurrentAction->OnActionStart(OwnerNPC.Get(), CurrentStep->TargetLocation, CurrentStep->TargetActor.Get());

		OnActionStarted.Broadcast(OwnerNPC.Get(), CurrentAction, CurrentPlan.CurrentStepIndex);
	}

	// Tick action
	bool bContinue = CurrentAction->TickAction(OwnerNPC.Get(), DeltaTime);

	// Check if complete
	if (!bContinue || CurrentAction->IsComplete(OwnerNPC.Get()))
	{
		bool bSuccess = CurrentAction->IsComplete(OwnerNPC.Get());
		CurrentAction->OnActionEnd(OwnerNPC.Get(), bSuccess);

		OnActionCompleted.Broadcast(OwnerNPC.Get(), CurrentAction, bSuccess);

		if (bSuccess)
		{
			// Move to next action
			AdvancePlan();
		}
		else
		{
			// Action failed, replan
			UE_LOG(LogLyraNPC, Warning, TEXT("%s: Action failed, replanning"), *OwnerNPC->GetNPCName());
			ForceReplan();
		}

		CurrentAction = nullptr;
	}
}

void ULyraNPCGOAPComponent::AdvancePlan()
{
	CurrentPlan.AdvanceStep();
}

bool ULyraNPCGOAPComponent::IsPlanValid() const
{
	// Check if plan is still relevant
	// This is a simplified check - could be extended

	if (!HasActivePlan()) return false;

	// Check if goal priority changed drastically
	for (const FLyraNPCGoal& Goal : AvailableGoals)
	{
		if (Goal.GoalTag == CurrentGoal.GoalTag) continue;

		if (Goal.CurrentPriority > CurrentGoal.CurrentPriority * 2.0f)
		{
			return false;  // Different goal much more important now
		}
	}

	return true;
}

void ULyraNPCGOAPComponent::ForceReplan()
{
	StopExecution();
	CurrentPlan = FLyraNPCActionPlan();
	TimeSinceLastReplan = ReplanInterval;  // Force replan on next tick
}

void ULyraNPCGOAPComponent::UpdateWorldState()
{
	if (!OwnerNPC.IsValid()) return;

	// Update from needs
	if (OwnerNPC->NeedsComponent)
	{
		CurrentWorldState.SetValue(TEXT("Hunger"), OwnerNPC->NeedsComponent->GetNeedValue(ELyraNPCNeedType::Hunger));
		CurrentWorldState.SetValue(TEXT("Energy"), OwnerNPC->NeedsComponent->GetNeedValue(ELyraNPCNeedType::Energy));
		CurrentWorldState.SetValue(TEXT("Social"), OwnerNPC->NeedsComponent->GetNeedValue(ELyraNPCNeedType::Social));
		CurrentWorldState.SetValue(TEXT("Wellbeing"), OwnerNPC->NeedsComponent->GetOverallWellbeing());

		CurrentWorldState.SetFlag(TEXT("HasCriticalNeed"), OwnerNPC->NeedsComponent->HasCriticalNeed());
		CurrentWorldState.SetFlag(TEXT("HasUrgentNeed"), OwnerNPC->NeedsComponent->HasUrgentNeed());
	}

	// Update from schedule
	if (OwnerNPC->ScheduleComponent)
	{
		CurrentWorldState.SetValue(TEXT("GameHour"), OwnerNPC->ScheduleComponent->GetCurrentGameHour());
		CurrentWorldState.SetFlag(TEXT("IsNightTime"), OwnerNPC->ScheduleComponent->IsNightTime());
		CurrentWorldState.SetFlag(TEXT("ShouldBeWorking"), OwnerNPC->ScheduleComponent->ShouldBeWorking());
		CurrentWorldState.SetFlag(TEXT("ShouldBeSleeping"), OwnerNPC->ScheduleComponent->ShouldBeSleeping());
	}

	// Update from cognitive
	if (OwnerNPC->CognitiveComponent)
	{
		CurrentWorldState.SetFlag(TEXT("IsInCombat"), OwnerNPC->CognitiveComponent->IsInCombat());
		CurrentWorldState.SetFlag(TEXT("IsAlerted"), OwnerNPC->CognitiveComponent->IsAlerted());
	}

	// Update from character state
	CurrentWorldState.SetFlag(TEXT("IsAlive"), OwnerNPC->IsAlive());
	CurrentWorldState.SetValue(TEXT("Health"), OwnerNPC->CombatStats.CurrentHealth);
	CurrentWorldState.SetValue(TEXT("HealthPercent"), OwnerNPC->GetHealthPercent());
}

// Remaining simple implementations
float ULyraNPCGOAPComponent::GetWorldStateValue(FName Key, float Default) const
{
	return CurrentWorldState.GetValue(Key, Default);
}

bool ULyraNPCGOAPComponent::GetWorldStateFlag(FName Key, bool bDefault) const
{
	return CurrentWorldState.GetFlag(Key, bDefault);
}

void ULyraNPCGOAPComponent::SetWorldStateValue(FName Key, float Value)
{
	CurrentWorldState.SetValue(Key, Value);
}

void ULyraNPCGOAPComponent::SetWorldStateFlag(FName Key, bool bValue)
{
	CurrentWorldState.SetFlag(Key, bValue);
}

void ULyraNPCGOAPComponent::AddAction(ULyraNPCGOAPAction* Action)
{
	if (Action && !AvailableActions.Contains(Action))
	{
		AvailableActions.Add(Action);
	}
}

void ULyraNPCGOAPComponent::RemoveAction(ULyraNPCGOAPAction* Action)
{
	AvailableActions.Remove(Action);
}

TArray<ULyraNPCGOAPAction*> ULyraNPCGOAPComponent::GetValidActions() const
{
	TArray<ULyraNPCGOAPAction*> ValidActions;

	if (OwnerNPC.IsValid())
	{
		for (ULyraNPCGOAPAction* Action : AvailableActions)
		{
			if (Action && Action->IsValid(OwnerNPC.Get(), CurrentWorldState))
			{
				ValidActions.Add(Action);
			}
		}
	}

	return ValidActions;
}

void ULyraNPCGOAPComponent::AddGoal(const FLyraNPCGoal& Goal)
{
	AvailableGoals.Add(Goal);
}

void ULyraNPCGOAPComponent::RemoveGoal(FGameplayTag GoalTag)
{
	for (int32 i = AvailableGoals.Num() - 1; i >= 0; --i)
	{
		if (AvailableGoals[i].GoalTag == GoalTag)
		{
			AvailableGoals.RemoveAt(i);
		}
	}
}

void ULyraNPCGOAPComponent::SetGoalActive(FGameplayTag GoalTag, bool bActive)
{
	for (FLyraNPCGoal& Goal : AvailableGoals)
	{
		if (Goal.GoalTag == GoalTag)
		{
			Goal.bIsActive = bActive;
		}
	}
}

int32 ULyraNPCGOAPComponent::GetPlanStepsRemaining() const
{
	return CurrentPlan.Steps.Num() - CurrentPlan.CurrentStepIndex;
}

float ULyraNPCGOAPComponent::GetPlanProgress() const
{
	if (CurrentPlan.Steps.Num() == 0) return 0.0f;
	return static_cast<float>(CurrentPlan.CurrentStepIndex) / static_cast<float>(CurrentPlan.Steps.Num());
}
