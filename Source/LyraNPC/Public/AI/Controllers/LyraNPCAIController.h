// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Core/LyraNPCTypes.h"
#include "Perception/AIPerceptionTypes.h"
#include "LyraNPCAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;
class ULyraNPCIdentityComponent;
class ULyraNPCCognitiveComponent;
class ULyraNPCNeedsComponent;
class ULyraNPCScheduleComponent;
class ULyraNPCTaskActor;

/**
 * Main AI Controller for LyraNPC characters.
 * This controller automatically discovers and uses all LyraNPC components.
 * Just add this controller to your NPC and it works out of the box.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(DisplayName="LyraNPC AI Controller"))
class LYRANPC_API ALyraNPCAIController : public AAIController
{
	GENERATED_BODY()

public:
	ALyraNPCAIController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ===== CORE COMPONENTS =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<UBlackboardComponent> BlackboardComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<UAIPerceptionComponent> AIPerceptionComponent;

	// ===== CONFIGURATION =====

	// Default Behavior Tree to run (can be overridden per NPC)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

	// Current AI LOD level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|AI", Replicated)
	ELyraNPCAILOD CurrentAILOD = ELyraNPCAILOD::Full;

	// Update rate based on LOD (seconds between ticks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Performance")
	TMap<ELyraNPCAILOD, float> LODUpdateRates;

	// Distance thresholds for LOD switching
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Performance")
	float FullLODDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Performance")
	float ReducedLODDistance = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Performance")
	float MinimalLODDistance = 10000.0f;

	// ===== CACHED COMPONENTS =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCIdentityComponent> IdentityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCCognitiveComponent> CognitiveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCNeedsComponent> NeedsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCScheduleComponent> ScheduleComponent;

	// ===== CURRENT STATE =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|State")
	TWeakObjectPtr<ULyraNPCTaskActor> CurrentTask;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|State")
	float CurrentTaskRemainingTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|State")
	TArray<FLyraNPCPerceptionResult> PerceivedThreats;

	// ===== EVENTS =====

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Events")
	FOnNPCAILODChanged OnAILODChanged;

public:
	// ===== LIFECYCLE =====

	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void Tick(float DeltaTime) override;

	// ===== AI LOD MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|LOD")
	void UpdateAILOD();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|LOD")
	void SetAILOD(ELyraNPCAILOD NewLOD);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|LOD")
	float GetDistanceToNearestPlayer() const;

	// ===== BEHAVIOR TREE MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|BehaviorTree")
	bool StartBehaviorTree(UBehaviorTree* TreeToRun);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|BehaviorTree")
	void StopBehaviorTree();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|BehaviorTree")
	void PauseBehaviorTree();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|BehaviorTree")
	void ResumeBehaviorTree();

	// ===== BLACKBOARD HELPERS =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Blackboard")
	void UpdateBlackboardFromComponents();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Blackboard")
	void SetBlackboardTask(ULyraNPCTaskActor* Task);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Blackboard")
	void ClearBlackboardTask();

	// ===== TASK MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Tasks")
	ULyraNPCTaskActor* FindBestTask(FGameplayTag TaskType = FGameplayTag());

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Tasks")
	bool StartUsingTask(ULyraNPCTaskActor* Task);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Tasks")
	void StopUsingCurrentTask();

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	bool IsUsingTask() const { return CurrentTask.IsValid(); }

	// ===== UTILITY AI SCORING =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Utility")
	float CalculateActionScore(FGameplayTag ActionType) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	FGameplayTag GetBestAction() const;

	// ===== PERCEPTION =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Perception")
	bool HasThreats() const { return PerceivedThreats.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Perception")
	AActor* GetHighestPriorityThreat() const;

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Perception")
	void UpdatePerception();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Perception event handlers
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	void CacheComponents();
	void SetupPerception();
	void UpdateTaskTimer(float DeltaTime);
	void ApplyLODSettings();

	float TimeSinceLastLODCheck = 0.0f;
	float LODCheckInterval = 1.0f;
};
