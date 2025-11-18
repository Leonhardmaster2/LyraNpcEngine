// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LyraNPCGroupTypes.generated.h"

class ALyraNPCCharacter;

/**
 * Role that an NPC can have within a group.
 */
UENUM(BlueprintType)
enum class ELyraNPCGroupRole : uint8
{
	None UMETA(DisplayName = "None"),
	Leader UMETA(DisplayName = "Leader"),
	Scout UMETA(DisplayName = "Scout"),
	Flanker UMETA(DisplayName = "Flanker"),
	Support UMETA(DisplayName = "Support"),
	HeavyHitter UMETA(DisplayName = "Heavy Hitter"),
	Defender UMETA(DisplayName = "Defender"),
	Medic UMETA(DisplayName = "Medic"),
	Follower UMETA(DisplayName = "Follower")
};

/**
 * Formation type for group movement.
 */
UENUM(BlueprintType)
enum class ELyraNPCGroupFormation : uint8
{
	None UMETA(DisplayName = "None"),
	Line UMETA(DisplayName = "Line"),
	Column UMETA(DisplayName = "Column"),
	Wedge UMETA(DisplayName = "Wedge"),
	Circle UMETA(DisplayName = "Circle"),
	Scatter UMETA(DisplayName = "Scatter"),
	Shield UMETA(DisplayName = "Shield Wall")
};

/**
 * Group tactical state.
 */
UENUM(BlueprintType)
enum class ELyraNPCGroupTactic : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Patrol UMETA(DisplayName = "Patrol"),
	Hold UMETA(DisplayName = "Hold Position"),
	Advance UMETA(DisplayName = "Advance"),
	Retreat UMETA(DisplayName = "Retreat"),
	Flank UMETA(DisplayName = "Flank"),
	Surround UMETA(DisplayName = "Surround"),
	Ambush UMETA(DisplayName = "Ambush"),
	Defensive UMETA(DisplayName = "Defensive")
};

/**
 * Type of shared knowledge within a group.
 */
UENUM(BlueprintType)
enum class ELyraNPCGroupKnowledgeType : uint8
{
	EnemySighting UMETA(DisplayName = "Enemy Sighting"),
	ThreatLocation UMETA(DisplayName = "Threat Location"),
	SafeLocation UMETA(DisplayName = "Safe Location"),
	ResourceLocation UMETA(DisplayName = "Resource Location"),
	InjuredAlly UMETA(DisplayName = "Injured Ally"),
	DeadAlly UMETA(DisplayName = "Dead Ally"),
	PathBlocked UMETA(DisplayName = "Path Blocked"),
	AlarmRaised UMETA(DisplayName = "Alarm Raised")
};

/**
 * Shared piece of tactical knowledge within a group.
 */
USTRUCT(BlueprintType)
struct FLyraNPCGroupKnowledge
{
	GENERATED_BODY()

	// Type of knowledge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELyraNPCGroupKnowledgeType Type = ELyraNPCGroupKnowledgeType::EnemySighting;

	// Location of the knowledge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Location = FVector::ZeroVector;

	// Related actor (if any)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> RelatedActor = nullptr;

	// Who reported this knowledge
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid ReporterId;

	// When this knowledge was added
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Timestamp = 0.0f;

	// How important/urgent this knowledge is
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Urgency = 0.5f;

	// How accurate this knowledge is (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Accuracy = 1.0f;

	// Additional context data
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ContextTag;
};

/**
 * Formation position for a group member.
 */
USTRUCT(BlueprintType)
struct FLyraNPCFormationSlot
{
	GENERATED_BODY()

	// Relative offset from formation center
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector RelativeOffset = FVector::ZeroVector;

	// Role best suited for this position
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELyraNPCGroupRole PreferredRole = ELyraNPCGroupRole::Follower;

	// Assigned NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<ALyraNPCCharacter> AssignedNPC = nullptr;

	// Is this slot occupied
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOccupied = false;
};

/**
 * Group member data.
 */
USTRUCT(BlueprintType)
struct FLyraNPCGroupMember
{
	GENERATED_BODY()

	// The NPC character
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<ALyraNPCCharacter> NPC = nullptr;

	// Unique ID of the NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGuid NPCId;

	// Role in the group
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ELyraNPCGroupRole Role = ELyraNPCGroupRole::Follower;

	// Formation slot index
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 FormationSlot = -1;

	// When they joined the group
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float JoinTime = 0.0f;

	// How much this member contributes to group decisions (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Influence = 0.5f;

	// Last known health percentage
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LastKnownHealth = 100.0f;
};

/**
 * Coordinated group action.
 */
USTRUCT(BlueprintType)
struct FLyraNPCCoordinatedAction
{
	GENERATED_BODY()

	// Action identifier
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag ActionTag;

	// Target location
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetLocation = FVector::ZeroVector;

	// Target actor
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TWeakObjectPtr<AActor> TargetActor = nullptr;

	// NPCs assigned to this action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGuid> AssignedNPCs;

	// When this action should start (synchronized)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ScheduledStartTime = 0.0f;

	// Maximum duration for this action
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDuration = 30.0f;

	// Action priority
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Priority = 0.5f;

	// Is this action currently active
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive = false;

	// Has this action been completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCompleted = false;
};

/**
 * Group emotional state (can spread through members).
 */
USTRUCT(BlueprintType)
struct FLyraNPCGroupMood
{
	GENERATED_BODY()

	// Average fear level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Fear = 0.0f;

	// Average aggression level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Aggression = 0.5f;

	// Average morale (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Morale = 0.7f;

	// Group cohesion (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cohesion = 0.5f;
};
