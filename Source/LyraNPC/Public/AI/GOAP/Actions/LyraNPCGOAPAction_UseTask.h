// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "GameplayTagContainer.h"
#include "LyraNPCGOAPAction_UseTask.generated.h"

/**
 * Generic GOAP Action for using a task actor to satisfy needs.
 * This is a flexible action that can target different task types.
 */
UCLASS()
class LYRANPC_API ULyraNPCGOAPAction_UseTask : public ULyraNPCGOAPAction
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPAction_UseTask();

	virtual bool CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;
	virtual FLyraNPCWorldState ApplyEffects(const FLyraNPCWorldState& CurrentState) const override;
	virtual float CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;

	virtual void OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor) override;
	virtual bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime) override;
	virtual void OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess) override;

	// Configure this action for a specific task type
	UFUNCTION(BlueprintCallable, Category = "GOAP")
	void SetTaskType(FGameplayTag InTaskType);

protected:
	// The type of task this action targets
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	FGameplayTag TaskType;

	// How long using the task takes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float TaskDuration = 20.0f;

	// Whether this task satisfies a specific need
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	ELyraNPCNeedType TargetNeed = ELyraNPCNeedType::Comfort;

	// Minimum need value to trigger this action
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float MinimumNeedValue = 30.0f;

private:
	float TimeSpentOnTask = 0.0f;
	TWeakObjectPtr<class ULyraNPCTaskActor> CurrentTask;
};
