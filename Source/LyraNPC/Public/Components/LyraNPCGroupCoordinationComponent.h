// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/Group/LyraNPCGroupTypes.h"
#include "GameplayTagContainer.h"
#include "LyraNPCGroupCoordinationComponent.generated.h"

class ALyraNPCCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupMemberJoined, ALyraNPCCharacter*, NewMember, ELyraNPCGroupRole, AssignedRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroupMemberLeft, FGuid, MemberId, ELyraNPCGroupRole, FormerRole);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroupKnowledgeAdded, const FLyraNPCGroupKnowledge&, Knowledge);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroupTacticChanged, ELyraNPCGroupTactic, NewTactic);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCoordinatedActionStarted, const FLyraNPCCoordinatedAction&, Action);

/**
 * Component that enables NPCs to coordinate as a group.
 * Provides shared knowledge, tactical coordination, formations, and group decision making.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent))
class LYRANPC_API ULyraNPCGroupCoordinationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCGroupCoordinationComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ===== CONFIGURATION =====

	// Unique group identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	FName GroupId = NAME_None;

	// Maximum members in this group
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	int32 MaxGroupSize = 8;

	// Enable automatic role assignment
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	bool bAutoAssignRoles = true;

	// Enable formation movement
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	bool bUseFormations = true;

	// Enable shared tactical knowledge
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	bool bShareKnowledge = true;

	// Enable emotional contagion (fear/aggression spreading)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	bool bEmotionalContagion = true;

	// How long knowledge stays relevant (seconds)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	float KnowledgeDecayTime = 300.0f;

	// Minimum morale before group considers retreating
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	float RetreatMoraleThreshold = 0.3f;

	// Distance threshold for formation positions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Group")
	float FormationTolerance = 200.0f;

	// ===== GROUP STATE =====

	// All members in this group
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "LyraNPC|Group")
	TArray<FLyraNPCGroupMember> Members;

	// Current formation
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "LyraNPC|Group")
	ELyraNPCGroupFormation CurrentFormation = ELyraNPCGroupFormation::None;

	// Current tactical state
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "LyraNPC|Group")
	ELyraNPCGroupTactic CurrentTactic = ELyraNPCGroupTactic::Idle;

	// Shared knowledge pool
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "LyraNPC|Group")
	TArray<FLyraNPCGroupKnowledge> SharedKnowledge;

	// Active coordinated actions
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "LyraNPC|Group")
	TArray<FLyraNPCCoordinatedAction> CoordinatedActions;

	// Formation slots
	UPROPERTY(BlueprintReadOnly, Category = "LyraNPC|Group")
	TArray<FLyraNPCFormationSlot> FormationSlots;

	// Group emotional state
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "LyraNPC|Group")
	FLyraNPCGroupMood GroupMood;

	// Formation center point
	UPROPERTY(BlueprintReadWrite, Category = "LyraNPC|Group")
	FVector FormationCenter = FVector::ZeroVector;

	// Formation facing direction
	UPROPERTY(BlueprintReadWrite, Category = "LyraNPC|Group")
	FVector FormationDirection = FVector::ForwardVector;

	// ===== EVENTS =====

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Group|Events")
	FOnGroupMemberJoined OnGroupMemberJoined;

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Group|Events")
	FOnGroupMemberLeft OnGroupMemberLeft;

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Group|Events")
	FOnGroupKnowledgeAdded OnGroupKnowledgeAdded;

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Group|Events")
	FOnGroupTacticChanged OnGroupTacticChanged;

	UPROPERTY(BlueprintAssignable, Category = "LyraNPC|Group|Events")
	FOnCoordinatedActionStarted OnCoordinatedActionStarted;

	// ===== GROUP MEMBERSHIP =====

	// Add a member to the group
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	bool AddMember(ALyraNPCCharacter* NPC, ELyraNPCGroupRole Role = ELyraNPCGroupRole::Follower);

	// Remove a member from the group
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	bool RemoveMember(const FGuid& NPCId);

	// Change a member's role
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void SetMemberRole(const FGuid& NPCId, ELyraNPCGroupRole NewRole);

	// Get a member's role
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	ELyraNPCGroupRole GetMemberRole(const FGuid& NPCId) const;

	// Check if NPC is in this group
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	bool IsMember(const FGuid& NPCId) const;

	// Get member count
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	int32 GetMemberCount() const { return Members.Num(); }

	// Get the group leader
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	ALyraNPCCharacter* GetLeader() const;

	// Get all members with a specific role
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	TArray<ALyraNPCCharacter*> GetMembersByRole(ELyraNPCGroupRole Role) const;

	// ===== SHARED KNOWLEDGE =====

	// Share knowledge with the group
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ShareKnowledge(const FLyraNPCGroupKnowledge& Knowledge);

	// Get knowledge of a specific type
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	TArray<FLyraNPCGroupKnowledge> GetKnowledgeByType(ELyraNPCGroupKnowledgeType Type) const;

	// Get most urgent knowledge
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	FLyraNPCGroupKnowledge GetMostUrgentKnowledge() const;

	// Clear old knowledge
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ClearOldKnowledge();

	// Quick helpers for common knowledge types
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ReportEnemySighting(AActor* Enemy, FVector Location, const FGuid& ReporterId, float Urgency = 0.8f);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ReportThreat(FVector Location, const FGuid& ReporterId, float Urgency = 0.7f);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ReportAllyDown(ALyraNPCCharacter* Ally, const FGuid& ReporterId);

	// ===== FORMATIONS =====

	// Set the formation type
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void SetFormation(ELyraNPCGroupFormation Formation);

	// Update formation center and direction
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void UpdateFormation(FVector Center, FVector Direction);

	// Get formation position for a member
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	FVector GetFormationPositionForMember(const FGuid& NPCId) const;

	// Get formation position for a slot
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	FVector GetFormationPositionForSlot(int32 SlotIndex) const;

	// Assign members to formation slots
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void AssignFormationSlots();

	// ===== TACTICS =====

	// Change the group's tactic
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void SetTactic(ELyraNPCGroupTactic NewTactic);

	// Automatically choose best tactic based on situation
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void EvaluateAndSetTactic();

	// Check if group should retreat
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	bool ShouldRetreat() const;

	// ===== COORDINATED ACTIONS =====

	// Start a coordinated action
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	bool StartCoordinatedAction(const FLyraNPCCoordinatedAction& Action);

	// Cancel a coordinated action
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void CancelCoordinatedAction(FGameplayTag ActionTag);

	// Get active coordinated action for an NPC
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	FLyraNPCCoordinatedAction GetActiveActionForNPC(const FGuid& NPCId) const;

	// Is NPC assigned to a coordinated action?
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	bool IsNPCInCoordinatedAction(const FGuid& NPCId) const;

	// ===== GROUP MOOD & EMOTIONS =====

	// Update group mood based on members
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void UpdateGroupMood();

	// Apply emotional contagion to members
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	void ApplyEmotionalContagion(float DeltaTime);

	// Get average health of group
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	float GetAverageGroupHealth() const;

	// Get number of enemies known to the group
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	int32 GetKnownEnemyCount() const;

	// ===== DECISION MAKING =====

	// Vote on a decision (returns percentage of votes in favor)
	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Group")
	float VoteOnDecision(FGameplayTag DecisionTag) const;

	// Get most influential member
	UFUNCTION(BlueprintPure, Category = "LyraNPC|Group")
	ALyraNPCCharacter* GetMostInfluentialMember() const;

private:
	// Find member data by ID
	FLyraNPCGroupMember* FindMember(const FGuid& NPCId);
	const FLyraNPCGroupMember* FindMember(const FGuid& NPCId) const;

	// Auto-assign roles based on NPC archetypes and skills
	void AutoAssignRole(FLyraNPCGroupMember& Member);

	// Generate formation slots based on formation type
	void GenerateFormationSlots();

	// Update coordinated actions
	void UpdateCoordinatedActions(float DeltaTime);

	// Clean up dead members
	void CleanupInvalidMembers();

	// Timing
	float TimeSinceLastUpdate = 0.0f;
	float TimeSinceLastMoodUpdate = 0.0f;
};
