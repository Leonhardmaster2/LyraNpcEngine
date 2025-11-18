// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "LyraNPCGOAPAction_Sleep.generated.h"

/**
 * GOAP Action for sleeping to restore energy.
 */
UCLASS()
class LYRANPC_API ULyraNPCGOAPAction_Sleep : public ULyraNPCGOAPAction
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPAction_Sleep();

	virtual bool CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;
	virtual FLyraNPCWorldState ApplyEffects(const FLyraNPCWorldState& CurrentState) const override;
	virtual float CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;

	virtual void OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor) override;
	virtual bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime) override;
	virtual void OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess) override;

protected:
	// How much energy is restored (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float EnergyRestoration = 80.0f;

	// How long sleeping takes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float SleepDuration = 30.0f;

private:
	float TimeSpentSleeping = 0.0f;
	TWeakObjectPtr<class ULyraNPCTaskActor> Bed;
};
