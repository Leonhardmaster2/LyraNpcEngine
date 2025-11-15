// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCTaskActor.generated.h"

class ALyraNPCCharacter;

/**
 * Base component for task actors - world objects NPCs can interact with.
 * Examples: Beds, workbenches, chairs, forges, cooking spots, etc.
 * Add this to any actor to make it usable by NPCs.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Task Actor"))
class LYRANPC_API ULyraNPCTaskActor : public USceneComponent
{
	GENERATED_BODY()

public:
	ULyraNPCTaskActor();

	// ===== TASK CONFIGURATION =====

	// Unique identifier for this task type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Identity")
	FGameplayTag TaskType;

	// Human-readable name
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Identity")
	FString TaskName = TEXT("Generic Task");

	// Description of what this task does
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Identity")
	FString TaskDescription;

	// Which archetypes can use this task
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Access")
	TArray<ELyraNPCArchetype> AllowedArchetypes;

	// Required tags for NPC to use this task
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Access")
	FGameplayTagContainer RequiredTags;

	// Tags that block NPC from using this task
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Access")
	FGameplayTagContainer BlockingTags;

	// Maximum NPCs that can use this simultaneously
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Capacity", meta = (ClampMin = "1"))
	int32 MaxUsers = 1;

	// Is this task private (owned by specific NPC)?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Access")
	bool bIsPrivate = false;

	// Owner NPC (if private)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Access")
	FGuid OwnerNPCId;

	// ===== TASK BEHAVIOR =====

	// How long NPCs typically use this task (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Behavior")
	float DefaultDuration = 60.0f;

	// Minimum duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Behavior")
	float MinDuration = 30.0f;

	// Maximum duration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Behavior")
	float MaxDuration = 300.0f;

	// Priority of this task (higher = more likely to be chosen)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Behavior")
	float TaskPriority = 1.0f;

	// Can task be interrupted?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Behavior")
	bool bCanBeInterrupted = true;

	// ===== NEEDS SATISFACTION =====

	// Which needs this task satisfies and by how much (per minute of use)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Effects")
	TMap<ELyraNPCNeedType, float> NeedsSatisfaction;

	// ===== POSITIONING =====

	// Interaction points for NPCs (local space)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Positioning")
	TArray<FTransform> InteractionPoints;

	// Radius within which NPC can interact
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Positioning")
	float InteractionRadius = 100.0f;

	// ===== ANIMATION =====

	// Animation to play while using task (tag-based lookup)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Animation")
	FGameplayTag AnimationTag;

	// Should NPC sit/lay down for this task?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Animation")
	bool bSnapToPosition = true;

	// Should NPC face specific direction?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|Animation")
	bool bSnapRotation = true;

	// ===== STATE =====

	// Current users of this task
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task|State")
	TArray<TWeakObjectPtr<ALyraNPCCharacter>> CurrentUsers;

	// NPCs that have reserved this task
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task|State")
	TArray<TWeakObjectPtr<ALyraNPCCharacter>> ReservedBy;

	// Is task currently available?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Task|State")
	bool bIsAvailable = true;

	// Is task enabled?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task|State")
	bool bIsEnabled = true;

	// ===== EVENTS =====

	UPROPERTY(BlueprintAssignable, Category = "Task|Events")
	FOnNPCTaskStarted OnTaskStarted;

	UPROPERTY(BlueprintAssignable, Category = "Task|Events")
	FOnNPCTaskCompleted OnTaskCompleted;

public:
	// ===== RESERVATION SYSTEM =====

	UFUNCTION(BlueprintCallable, Category = "Task|Reservation")
	bool CanReserve(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintCallable, Category = "Task|Reservation")
	bool Reserve(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintCallable, Category = "Task|Reservation")
	void CancelReservation(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintPure, Category = "Task|Reservation")
	bool IsReservedBy(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "Task|Reservation")
	int32 GetAvailableSlots() const;

	// ===== USAGE =====

	UFUNCTION(BlueprintCallable, Category = "Task|Usage")
	bool StartUsing(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintCallable, Category = "Task|Usage")
	void StopUsing(ALyraNPCCharacter* NPC);

	UFUNCTION(BlueprintPure, Category = "Task|Usage")
	bool IsBeingUsedBy(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "Task|Usage")
	int32 GetCurrentUserCount() const;

	// ===== QUERIES =====

	UFUNCTION(BlueprintPure, Category = "Task|Query")
	bool CanNPCUseTask(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "Task|Query")
	float GetScoreForNPC(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "Task|Query")
	FTransform GetBestInteractionPoint(ALyraNPCCharacter* NPC) const;

	UFUNCTION(BlueprintPure, Category = "Task|Query")
	FVector GetTaskLocation() const;

	UFUNCTION(BlueprintPure, Category = "Task|Query")
	float GetRandomDuration() const;

	// ===== UTILITY =====

	UFUNCTION(BlueprintCallable, Category = "Task|Utility")
	void CleanupInvalidReferences();

	UFUNCTION(BlueprintCallable, Category = "Task|Utility")
	void SetEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Task|Utility")
	void SetOwner(const FGuid& NPCId);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void UpdateAvailability();
	bool CheckArchetypeAccess(ELyraNPCArchetype Archetype) const;
	bool CheckTagAccess(const FGameplayTagContainer& NPCTags) const;
};
