// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AI/GOAP/LyraNPCGOAPAction.h"
#include "LyraNPCGOAPAction_Socialize.generated.h"

/**
 * GOAP Action for socializing with other NPCs to satisfy social need.
 */
UCLASS()
class LYRANPC_API ULyraNPCGOAPAction_Socialize : public ULyraNPCGOAPAction
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPAction_Socialize();

	virtual bool CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;
	virtual FLyraNPCWorldState ApplyEffects(const FLyraNPCWorldState& CurrentState) const override;
	virtual float CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const override;

	virtual void OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor) override;
	virtual bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime) override;
	virtual void OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess) override;

protected:
	// How much social need is satisfied (0-100)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float SocialSatisfaction = 40.0f;

	// How long socializing takes
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float SocializeDuration = 15.0f;

	// Maximum distance to find social partners
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP")
	float SearchRadius = 1000.0f;

private:
	float TimeSpentSocializing = 0.0f;
	TWeakObjectPtr<ALyraNPCCharacter> SocialPartner;
};
