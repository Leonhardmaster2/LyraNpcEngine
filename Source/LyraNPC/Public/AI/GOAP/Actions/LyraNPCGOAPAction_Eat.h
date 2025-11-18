// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "LyraNPCGOAPAction_Eat.generated.h"

/**
 * GOAP Action for eating to satisfy hunger need.
 */
UCLASS()
class LYRANPC_API ULyraNPCGOAPAction_Eat : public ULyraNPCGOAPAction
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPAction_Eat();

	virtual bool CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;
	virtual FLyraNPCWorldState ApplyEffects(const FLyraNPCWorldState& CurrentState) const override;
	virtual float CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;

	virtual void OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor) override;
	virtual bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime) override;
	virtual void OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess) override;

protected:
	// How much hunger is satisfied (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float HungerSatisfaction = 50.0f;

	// How long eating takes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float EatingDuration = 10.0f;

private:
	float TimeSpentEating = 0.0f;
	TWeakObjectPtr<class ULyraNPCTaskActor> FoodSource;
};
