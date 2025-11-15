// Copyright LyraNPC Framework. All Rights Reserved.

#include "Core/LyraNPCFunctionLibrary.h"
#include "Core/LyraNPCCharacter.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"
#include "Engine/World.h"

ULyraNPCWorldSubsystem* ULyraNPCFunctionLibrary::GetNPCWorldSubsystem(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	return World->GetSubsystem<ULyraNPCWorldSubsystem>();
}

TArray<ALyraNPCCharacter*> ULyraNPCFunctionLibrary::GetAllNPCs(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetAllNPCs() : TArray<ALyraNPCCharacter*>();
}

TArray<ALyraNPCCharacter*> ULyraNPCFunctionLibrary::GetNPCsInRadius(UObject* WorldContextObject, FVector Location, float Radius)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetNPCsInRadius(Location, Radius) : TArray<ALyraNPCCharacter*>();
}

ALyraNPCCharacter* ULyraNPCFunctionLibrary::GetClosestNPC(UObject* WorldContextObject, FVector Location)
{
	TArray<ALyraNPCCharacter*> AllNPCs = GetAllNPCs(WorldContextObject);

	ALyraNPCCharacter* Closest = nullptr;
	float ClosestDist = MAX_FLT;

	for (ALyraNPCCharacter* NPC : AllNPCs)
	{
		float Dist = FVector::Dist(NPC->GetActorLocation(), Location);
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			Closest = NPC;
		}
	}

	return Closest;
}

TArray<ALyraNPCCharacter*> ULyraNPCFunctionLibrary::GetNPCsByArchetype(UObject* WorldContextObject, ELyraNPCArchetype Archetype)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetNPCsByArchetype(Archetype) : TArray<ALyraNPCCharacter*>();
}

int32 ULyraNPCFunctionLibrary::GetTotalNPCCount(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetTotalNPCCount() : 0;
}

void ULyraNPCFunctionLibrary::SetGlobalGameTime(UObject* WorldContextObject, float Hour)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	if (Subsystem)
	{
		Subsystem->SetGlobalGameHour(Hour);
	}
}

float ULyraNPCFunctionLibrary::GetGlobalGameTime(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GlobalGameHour : 0.0f;
}

FString ULyraNPCFunctionLibrary::GetGlobalTimeString(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetTimeString() : TEXT("00:00");
}

void ULyraNPCFunctionLibrary::SetGlobalTimeScale(UObject* WorldContextObject, float TimeScale)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	if (Subsystem)
	{
		Subsystem->SetTimeScale(TimeScale);
	}
}

TArray<ULyraNPCTaskActor*> ULyraNPCFunctionLibrary::GetAllTasks(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetAllTasks() : TArray<ULyraNPCTaskActor*>();
}

ULyraNPCTaskActor* ULyraNPCFunctionLibrary::FindBestTaskForNPC(UObject* WorldContextObject, ALyraNPCCharacter* NPC, FGameplayTag TaskType)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->FindBestTaskForNPC(NPC, TaskType) : nullptr;
}

ALyraNPCCharacter* ULyraNPCFunctionLibrary::SpawnNPC(UObject* WorldContextObject, TSubclassOf<ALyraNPCCharacter> NPCClass, FVector Location, FRotator Rotation, ELyraNPCArchetype Archetype, float CognitiveSkill)
{
	if (!WorldContextObject || !NPCClass) return nullptr;

	UWorld* World = WorldContextObject->GetWorld();
	if (!World) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ALyraNPCCharacter* NewNPC = World->SpawnActor<ALyraNPCCharacter>(NPCClass, Location, Rotation, SpawnParams);
	if (NewNPC)
	{
		NewNPC->InitialArchetype = Archetype;
		NewNPC->InitialCognitiveSkill = CognitiveSkill;
		NewNPC->InitializeNPC();

		// Register with world subsystem
		ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
		if (Subsystem)
		{
			Subsystem->RegisterNPC(NewNPC);
		}
	}

	return NewNPC;
}

ALyraNPCCharacter* ULyraNPCFunctionLibrary::SpawnRandomNPC(UObject* WorldContextObject, TSubclassOf<ALyraNPCCharacter> NPCClass, FVector Location, FRotator Rotation)
{
	TArray<ELyraNPCArchetype> Archetypes = {
		ELyraNPCArchetype::Villager,
		ELyraNPCArchetype::Worker,
		ELyraNPCArchetype::Guard,
		ELyraNPCArchetype::Merchant,
		ELyraNPCArchetype::Traveler
	};

	ELyraNPCArchetype RandomArchetype = Archetypes[FMath::RandRange(0, Archetypes.Num() - 1)];
	float RandomCognitiveSkill = FMath::FRandRange(0.2f, 0.9f);

	return SpawnNPC(WorldContextObject, NPCClass, Location, Rotation, RandomArchetype, RandomCognitiveSkill);
}

FString ULyraNPCFunctionLibrary::GetArchetypeName(ELyraNPCArchetype Archetype)
{
	switch (Archetype)
	{
	case ELyraNPCArchetype::Villager: return TEXT("Villager");
	case ELyraNPCArchetype::Guard: return TEXT("Guard");
	case ELyraNPCArchetype::Worker: return TEXT("Worker");
	case ELyraNPCArchetype::Merchant: return TEXT("Merchant");
	case ELyraNPCArchetype::Traveler: return TEXT("Traveler");
	case ELyraNPCArchetype::Enemy: return TEXT("Enemy");
	case ELyraNPCArchetype::Neutral: return TEXT("Neutral");
	case ELyraNPCArchetype::Companion: return TEXT("Companion");
	case ELyraNPCArchetype::Custom: return TEXT("Custom");
	default: return TEXT("Unknown");
	}
}

FString ULyraNPCFunctionLibrary::GetLifeStateName(ELyraNPCLifeState State)
{
	switch (State)
	{
	case ELyraNPCLifeState::Idle: return TEXT("Idle");
	case ELyraNPCLifeState::Working: return TEXT("Working");
	case ELyraNPCLifeState::Resting: return TEXT("Resting");
	case ELyraNPCLifeState::Eating: return TEXT("Eating");
	case ELyraNPCLifeState::Socializing: return TEXT("Socializing");
	case ELyraNPCLifeState::Traveling: return TEXT("Traveling");
	case ELyraNPCLifeState::Patrolling: return TEXT("Patrolling");
	case ELyraNPCLifeState::Combat: return TEXT("Combat");
	case ELyraNPCLifeState::Fleeing: return TEXT("Fleeing");
	case ELyraNPCLifeState::Investigating: return TEXT("Investigating");
	case ELyraNPCLifeState::UsingTask: return TEXT("Using Task");
	case ELyraNPCLifeState::Dead: return TEXT("Dead");
	default: return TEXT("Unknown");
	}
}

FString ULyraNPCFunctionLibrary::GetAlertLevelName(ELyraNPCAlertLevel AlertLevel)
{
	switch (AlertLevel)
	{
	case ELyraNPCAlertLevel::Unaware: return TEXT("Unaware");
	case ELyraNPCAlertLevel::Curious: return TEXT("Curious");
	case ELyraNPCAlertLevel::Suspicious: return TEXT("Suspicious");
	case ELyraNPCAlertLevel::Alert: return TEXT("Alert");
	case ELyraNPCAlertLevel::Combat: return TEXT("Combat");
	default: return TEXT("Unknown");
	}
}

FString ULyraNPCFunctionLibrary::GetNeedTypeName(ELyraNPCNeedType NeedType)
{
	switch (NeedType)
	{
	case ELyraNPCNeedType::Hunger: return TEXT("Hunger");
	case ELyraNPCNeedType::Energy: return TEXT("Energy");
	case ELyraNPCNeedType::Social: return TEXT("Social");
	case ELyraNPCNeedType::Safety: return TEXT("Safety");
	case ELyraNPCNeedType::Comfort: return TEXT("Comfort");
	case ELyraNPCNeedType::Entertainment: return TEXT("Entertainment");
	case ELyraNPCNeedType::Hygiene: return TEXT("Hygiene");
	case ELyraNPCNeedType::Purpose: return TEXT("Purpose");
	default: return TEXT("Unknown");
	}
}

FLyraNPCPersonality ULyraNPCFunctionLibrary::GenerateRandomPersonality()
{
	FLyraNPCPersonality Personality;

	Personality.Openness = FMath::FRandRange(0.1f, 0.9f);
	Personality.Conscientiousness = FMath::FRandRange(0.1f, 0.9f);
	Personality.Extraversion = FMath::FRandRange(0.1f, 0.9f);
	Personality.Agreeableness = FMath::FRandRange(0.1f, 0.9f);
	Personality.Neuroticism = FMath::FRandRange(0.1f, 0.9f);
	Personality.Bravery = FMath::FRandRange(0.1f, 0.9f);
	Personality.Honesty = FMath::FRandRange(0.1f, 0.9f);
	Personality.Curiosity = FMath::FRandRange(0.1f, 0.9f);
	Personality.Patience = FMath::FRandRange(0.1f, 0.9f);
	Personality.Loyalty = FMath::FRandRange(0.1f, 0.9f);

	return Personality;
}

float ULyraNPCFunctionLibrary::GetPersonalityCompatibility(const FLyraNPCPersonality& PersonalityA, const FLyraNPCPersonality& PersonalityB)
{
	// Calculate compatibility based on personality trait similarity
	float TotalDifference = 0.0f;

	TotalDifference += FMath::Abs(PersonalityA.Extraversion - PersonalityB.Extraversion);
	TotalDifference += FMath::Abs(PersonalityA.Agreeableness - PersonalityB.Agreeableness);
	TotalDifference += FMath::Abs(PersonalityA.Conscientiousness - PersonalityB.Conscientiousness);

	// Opposites attract for some traits
	float OpennessScore = 1.0f - FMath::Abs(PersonalityA.Openness - PersonalityB.Openness) * 0.5f;
	float NeuroticismScore = 1.0f - (PersonalityA.Neuroticism + PersonalityB.Neuroticism) * 0.3f;

	// Calculate final compatibility (0-100)
	float BaseCompatibility = 100.0f - (TotalDifference * 33.33f);
	BaseCompatibility *= OpennessScore;
	BaseCompatibility *= FMath::Max(0.5f, NeuroticismScore);

	return FMath::Clamp(BaseCompatibility, 0.0f, 100.0f);
}

float ULyraNPCFunctionLibrary::GetAverageNPCWellbeing(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetAverageNPCWellbeing() : 100.0f;
}

int32 ULyraNPCFunctionLibrary::GetNPCsInCombatCount(UObject* WorldContextObject)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetNPCsInCombatCount() : 0;
}

int32 ULyraNPCFunctionLibrary::GetNPCCountByAILOD(UObject* WorldContextObject, ELyraNPCAILOD LOD)
{
	ULyraNPCWorldSubsystem* Subsystem = GetNPCWorldSubsystem(WorldContextObject);
	return Subsystem ? Subsystem->GetNPCCountByLOD(LOD) : 0;
}
