// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AI/GOAP/LyraNPCGOAPTypes.h"
#include "LyraNPCGOAPAction.generated.h"

class ALyraNPCCharacter;

/**
 * Base class for all GOAP actions.
 * Actions represent things NPCs can do to change the world state.
 */
UCLASS(Abstract, Blueprintable, EditInlineNew)
class LYRANPC_API ULyraNPCGOAPAction : public UObject
{
	GENERATED_BODY()

public:
	ULyraNPCGOAPAction();

	// ===== CONFIGURATION =====

	// Unique identifier for this action type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Identity")
	FGameplayTag ActionTag;

	// Display name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Identity")
	FString ActionName;

	// Description of what this action does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Identity")
	FString ActionDescription;

	// Preconditions that must be met for this action to be valid
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Conditions")
	FLyraNPCWorldState Preconditions;

	// Effects this action has on world state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Conditions")
	FLyraNPCWorldState Effects;

	// Cost configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Cost")
	FLyraNPCActionCost Cost;

	// Does this action require a target location?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Targeting")
	bool bRequiresTarget = false;

	// Does this action require a target actor?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Targeting")
	bool bRequiresTargetActor = false;

	// Maximum range for this action (-1 = unlimited)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GOAP|Targeting")
	float MaxRange = -1.0f;

	// ===== PLANNING =====

	/**
	 * Check if this action is valid in the current context
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	bool IsValid(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;
	virtual bool IsValid_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;

	/**
	 * Check if preconditions are met
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	bool CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;
	virtual bool CheckPreconditions_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;

	/**
	 * Apply effects to world state (for planning)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	FLyraNPCWorldState ApplyEffects(const FLyraNPCWorldState& CurrentState) const;
	virtual FLyraNPCWorldState ApplyEffects_Implementation(const FLyraNPCWorldState& CurrentState) const;

	/**
	 * Calculate the cost of performing this action
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	float CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;
	virtual float CalculateCost_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;

	/**
	 * Find a target for this action (location or actor)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	bool FindTarget(ALyraNPCCharacter* NPC, FVector& OutLocation, AActor*& OutActor);
	virtual bool FindTarget_Implementation(ALyraNPCCharacter* NPC, FVector& OutLocation, AActor*& OutActor);

	// ===== EXECUTION =====

	/**
	 * Called when action execution starts
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	void OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor);
	virtual void OnActionStart_Implementation(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor);

	/**
	 * Called every tick while action is executing
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime);
	virtual bool TickAction_Implementation(ALyraNPCCharacter* NPC, float DeltaTime);

	/**
	 * Called when action completes (successfully or failed)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	void OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess);
	virtual void OnActionEnd_Implementation(ALyraNPCCharacter* NPC, bool bSuccess);

	/**
	 * Check if action is complete
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	bool IsComplete(ALyraNPCCharacter* NPC) const;
	virtual bool IsComplete_Implementation(ALyraNPCCharacter* NPC) const;

	/**
	 * Abort action execution
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	void AbortAction(ALyraNPCCharacter* NPC);
	virtual void AbortAction_Implementation(ALyraNPCCharacter* NPC);

	// ===== UTILITY =====

	/**
	 * Get procedural effects based on current context
	 * (Override for dynamic effects)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "GOAP")
	FLyraNPCWorldState GetProceduralEffects(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;
	virtual FLyraNPCWorldState GetProceduralEffects_Implementation(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const;

	/**
	 * Get display name for debugging
	 */
	FString GetDisplayName() const { return ActionName.IsEmpty() ? GetName() : ActionName; }

protected:
	// Current execution state
	UPROPERTY(Transient)
	bool bIsExecuting = false;

	UPROPERTY(Transient)
	float ExecutionStartTime = 0.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<ALyraNPCCharacter> CurrentNPC;

	// Helper to get world from NPC
	UWorld* GetWorldFromNPC(ALyraNPCCharacter* NPC) const;
};
