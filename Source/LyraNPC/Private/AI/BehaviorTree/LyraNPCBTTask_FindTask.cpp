// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_FindTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Controllers/LyraNPCAIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCScheduleComponent.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Systems/LyraNPCWorldSubsystem.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_FindTask::ULyraNPCBTTask_FindTask()
{
	NodeName = "Find Best Task";

	// Setup blackboard key filters
	TargetTaskKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ULyraNPCBTTask_FindTask, TargetTaskKey), UObject::StaticClass());
	TaskLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(ULyraNPCBTTask_FindTask, TaskLocationKey));
}

EBTNodeResult::Type ULyraNPCBTTask_FindTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	ALyraNPCAIController* AIController = Cast<ALyraNPCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	ALyraNPCCharacter* NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPC)
	{
		return EBTNodeResult::Failed;
	}

	// Determine task type to search for
	FGameplayTag SearchTag = TaskTypeFilter;

	if (bUseScheduleForTaskType && NPC->ScheduleComponent)
	{
		FLyraNPCScheduleBlock CurrentBlock = NPC->ScheduleComponent->GetCurrentScheduledActivity();
		if (CurrentBlock.ActivityTag.IsValid())
		{
			// Map activity tags to task types
			FString ActivityStr = CurrentBlock.ActivityTag.ToString();
			if (ActivityStr.Contains(TEXT("Eat")))
			{
				SearchTag = FGameplayTag::RequestGameplayTag(TEXT("Task.Eat"));
			}
			else if (ActivityStr.Contains(TEXT("Sleep")))
			{
				SearchTag = FGameplayTag::RequestGameplayTag(TEXT("Task.Sleep"));
			}
			else if (ActivityStr.Contains(TEXT("Work")))
			{
				SearchTag = FGameplayTag::RequestGameplayTag(TEXT("Task.Work"));
			}
			else if (ActivityStr.Contains(TEXT("Leisure")))
			{
				SearchTag = FGameplayTag::RequestGameplayTag(TEXT("Task.Leisure"));
			}
		}
	}

	// Use world subsystem for optimized search
	ULyraNPCWorldSubsystem* WorldSubsystem = NPC->GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();
	ULyraNPCTaskActor* BestTask = nullptr;

	if (WorldSubsystem)
	{
		BestTask = WorldSubsystem->FindBestTaskForNPC(NPC, SearchTag);
	}
	else
	{
		// Fallback to controller's search
		BestTask = AIController->FindBestTask(SearchTag);
	}

	if (BestTask)
	{
		// Store in blackboard
		UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
		if (BBComp)
		{
			BBComp->SetValueAsObject(TargetTaskKey.SelectedKeyName, BestTask);

			FTransform InteractionPoint = BestTask->GetBestInteractionPoint(NPC);
			BBComp->SetValueAsVector(TaskLocationKey.SelectedKeyName, InteractionPoint.GetLocation());
		}

		UE_LOG(LogLyraNPC, Verbose, TEXT("Found task: %s"), *BestTask->TaskName);
		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogLyraNPC, Verbose, TEXT("No suitable task found"));
	return EBTNodeResult::Failed;
}

FString ULyraNPCBTTask_FindTask::GetStaticDescription() const
{
	FString Description = TEXT("Finds best task");
	if (TaskTypeFilter.IsValid())
	{
		Description += FString::Printf(TEXT(" of type: %s"), *TaskTypeFilter.ToString());
	}
	if (bUseScheduleForTaskType)
	{
		Description += TEXT(" (uses schedule)");
	}
	return Description;
}
