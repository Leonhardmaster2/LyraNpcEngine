// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCSocialComponent.generated.h"

class ALyraNPCCharacter;

/**
 * Component that manages NPC relationships and social interactions.
 * Tracks friendships, rivalries, family bonds, and social history.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Social"))
class LYRANPC_API ULyraNPCSocialComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCSocialComponent();

	// All relationships this NPC has
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social", Replicated)
	TArray<FLyraNPCRelationship> Relationships;

	// Social interaction cooldown (seconds between social actions)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social|Settings")
	float SocialCooldown = 300.0f;

	// How quickly affinity changes from interactions
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social|Settings")
	float AffinityChangeRate = 1.0f;

	// Maximum number of close friends
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social|Settings")
	int32 MaxCloseFriends = 5;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== RELATIONSHIP MANAGEMENT =====

	UFUNCTION(BlueprintCallable, Category = "Social|Relationships")
	void AddRelationship(ALyraNPCCharacter* OtherNPC, ELyraNPCRelationshipType Type = ELyraNPCRelationshipType::Stranger);

	UFUNCTION(BlueprintCallable, Category = "Social|Relationships")
	void RemoveRelationship(const FGuid& OtherNPCId);

	UFUNCTION(BlueprintCallable, Category = "Social|Relationships")
	void UpdateRelationship(const FGuid& OtherNPCId, float AffinityDelta, float FamiliarityDelta = 0.0f);

	UFUNCTION(BlueprintCallable, Category = "Social|Relationships")
	void SetRelationshipType(const FGuid& OtherNPCId, ELyraNPCRelationshipType NewType);

	// ===== RELATIONSHIP QUERIES =====

	UFUNCTION(BlueprintPure, Category = "Social|Relationships")
	FLyraNPCRelationship GetRelationship(const FGuid& OtherNPCId) const;

	UFUNCTION(BlueprintPure, Category = "Social|Relationships")
	bool HasRelationship(const FGuid& OtherNPCId) const;

	UFUNCTION(BlueprintPure, Category = "Social|Relationships")
	float GetAffinityWith(const FGuid& OtherNPCId) const;

	UFUNCTION(BlueprintPure, Category = "Social|Relationships")
	float GetTrustLevel(const FGuid& OtherNPCId) const;

	UFUNCTION(BlueprintPure, Category = "Social|Relationships")
	ELyraNPCRelationshipType GetRelationshipType(const FGuid& OtherNPCId) const;

	// ===== SOCIAL QUERIES =====

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	TArray<FGuid> GetFriends() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	TArray<FGuid> GetEnemies() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	TArray<FGuid> GetFamily() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	int32 GetFriendCount() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	FGuid GetBestFriend() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	FGuid GetWorstEnemy() const;

	UFUNCTION(BlueprintPure, Category = "Social|Query")
	float GetAverageSocialStanding() const;

	// ===== SOCIAL INTERACTIONS =====

	UFUNCTION(BlueprintCallable, Category = "Social|Interaction")
	void OnPositiveInteraction(const FGuid& OtherNPCId, float Magnitude = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Social|Interaction")
	void OnNegativeInteraction(const FGuid& OtherNPCId, float Magnitude = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Social|Interaction")
	void OnNeutralInteraction(const FGuid& OtherNPCId);

	// Would this NPC help the other?
	UFUNCTION(BlueprintPure, Category = "Social|Decision")
	bool WouldHelpNPC(const FGuid& OtherNPCId) const;

	// Would this NPC fight alongside the other?
	UFUNCTION(BlueprintPure, Category = "Social|Decision")
	bool WouldFightForNPC(const FGuid& OtherNPCId) const;

	// Would this NPC trust information from the other?
	UFUNCTION(BlueprintPure, Category = "Social|Decision")
	bool WouldTrustNPC(const FGuid& OtherNPCId) const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	FLyraNPCRelationship* FindRelationship(const FGuid& OtherNPCId);
	const FLyraNPCRelationship* FindRelationship(const FGuid& OtherNPCId) const;
	void UpdateRelationshipTypes();
	void DecayRelationships(float DeltaTime);

	float TimeSinceLastDecay = 0.0f;
};
