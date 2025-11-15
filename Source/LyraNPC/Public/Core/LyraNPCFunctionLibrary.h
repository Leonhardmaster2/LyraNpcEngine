// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCFunctionLibrary.generated.h"

class ALyraNPCCharacter;
class ULyraNPCTaskActor;
class ULyraNPCWorldSubsystem;

/**
 * Blueprint Function Library for common LyraNPC operations.
 * Provides easy access to NPC management, queries, and utilities.
 */
UCLASS()
class LYRANPC_API ULyraNPCFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ===== WORLD SUBSYSTEM ACCESS =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|World", meta = (WorldContext = "WorldContextObject"))
	static ULyraNPCWorldSubsystem* GetNPCWorldSubsystem(UObject* WorldContextObject);

	// ===== NPC QUERIES =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<ALyraNPCCharacter*> GetAllNPCs(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<ALyraNPCCharacter*> GetNPCsInRadius(UObject* WorldContextObject, FVector Location, float Radius);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Query", meta = (WorldContext = "WorldContextObject"))
	static ALyraNPCCharacter* GetClosestNPC(UObject* WorldContextObject, FVector Location);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Query", meta = (WorldContext = "WorldContextObject"))
	static TArray<ALyraNPCCharacter*> GetNPCsByArchetype(UObject* WorldContextObject, ELyraNPCArchetype Archetype);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Query", meta = (WorldContext = "WorldContextObject"))
	static int32 GetTotalNPCCount(UObject* WorldContextObject);

	// ===== TIME MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Time", meta = (WorldContext = "WorldContextObject"))
	static void SetGlobalGameTime(UObject* WorldContextObject, float Hour);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Time", meta = (WorldContext = "WorldContextObject"))
	static float GetGlobalGameTime(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Time", meta = (WorldContext = "WorldContextObject"))
	static FString GetGlobalTimeString(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Time", meta = (WorldContext = "WorldContextObject"))
	static void SetGlobalTimeScale(UObject* WorldContextObject, float TimeScale);

	// ===== TASK MANAGEMENT =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks", meta = (WorldContext = "WorldContextObject"))
	static TArray<ULyraNPCTaskActor*> GetAllTasks(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Tasks", meta = (WorldContext = "WorldContextObject"))
	static ULyraNPCTaskActor* FindBestTaskForNPC(UObject* WorldContextObject, ALyraNPCCharacter* NPC, FGameplayTag TaskType);

	// ===== NPC SPAWNING =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Spawn", meta = (WorldContext = "WorldContextObject"))
	static ALyraNPCCharacter* SpawnNPC(UObject* WorldContextObject, TSubclassOf<ALyraNPCCharacter> NPCClass, FVector Location, FRotator Rotation, ELyraNPCArchetype Archetype = ELyraNPCArchetype::Villager, float CognitiveSkill = 0.5f);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Spawn", meta = (WorldContext = "WorldContextObject"))
	static ALyraNPCCharacter* SpawnRandomNPC(UObject* WorldContextObject, TSubclassOf<ALyraNPCCharacter> NPCClass, FVector Location, FRotator Rotation);

	// ===== UTILITY =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static FString GetArchetypeName(ELyraNPCArchetype Archetype);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static FString GetLifeStateName(ELyraNPCLifeState State);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static FString GetAlertLevelName(ELyraNPCAlertLevel AlertLevel);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static FString GetNeedTypeName(ELyraNPCNeedType NeedType);

	// Generate random personality
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static FLyraNPCPersonality GenerateRandomPersonality();

	// Get personality compatibility score between two NPCs
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Utility")
	static float GetPersonalityCompatibility(const FLyraNPCPersonality& PersonalityA, const FLyraNPCPersonality& PersonalityB);

	// ===== STATISTICS =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats", meta = (WorldContext = "WorldContextObject"))
	static float GetAverageNPCWellbeing(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats", meta = (WorldContext = "WorldContextObject"))
	static int32 GetNPCsInCombatCount(UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Stats", meta = (WorldContext = "WorldContextObject"))
	static int32 GetNPCCountByAILOD(UObject* WorldContextObject, ELyraNPCAILOD LOD);
};
