// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/EQS/LyraNPCEnvQueryGenerator_Tasks.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "Core/LyraNPCCharacter.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"
#include "AIController.h"

ULyraNPCEnvQueryGenerator_Tasks::ULyraNPCEnvQueryGenerator_Tasks()
{
	ItemType = UEnvQueryItemType_VectorBase::StaticClass();
	AutoRegisterWithAI = false;
}

void ULyraNPCEnvQueryGenerator_Tasks::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* QuerierObject = QueryInstance.Owner.Get();
	if (!QuerierObject)
	{
		return;
	}

	// Get the NPC character
	ALyraNPCCharacter* NPC = nullptr;

	if (AAIController* AIController = Cast<AAIController>(QuerierObject))
	{
		NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	}
	else if (APawn* Pawn = Cast<APawn>(QuerierObject))
	{
		NPC = Cast<ALyraNPCCharacter>(Pawn);
	}

	if (!NPC)
	{
		return;
	}

	// Get world subsystem for optimized task lookup
	UWorld* World = NPC->GetWorld();
	if (!World)
	{
		return;
	}

	ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
	if (!WorldSubsystem)
	{
		return;
	}

	// Get tasks within radius
	TArray<ULyraNPCTaskActor*> Tasks = WorldSubsystem->GetTasksInRadius(NPC->GetActorLocation(), SearchRadius);

	// Filter and sort tasks
	TArray<TPair<ULyraNPCTaskActor*, float>> ScoredTasks;

	for (ULyraNPCTaskActor* Task : Tasks)
	{
		if (!Task) continue;

		// Filter by task type
		if (TaskTypeFilter.IsValid() && !Task->TaskType.MatchesTag(TaskTypeFilter))
		{
			continue;
		}

		// Filter by availability
		if (bOnlyAvailable && !Task->bIsAvailable)
		{
			continue;
		}

		// Filter by NPC access
		if (bCheckNPCAccess && !Task->CanNPCUseTask(NPC))
		{
			continue;
		}

		// Calculate score
		float Score = Task->GetScoreForNPC(NPC);
		ScoredTasks.Add(TPair<ULyraNPCTaskActor*, float>(Task, Score));
	}

	// Sort by score (highest first)
	ScoredTasks.Sort([](const TPair<ULyraNPCTaskActor*, float>& A, const TPair<ULyraNPCTaskActor*, float>& B)
	{
		return A.Value > B.Value;
	});

	// Generate items
	TArray<FVector> TaskLocations;
	int32 Count = FMath::Min(ScoredTasks.Num(), MaxResults);

	for (int32 i = 0; i < Count; ++i)
	{
		FTransform InteractionPoint = ScoredTasks[i].Key->GetBestInteractionPoint(NPC);
		TaskLocations.Add(InteractionPoint.GetLocation());
	}

	QueryInstance.AddItemData<UEnvQueryItemType_VectorBase>(TaskLocations);
}

FText ULyraNPCEnvQueryGenerator_Tasks::GetDescriptionTitle() const
{
	return FText::FromString(TEXT("LyraNPC Tasks"));
}

FText ULyraNPCEnvQueryGenerator_Tasks::GetDescriptionDetails() const
{
	FString Details = FString::Printf(TEXT("Radius: %.0f"), SearchRadius);

	if (TaskTypeFilter.IsValid())
	{
		Details += FString::Printf(TEXT(", Type: %s"), *TaskTypeFilter.ToString());
	}

	if (bOnlyAvailable)
	{
		Details += TEXT(", Available Only");
	}

	return FText::FromString(Details);
}
