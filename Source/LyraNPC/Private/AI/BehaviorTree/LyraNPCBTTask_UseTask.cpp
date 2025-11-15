// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTTask_UseTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "AI/Controllers/LyraNPCAIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "LyraNPCModule.h"

ULyraNPCBTTask_UseTask::ULyraNPCBTTask_UseTask()
{
	NodeName = "Use Task";
	bNotifyTick = true;

	TaskKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ULyraNPCBTTask_UseTask, TaskKey), UObject::StaticClass());
}

uint16 ULyraNPCBTTask_UseTask::GetInstanceMemorySize() const
{
	return sizeof(FTaskMemory);
}

EBTNodeResult::Type ULyraNPCBTTask_UseTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* Memory = reinterpret_cast<FTaskMemory*>(NodeMemory);
	Memory->bTaskStarted = false;
	Memory->RemainingTime = 0.0f;

	ALyraNPCAIController* AIController = Cast<ALyraNPCAIController>(OwnerComp.GetAIOwner());
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp)
	{
		return EBTNodeResult::Failed;
	}

	ULyraNPCTaskActor* Task = Cast<ULyraNPCTaskActor>(BBComp->GetValueAsObject(TaskKey.SelectedKeyName));
	if (!Task)
	{
		UE_LOG(LogLyraNPC, Warning, TEXT("UseTask: No task found in blackboard"));
		return EBTNodeResult::Failed;
	}

	// Start using the task
	if (AIController->StartUsingTask(Task))
	{
		Memory->bTaskStarted = true;
		Memory->RemainingTime = OverrideDuration > 0.0f ? OverrideDuration : Task->GetRandomDuration();

		UE_LOG(LogLyraNPC, Verbose, TEXT("Started using task: %s for %.1f seconds"), *Task->TaskName, Memory->RemainingTime);
		return EBTNodeResult::InProgress;
	}

	return EBTNodeResult::Failed;
}

void ULyraNPCBTTask_UseTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FTaskMemory* Memory = reinterpret_cast<FTaskMemory*>(NodeMemory);

	if (!Memory->bTaskStarted)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	Memory->RemainingTime -= DeltaSeconds;

	if (Memory->RemainingTime <= 0.0f)
	{
		ALyraNPCAIController* AIController = Cast<ALyraNPCAIController>(OwnerComp.GetAIOwner());
		if (AIController)
		{
			AIController->StopUsingCurrentTask();
		}

		UE_LOG(LogLyraNPC, Verbose, TEXT("Finished using task"));
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type ULyraNPCBTTask_UseTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* Memory = reinterpret_cast<FTaskMemory*>(NodeMemory);

	if (Memory->bTaskStarted)
	{
		ALyraNPCAIController* AIController = Cast<ALyraNPCAIController>(OwnerComp.GetAIOwner());
		if (AIController)
		{
			AIController->StopUsingCurrentTask();
		}
	}

	return EBTNodeResult::Aborted;
}

FString ULyraNPCBTTask_UseTask::GetStaticDescription() const
{
	FString Duration = OverrideDuration > 0.0f
		? FString::Printf(TEXT("%.1fs"), OverrideDuration)
		: TEXT("default duration");

	return FString::Printf(TEXT("Uses task from %s for %s"),
		*TaskKey.SelectedKeyName.ToString(), *Duration);
}
