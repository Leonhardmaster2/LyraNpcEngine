// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCWorldSubsystem.generated.h"

class ALyraNPCCharacter;
class ULyraNPCTaskActor;

/**
 * World subsystem that manages all LyraNPC characters globally.
 * Provides optimized queries, task pooling, and global time management.
 */
UCLASS()
class LYRANPC_API ULyraNPCWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULyraNPCWorldSubsystem();

	// ===== LIFECYCLE =====

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

	// ===== GLOBAL TIME =====

	// Current game hour (shared across all NPCs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Time")
	float GlobalGameHour = 6.0f;

	// Time scale (1 real second = TimeScale game seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Time")
	float GlobalTimeScale = 24.0f;

	// Whether to automatically advance time
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Time")
	bool bAutoAdvanceTime = true;

	// ===== NPC MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Management")
	void RegisterNPC(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Management")
	void UnregisterNPC(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	int32 GetTotalNPCCount() const { return RegisteredNPCs.Num(); }

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	TArray<ALyraNPCCharacter*> GetAllNPCs() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	TArray<ALyraNPCCharacter*> GetNPCsByArchetype(ELyraNPCArchetype Archetype) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	TArray<ALyraNPCCharacter*> GetNPCsInRadius(FVector Location, float Radius) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	ALyraNPCCharacter* FindNPCById(const FGuid& NPCId) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Management")
	ALyraNPCCharacter* FindNPCByName(const FString& Name) const;

	// ===== TASK MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Tasks")
	void RegisterTaskActor(ULyraNPCTaskActor* Task);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Tasks")
	void UnregisterTaskActor(ULyraNPCTaskActor* Task);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	TArray<ULyraNPCTaskActor*> GetAllTasks() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	TArray<ULyraNPCTaskActor*> GetTasksByType(FGameplayTag TaskType) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	TArray<ULyraNPCTaskActor*> GetAvailableTasksForNPC(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	ULyraNPCTaskActor* FindBestTaskForNPC(ALyraNPCCharacter* NPC, FGameplayTag TaskType = FGameplayTag()) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks")
	TArray<ULyraNPCTaskActor*> GetTasksInRadius(FVector Location, float Radius) const;

	// ===== GLOBAL TIME CONTROL =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Time")
	void SetGlobalGameHour(float NewHour);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Time")
	void AdvanceGlobalTime(float Hours);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Time")
	void SetTimeScale(float NewScale);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Time")
	bool IsGlobalNightTime() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Time")
	bool IsGlobalDayTime() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Time")
	FString GetTimeString() const;

	// ===== STATISTICS =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats")
	int32 GetNPCCountByLOD(ELyraNPCAILOD LOD) const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats")
	float GetAverageNPCWellbeing() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats")
	int32 GetNPCsInCombatCount() const;

	// ===== BATCH OPERATIONS =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Batch")
	void SetAllNPCsTimeScale(float NewTimeScale);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Batch")
	void PauseAllNPCs();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Batch")
	void ResumeAllNPCs();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Batch")
	void SyncAllNPCSchedulesToGlobalTime();

protected:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

private:
	UPROPERTY()
	TArray<TWeakObjectPtr<ALyraNPCCharacter>> RegisteredNPCs;

	UPROPERTY()
	TArray<TWeakObjectPtr<ULyraNPCTaskActor>> RegisteredTasks;

	void UpdateGlobalTime(float DeltaTime);
	void CleanupInvalidReferences();

	float TimeSinceLastCleanup = 0.0f;
	float CleanupInterval = 10.0f;
};
