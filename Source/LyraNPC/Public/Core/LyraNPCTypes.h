// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "LyraNPCTypes.generated.h"

// Forward declarations
class ULyraNPCTaskActor;
class ALyraNPCCharacter;

/**
 * AI Level of Detail - determines how much processing an NPC receives
 */
UENUM(BlueprintType)
enum class ELyraNPCAILOD : uint8
{
	Full		UMETA(DisplayName = "Full AI", ToolTip = "Complete AI processing - perception, decisions, animations"),
	Reduced		UMETA(DisplayName = "Reduced AI", ToolTip = "Simplified perception and decision making"),
	Minimal		UMETA(DisplayName = "Minimal AI", ToolTip = "Schedule-based only, minimal processing"),
	Dormant		UMETA(DisplayName = "Dormant", ToolTip = "No processing, state preserved"),
	MAX			UMETA(Hidden)
};

/**
 * NPC Archetype - defines the general role/type of NPC
 */
UENUM(BlueprintType)
enum class ELyraNPCArchetype : uint8
{
	Villager	UMETA(DisplayName = "Villager", ToolTip = "Civilian NPC with daily routine"),
	Guard		UMETA(DisplayName = "Guard", ToolTip = "Protective NPC with patrol duties"),
	Worker		UMETA(DisplayName = "Worker", ToolTip = "Labor-focused NPC"),
	Merchant	UMETA(DisplayName = "Merchant", ToolTip = "Trading and shop-keeping NPC"),
	Traveler	UMETA(DisplayName = "Traveler", ToolTip = "Wandering NPC"),
	Enemy		UMETA(DisplayName = "Enemy", ToolTip = "Hostile NPC"),
	Neutral		UMETA(DisplayName = "Neutral", ToolTip = "Non-aligned NPC"),
	Companion	UMETA(DisplayName = "Companion", ToolTip = "Follower NPC"),
	Custom		UMETA(DisplayName = "Custom", ToolTip = "User-defined archetype")
};

/**
 * NPC Life State - current overall state of the NPC
 */
UENUM(BlueprintType)
enum class ELyraNPCLifeState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Working		UMETA(DisplayName = "Working"),
	Resting		UMETA(DisplayName = "Resting"),
	Eating		UMETA(DisplayName = "Eating"),
	Socializing	UMETA(DisplayName = "Socializing"),
	Traveling	UMETA(DisplayName = "Traveling"),
	Patrolling	UMETA(DisplayName = "Patrolling"),
	Combat		UMETA(DisplayName = "In Combat"),
	Fleeing		UMETA(DisplayName = "Fleeing"),
	Investigating UMETA(DisplayName = "Investigating"),
	UsingTask	UMETA(DisplayName = "Using Task"),
	Dead		UMETA(DisplayName = "Dead")
};

/**
 * Need Type - different needs an NPC can have
 */
UENUM(BlueprintType)
enum class ELyraNPCNeedType : uint8
{
	Hunger		UMETA(DisplayName = "Hunger"),
	Energy		UMETA(DisplayName = "Energy/Rest"),
	Social		UMETA(DisplayName = "Social"),
	Safety		UMETA(DisplayName = "Safety"),
	Comfort		UMETA(DisplayName = "Comfort"),
	Entertainment UMETA(DisplayName = "Entertainment"),
	Hygiene		UMETA(DisplayName = "Hygiene"),
	Purpose		UMETA(DisplayName = "Purpose/Work"),
	MAX			UMETA(Hidden)
};

/**
 * Relationship Type
 */
UENUM(BlueprintType)
enum class ELyraNPCRelationshipType : uint8
{
	Stranger	UMETA(DisplayName = "Stranger"),
	Acquaintance UMETA(DisplayName = "Acquaintance"),
	Friend		UMETA(DisplayName = "Friend"),
	CloseFriend	UMETA(DisplayName = "Close Friend"),
	Family		UMETA(DisplayName = "Family"),
	Romantic	UMETA(DisplayName = "Romantic Partner"),
	Rival		UMETA(DisplayName = "Rival"),
	Enemy		UMETA(DisplayName = "Enemy"),
	Employer	UMETA(DisplayName = "Employer"),
	Employee	UMETA(DisplayName = "Employee"),
	Custom		UMETA(DisplayName = "Custom")
};

/**
 * Emotion State
 */
UENUM(BlueprintType)
enum class ELyraNPCEmotion : uint8
{
	Neutral		UMETA(DisplayName = "Neutral"),
	Happy		UMETA(DisplayName = "Happy"),
	Sad			UMETA(DisplayName = "Sad"),
	Angry		UMETA(DisplayName = "Angry"),
	Fearful		UMETA(DisplayName = "Fearful"),
	Surprised	UMETA(DisplayName = "Surprised"),
	Disgusted	UMETA(DisplayName = "Disgusted"),
	Excited		UMETA(DisplayName = "Excited"),
	Bored		UMETA(DisplayName = "Bored"),
	Anxious		UMETA(DisplayName = "Anxious"),
	Content		UMETA(DisplayName = "Content")
};

/**
 * Alert Level - awareness state
 */
UENUM(BlueprintType)
enum class ELyraNPCAlertLevel : uint8
{
	Unaware		UMETA(DisplayName = "Unaware", ToolTip = "No threats detected"),
	Curious		UMETA(DisplayName = "Curious", ToolTip = "Something caught attention"),
	Suspicious	UMETA(DisplayName = "Suspicious", ToolTip = "Potential threat detected"),
	Alert		UMETA(DisplayName = "Alert", ToolTip = "Confirmed threat, preparing response"),
	Combat		UMETA(DisplayName = "Combat", ToolTip = "Actively engaged in combat")
};

/**
 * Task Priority
 */
UENUM(BlueprintType)
enum class ELyraNPCTaskPriority : uint8
{
	Critical	UMETA(DisplayName = "Critical", ToolTip = "Life-threatening, must do immediately"),
	High		UMETA(DisplayName = "High", ToolTip = "Important, should do soon"),
	Normal		UMETA(DisplayName = "Normal", ToolTip = "Standard priority"),
	Low			UMETA(DisplayName = "Low", ToolTip = "Can wait"),
	Optional	UMETA(DisplayName = "Optional", ToolTip = "Nice to have")
};

/**
 * Movement Style - how the NPC moves
 */
UENUM(BlueprintType)
enum class ELyraNPCMovementStyle : uint8
{
	Walk		UMETA(DisplayName = "Walk"),
	Jog			UMETA(DisplayName = "Jog"),
	Run			UMETA(DisplayName = "Run"),
	Sprint		UMETA(DisplayName = "Sprint"),
	Sneak		UMETA(DisplayName = "Sneak"),
	Limp		UMETA(DisplayName = "Limp"),
	Drunk		UMETA(DisplayName = "Drunk/Stumble")
};

/**
 * Schedule Time Block
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCScheduleBlock
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float StartHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float EndHour = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FGameplayTag ActivityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	FName LocationName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	ELyraNPCTaskPriority Priority = ELyraNPCTaskPriority::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	bool bMandatory = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	float FlexibilityMinutes = 30.0f;
};

/**
 * Single Need State
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCNeedState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
	ELyraNPCNeedType NeedType = ELyraNPCNeedType::Hunger;

	// Current value 0-100 (100 = fully satisfied)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float CurrentValue = 100.0f;

	// Rate of decay per hour
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
	float DecayRatePerHour = 4.0f;

	// Priority weight when selecting actions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
	float PriorityWeight = 1.0f;

	// Below this threshold, need becomes urgent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
	float UrgentThreshold = 25.0f;

	// Below this threshold, need becomes critical
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
	float CriticalThreshold = 10.0f;
};

/**
 * Relationship Entry
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCRelationship
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	TWeakObjectPtr<ALyraNPCCharacter> OtherNPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	FGuid OtherNPCId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	ELyraNPCRelationshipType RelationshipType = ELyraNPCRelationshipType::Stranger;

	// -100 to 100, where -100 is hatred and 100 is love
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float Affinity = 0.0f;

	// 0 to 100, how well they know this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Familiarity = 0.0f;

	// 0 to 100, how much they trust this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float Trust = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
	float LastInteractionTime = 0.0f;
};

/**
 * Memory Entry - something the NPC remembers
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCMemory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	FGameplayTag MemoryType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	FString Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float Timestamp = 0.0f;

	// 0 to 100, how important this memory is
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float Importance = 50.0f;

	// 0 to 100, how well they remember it (decays over time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	float Clarity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
	TWeakObjectPtr<AActor> RelatedActor;
};

/**
 * Personality Traits
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCPersonality
{
	GENERATED_BODY()

	// Big Five Personality Traits (0-1 scale)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Openness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Conscientiousness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Extraversion = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Agreeableness = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Neuroticism = 0.5f;

	// Additional traits
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Bravery = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Honesty = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Curiosity = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Patience = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Loyalty = 0.5f;
};

/**
 * NPC Biography
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCBiography
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FGuid UniqueId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString FirstName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString LastName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString Nickname;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	int32 Age = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString Occupation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString Origin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FString Backstory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	ELyraNPCArchetype Archetype = ELyraNPCArchetype::Villager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FLyraNPCPersonality Personality;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biography")
	FGameplayTagContainer CharacterTags;

	FString GetFullName() const { return FirstName + TEXT(" ") + LastName; }
	FString GetDisplayName() const { return Nickname.IsEmpty() ? FirstName : Nickname; }
};

/**
 * Task Usage Info
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCTaskUsageInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	TWeakObjectPtr<ULyraNPCTaskActor> TaskActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float StartTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	float PlannedDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bIsReserved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Task")
	bool bIsActive = false;
};

/**
 * Path Point for predetermined movement
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCPathPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	FRotator Rotation = FRotator::ZeroRotator;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	float WaitTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	ELyraNPCMovementStyle MovementStyle = ELyraNPCMovementStyle::Walk;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	FGameplayTag ActionAtPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
	float AcceptanceRadius = 50.0f;
};

/**
 * Combat Stats
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCCombatStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Defense = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Accuracy = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DodgeChance = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BlockChance = 0.2f;
};

/**
 * Perception Result
 */
USTRUCT(BlueprintType)
struct LYRANPC_API FLyraNPCPerceptionResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	TWeakObjectPtr<AActor> PerceivedActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	FVector LastKnownLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float LastSeenTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float ThreatLevel = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool bIsVisible = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	bool bWasHeard = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float Confidence = 1.0f;
};

// Delegate Declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCLifeStateChanged, ALyraNPCCharacter*, NPC, ELyraNPCLifeState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCNeedCritical, ALyraNPCCharacter*, NPC, ELyraNPCNeedType, NeedType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCAILODChanged, ALyraNPCCharacter*, NPC, ELyraNPCAILOD, NewLOD);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCAlertLevelChanged, ALyraNPCCharacter*, NPC, ELyraNPCAlertLevel, NewAlertLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNPCTaskStarted, ALyraNPCCharacter*, NPC, ULyraNPCTaskActor*, Task, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPCTaskCompleted, ALyraNPCCharacter*, NPC, ULyraNPCTaskActor*, Task);
