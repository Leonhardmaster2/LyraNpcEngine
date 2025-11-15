// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCNeedsComponent.generated.h"

/**
 * Component that manages NPC needs like hunger, energy, social, etc.
 * Needs decay over time and drive NPC behavior priorities.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Needs"))
class LYRANPC_API ULyraNPCNeedsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCNeedsComponent();

	// All active needs for this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", Replicated)
	TArray<FLyraNPCNeedState> Needs;

	// Time scale for need decay (1.0 = real time, 24.0 = 1 game day = 1 real hour)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Settings")
	float TimeScale = 24.0f;

	// Whether needs are currently being simulated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs|Settings")
	bool bSimulateNeeds = true;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Needs|Events")
	FOnNPCNeedCritical OnNeedCritical;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Initialize with default needs for archetype
	UFUNCTION(BlueprintCallable, Category = "Needs")
	void InitializeDefaultNeeds(ELyraNPCArchetype Archetype = ELyraNPCArchetype::Villager);

	// Get a specific need
	UFUNCTION(BlueprintPure, Category = "Needs")
	FLyraNPCNeedState GetNeed(ELyraNPCNeedType NeedType) const;

	UFUNCTION(BlueprintPure, Category = "Needs")
	float GetNeedValue(ELyraNPCNeedType NeedType) const;

	// Modify needs
	UFUNCTION(BlueprintCallable, Category = "Needs")
	void SetNeedValue(ELyraNPCNeedType NeedType, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void ModifyNeed(ELyraNPCNeedType NeedType, float Delta);

	UFUNCTION(BlueprintCallable, Category = "Needs")
	void SatisfyNeed(ELyraNPCNeedType NeedType, float Amount);

	// Queries
	UFUNCTION(BlueprintPure, Category = "Needs")
	bool HasCriticalNeed() const;

	UFUNCTION(BlueprintPure, Category = "Needs")
	bool HasUrgentNeed() const;

	UFUNCTION(BlueprintPure, Category = "Needs")
	ELyraNPCNeedType GetMostUrgentNeed() const;

	UFUNCTION(BlueprintPure, Category = "Needs")
	float GetOverallWellbeing() const;

	// Get priority score for a need (higher = more urgent)
	UFUNCTION(BlueprintPure, Category = "Needs")
	float GetNeedPriority(ELyraNPCNeedType NeedType) const;

	// Get all needs below a threshold
	UFUNCTION(BlueprintPure, Category = "Needs")
	TArray<ELyraNPCNeedType> GetNeedsBelowThreshold(float Threshold = 50.0f) const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void UpdateNeeds(float DeltaTime);
	void CheckCriticalNeeds();

	FLyraNPCNeedState* FindNeed(ELyraNPCNeedType NeedType);
	const FLyraNPCNeedState* FindNeed(ELyraNPCNeedType NeedType) const;
};
