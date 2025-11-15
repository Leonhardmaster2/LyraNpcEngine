// Copyright LyraNPC Framework. All Rights Reserved.

#include "Systems/LyraNPCWorldSubsystem.h"
#include "Core/LyraNPCCharacter.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Components/LyraNPCScheduleComponent.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "AI/Controllers/LyraNPCAIController.h"
#include "LyraNPCModule.h"

ULyraNPCWorldSubsystem::ULyraNPCWorldSubsystem()
{
}

void ULyraNPCWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogLyraNPC, Log, TEXT("LyraNPC World Subsystem Initialized"));
}

void ULyraNPCWorldSubsystem::Deinitialize()
{
	RegisteredNPCs.Empty();
	RegisteredTasks.Empty();
	Super::Deinitialize();
	UE_LOG(LogLyraNPC, Log, TEXT("LyraNPC World Subsystem Deinitialized"));
}

void ULyraNPCWorldSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAutoAdvanceTime)
	{
		UpdateGlobalTime(DeltaTime);
	}

	TimeSinceLastCleanup += DeltaTime;
	if (TimeSinceLastCleanup >= CleanupInterval)
	{
		CleanupInvalidReferences();
		TimeSinceLastCleanup = 0.0f;
	}
}

TStatId ULyraNPCWorldSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULyraNPCWorldSubsystem, STATGROUP_Tickables);
}

void ULyraNPCWorldSubsystem::RegisterNPC(ALyraNPCCharacter* NPC)
{
	if (NPC && !RegisteredNPCs.Contains(NPC))
	{
		RegisteredNPCs.Add(NPC);
		UE_LOG(LogLyraNPC, Verbose, TEXT("Registered NPC: %s (Total: %d)"), *NPC->GetNPCName(), RegisteredNPCs.Num());
	}
}

void ULyraNPCWorldSubsystem::UnregisterNPC(ALyraNPCCharacter* NPC)
{
	RegisteredNPCs.Remove(NPC);
	UE_LOG(LogLyraNPC, Verbose, TEXT("Unregistered NPC (Total: %d)"), RegisteredNPCs.Num());
}

TArray<ALyraNPCCharacter*> ULyraNPCWorldSubsystem::GetAllNPCs() const
{
	TArray<ALyraNPCCharacter*> Result;
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			Result.Add(NPCPtr.Get());
		}
	}
	return Result;
}

TArray<ALyraNPCCharacter*> ULyraNPCWorldSubsystem::GetNPCsByArchetype(ELyraNPCArchetype Archetype) const
{
	TArray<ALyraNPCCharacter*> Result;
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid() && NPCPtr->GetArchetype() == Archetype)
		{
			Result.Add(NPCPtr.Get());
		}
	}
	return Result;
}

TArray<ALyraNPCCharacter*> ULyraNPCWorldSubsystem::GetNPCsInRadius(FVector Location, float Radius) const
{
	TArray<ALyraNPCCharacter*> Result;
	float RadiusSq = Radius * Radius;

	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			float DistSq = FVector::DistSquared(NPCPtr->GetActorLocation(), Location);
			if (DistSq <= RadiusSq)
			{
				Result.Add(NPCPtr.Get());
			}
		}
	}
	return Result;
}

ALyraNPCCharacter* ULyraNPCWorldSubsystem::FindNPCById(const FGuid& NPCId) const
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ULyraNPCIdentityComponent* Identity = NPCPtr->IdentityComponent)
			{
				if (Identity->GetUniqueId() == NPCId)
				{
					return NPCPtr.Get();
				}
			}
		}
	}
	return nullptr;
}

ALyraNPCCharacter* ULyraNPCWorldSubsystem::FindNPCByName(const FString& Name) const
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (NPCPtr->GetNPCName().Contains(Name))
			{
				return NPCPtr.Get();
			}
		}
	}
	return nullptr;
}

void ULyraNPCWorldSubsystem::RegisterTaskActor(ULyraNPCTaskActor* Task)
{
	if (Task && !RegisteredTasks.Contains(Task))
	{
		RegisteredTasks.Add(Task);
		UE_LOG(LogLyraNPC, Verbose, TEXT("Registered Task: %s (Total: %d)"), *Task->TaskName, RegisteredTasks.Num());
	}
}

void ULyraNPCWorldSubsystem::UnregisterTaskActor(ULyraNPCTaskActor* Task)
{
	RegisteredTasks.Remove(Task);
}

TArray<ULyraNPCTaskActor*> ULyraNPCWorldSubsystem::GetAllTasks() const
{
	TArray<ULyraNPCTaskActor*> Result;
	for (const TWeakObjectPtr<ULyraNPCTaskActor>& TaskPtr : RegisteredTasks)
	{
		if (TaskPtr.IsValid())
		{
			Result.Add(TaskPtr.Get());
		}
	}
	return Result;
}

TArray<ULyraNPCTaskActor*> ULyraNPCWorldSubsystem::GetTasksByType(FGameplayTag TaskType) const
{
	TArray<ULyraNPCTaskActor*> Result;
	for (const TWeakObjectPtr<ULyraNPCTaskActor>& TaskPtr : RegisteredTasks)
	{
		if (TaskPtr.IsValid() && TaskPtr->TaskType.MatchesTag(TaskType))
		{
			Result.Add(TaskPtr.Get());
		}
	}
	return Result;
}

TArray<ULyraNPCTaskActor*> ULyraNPCWorldSubsystem::GetAvailableTasksForNPC(ALyraNPCCharacter* NPC) const
{
	TArray<ULyraNPCTaskActor*> Result;
	for (const TWeakObjectPtr<ULyraNPCTaskActor>& TaskPtr : RegisteredTasks)
	{
		if (TaskPtr.IsValid() && TaskPtr->bIsAvailable && TaskPtr->CanNPCUseTask(NPC))
		{
			Result.Add(TaskPtr.Get());
		}
	}
	return Result;
}

ULyraNPCTaskActor* ULyraNPCWorldSubsystem::FindBestTaskForNPC(ALyraNPCCharacter* NPC, FGameplayTag TaskType) const
{
	ULyraNPCTaskActor* BestTask = nullptr;
	float BestScore = 0.0f;

	for (const TWeakObjectPtr<ULyraNPCTaskActor>& TaskPtr : RegisteredTasks)
	{
		if (!TaskPtr.IsValid() || !TaskPtr->bIsAvailable) continue;

		// Filter by type if specified
		if (TaskType.IsValid() && !TaskPtr->TaskType.MatchesTag(TaskType)) continue;

		float Score = TaskPtr->GetScoreForNPC(NPC);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTask = TaskPtr.Get();
		}
	}

	return BestTask;
}

TArray<ULyraNPCTaskActor*> ULyraNPCWorldSubsystem::GetTasksInRadius(FVector Location, float Radius) const
{
	TArray<ULyraNPCTaskActor*> Result;
	float RadiusSq = Radius * Radius;

	for (const TWeakObjectPtr<ULyraNPCTaskActor>& TaskPtr : RegisteredTasks)
	{
		if (TaskPtr.IsValid())
		{
			float DistSq = FVector::DistSquared(TaskPtr->GetTaskLocation(), Location);
			if (DistSq <= RadiusSq)
			{
				Result.Add(TaskPtr.Get());
			}
		}
	}
	return Result;
}

void ULyraNPCWorldSubsystem::SetGlobalGameHour(float NewHour)
{
	GlobalGameHour = FMath::Fmod(NewHour, 24.0f);
	SyncAllNPCSchedulesToGlobalTime();
}

void ULyraNPCWorldSubsystem::AdvanceGlobalTime(float Hours)
{
	GlobalGameHour += Hours;
	while (GlobalGameHour >= 24.0f)
	{
		GlobalGameHour -= 24.0f;
	}
	SyncAllNPCSchedulesToGlobalTime();
}

void ULyraNPCWorldSubsystem::SetTimeScale(float NewScale)
{
	GlobalTimeScale = NewScale;
	SetAllNPCsTimeScale(NewScale);
}

bool ULyraNPCWorldSubsystem::IsGlobalNightTime() const
{
	return GlobalGameHour < 6.0f || GlobalGameHour >= 20.0f;
}

bool ULyraNPCWorldSubsystem::IsGlobalDayTime() const
{
	return !IsGlobalNightTime();
}

FString ULyraNPCWorldSubsystem::GetTimeString() const
{
	int32 Hours = FMath::FloorToInt(GlobalGameHour);
	int32 Minutes = FMath::FloorToInt((GlobalGameHour - Hours) * 60.0f);
	return FString::Printf(TEXT("%02d:%02d"), Hours, Minutes);
}

int32 ULyraNPCWorldSubsystem::GetNPCCountByLOD(ELyraNPCAILOD LOD) const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ALyraNPCAIController* Controller = Cast<ALyraNPCAIController>(NPCPtr->GetController()))
			{
				if (Controller->CurrentAILOD == LOD)
				{
					Count++;
				}
			}
		}
	}
	return Count;
}

float ULyraNPCWorldSubsystem::GetAverageNPCWellbeing() const
{
	if (RegisteredNPCs.Num() == 0) return 100.0f;

	float TotalWellbeing = 0.0f;
	int32 ValidCount = 0;

	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			TotalWellbeing += NPCPtr->GetOverallWellbeing();
			ValidCount++;
		}
	}

	return ValidCount > 0 ? TotalWellbeing / ValidCount : 100.0f;
}

int32 ULyraNPCWorldSubsystem::GetNPCsInCombatCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid() && NPCPtr->GetAlertLevel() == ELyraNPCAlertLevel::Combat)
		{
			Count++;
		}
	}
	return Count;
}

void ULyraNPCWorldSubsystem::SetAllNPCsTimeScale(float NewTimeScale)
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ULyraNPCScheduleComponent* Schedule = NPCPtr->ScheduleComponent)
			{
				Schedule->TimeScale = NewTimeScale;
			}
			if (ULyraNPCNeedsComponent* Needs = NPCPtr->NeedsComponent)
			{
				Needs->TimeScale = NewTimeScale;
			}
		}
	}
}

void ULyraNPCWorldSubsystem::PauseAllNPCs()
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ALyraNPCAIController* Controller = Cast<ALyraNPCAIController>(NPCPtr->GetController()))
			{
				Controller->PauseBehaviorTree();
			}
		}
	}
}

void ULyraNPCWorldSubsystem::ResumeAllNPCs()
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ALyraNPCAIController* Controller = Cast<ALyraNPCAIController>(NPCPtr->GetController()))
			{
				Controller->ResumeBehaviorTree();
			}
		}
	}
}

void ULyraNPCWorldSubsystem::SyncAllNPCSchedulesToGlobalTime()
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& NPCPtr : RegisteredNPCs)
	{
		if (NPCPtr.IsValid())
		{
			if (ULyraNPCScheduleComponent* Schedule = NPCPtr->ScheduleComponent)
			{
				Schedule->SetGameHour(GlobalGameHour);
			}
		}
	}
}

void ULyraNPCWorldSubsystem::UpdateGlobalTime(float DeltaTime)
{
	float GameTimeAdvance = (DeltaTime / 3600.0f) * GlobalTimeScale;
	GlobalGameHour += GameTimeAdvance;

	while (GlobalGameHour >= 24.0f)
	{
		GlobalGameHour -= 24.0f;
	}
}

void ULyraNPCWorldSubsystem::CleanupInvalidReferences()
{
	// Clean up invalid NPC references
	for (int32 i = RegisteredNPCs.Num() - 1; i >= 0; --i)
	{
		if (!RegisteredNPCs[i].IsValid())
		{
			RegisteredNPCs.RemoveAt(i);
		}
	}

	// Clean up invalid task references
	for (int32 i = RegisteredTasks.Num() - 1; i >= 0; --i)
	{
		if (!RegisteredTasks[i].IsValid())
		{
			RegisteredTasks.RemoveAt(i);
		}
	}
}
