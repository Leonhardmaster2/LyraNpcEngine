// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCCognitiveComponent.generated.h"

/**
 * Component that manages NPC intelligence, decision-making quality, and cognitive abilities.
 * The CognitiveSkill factor affects perception, movement, combat, and all decision-making.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Cognitive"))
class LYRANPC_API ULyraNPCCognitiveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCCognitiveComponent();

	// Core intelligence value (0 = very dumb, 1 = genius)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Core", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CognitiveSkill = 0.5f;

	// Current alert level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|State", Replicated)
	ELyraNPCAlertLevel AlertLevel = ELyraNPCAlertLevel::Unaware;

	// How quickly alertness decays back to normal (per second)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Alertness")
	float AlertnessDecayRate = 0.1f;

	// Memory capacity (affected by intelligence)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Memory")
	int32 MaxMemories = 50;

	// How fast memories decay (per hour, higher = faster decay)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Memory")
	float MemoryDecayRate = 2.0f;

	// Stored memories
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cognitive|Memory")
	TArray<FLyraNPCMemory> Memories;

	// Random decision variance (how much randomness in decisions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cognitive|Decisions", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DecisionVariance = 0.2f;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Cognitive|Events")
	FOnNPCAlertLevelChanged OnAlertLevelChanged;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== INTELLIGENCE MODIFIERS =====

	// Returns a modified perception radius based on intelligence
	UFUNCTION(BlueprintPure, Category = "Cognitive|Perception")
	float GetPerceptionRadiusModifier() const;

	// Returns sight angle modifier (smart NPCs have better peripheral vision)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Perception")
	float GetSightAngleModifier() const;

	// Returns hearing sensitivity modifier
	UFUNCTION(BlueprintPure, Category = "Cognitive|Perception")
	float GetHearingModifier() const;

	// Chance to notice something (affected by intelligence and alertness)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Perception")
	float GetNoticeChance(float BaseDifficulty = 0.5f) const;

	// ===== MOVEMENT MODIFIERS =====

	// Returns path following accuracy (1.0 = perfect, lower = more wobble)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Movement")
	float GetPathAccuracy() const;

	// Returns obstacle avoidance quality
	UFUNCTION(BlueprintPure, Category = "Cognitive|Movement")
	float GetObstacleAvoidanceQuality() const;

	// Gets movement speed modifier when panicked/stressed
	UFUNCTION(BlueprintPure, Category = "Cognitive|Movement")
	float GetStressedMovementModifier() const;

	// ===== COMBAT MODIFIERS =====

	// Returns combat accuracy modifier
	UFUNCTION(BlueprintPure, Category = "Cognitive|Combat")
	float GetCombatAccuracyModifier() const;

	// Returns reaction time modifier (lower is faster)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Combat")
	float GetReactionTimeModifier() const;

	// Returns tactical decision quality
	UFUNCTION(BlueprintPure, Category = "Cognitive|Combat")
	float GetTacticalQuality() const;

	// Can this NPC use flanking tactics?
	UFUNCTION(BlueprintPure, Category = "Cognitive|Combat")
	bool CanUseFlankingTactics() const;

	// Can this NPC coordinate with allies?
	UFUNCTION(BlueprintPure, Category = "Cognitive|Combat")
	bool CanCoordinateWithAllies() const;

	// ===== DECISION MAKING =====

	// Evaluates if NPC will remember to do something (can forget based on intelligence)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Decisions")
	bool WillRememberTask(float TaskImportance = 0.5f) const;

	// Gets the quality of a decision (1.0 = optimal, lower = suboptimal)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Decisions")
	float GetDecisionQuality() const;

	// Applies intelligence variance to a score (for utility AI)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Decisions")
	float ApplyIntelligenceVariance(float BaseScore) const;

	// Can the NPC plan multiple steps ahead?
	UFUNCTION(BlueprintPure, Category = "Cognitive|Decisions")
	int32 GetPlanningDepth() const;

	// ===== ALERTNESS MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "Cognitive|Alertness")
	void SetAlertLevel(ELyraNPCAlertLevel NewLevel);

	UFUNCTION(BlueprintCallable, Category = "Cognitive|Alertness")
	void IncreaseAlertness(float Amount = 0.25f);

	UFUNCTION(BlueprintPure, Category = "Cognitive|Alertness")
	bool IsAlerted() const { return AlertLevel >= ELyraNPCAlertLevel::Alert; }

	UFUNCTION(BlueprintPure, Category = "Cognitive|Alertness")
	bool IsInCombat() const { return AlertLevel == ELyraNPCAlertLevel::Combat; }

	// ===== MEMORY SYSTEM =====

	UFUNCTION(BlueprintCallable, Category = "Cognitive|Memory")
	void AddMemory(const FLyraNPCMemory& NewMemory);

	UFUNCTION(BlueprintCallable, Category = "Cognitive|Memory")
	void AddSimpleMemory(FGameplayTag MemoryType, const FString& Description, FVector Location, float Importance = 50.0f);

	UFUNCTION(BlueprintPure, Category = "Cognitive|Memory")
	bool HasMemoryOfType(FGameplayTag MemoryType) const;

	UFUNCTION(BlueprintPure, Category = "Cognitive|Memory")
	FLyraNPCMemory GetMostRecentMemory(FGameplayTag MemoryType) const;

	UFUNCTION(BlueprintPure, Category = "Cognitive|Memory")
	TArray<FLyraNPCMemory> GetMemoriesNearLocation(FVector Location, float Radius = 1000.0f) const;

	UFUNCTION(BlueprintCallable, Category = "Cognitive|Memory")
	void ForgetOldMemories();

	// ===== MISTAKE SYSTEM =====

	// Will the NPC make a mistake in this action? (based on intelligence and stress)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Mistakes")
	bool WillMakeMistake(float ActionDifficulty = 0.5f) const;

	// Gets the magnitude of a potential mistake (0 = no mistake, 1 = complete failure)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Mistakes")
	float GetMistakeMagnitude() const;

	// Applies a random error to a location (for inaccurate movement/aiming)
	UFUNCTION(BlueprintPure, Category = "Cognitive|Mistakes")
	FVector ApplyLocationError(FVector TargetLocation, float MaxErrorDistance = 100.0f) const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	float CurrentAlertness = 0.0f;
	float TimeSinceLastAlertChange = 0.0f;

	void UpdateAlertness(float DeltaTime);
	void UpdateMemoryDecay(float DeltaTime);
	void CleanupMemories();
};
