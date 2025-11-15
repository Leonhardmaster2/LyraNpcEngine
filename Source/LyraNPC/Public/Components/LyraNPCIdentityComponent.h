// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCIdentityComponent.generated.h"

/**
 * Component that manages NPC identity, biography, and personality.
 * Add this to your NPC character for full biography support.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Identity"))
class LYRANPC_API ULyraNPCIdentityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCIdentityComponent();

	// Biography Data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity", Replicated)
	FLyraNPCBiography Biography;

	// Current emotional state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity|State", Replicated)
	ELyraNPCEmotion CurrentEmotion = ELyraNPCEmotion::Neutral;

	// Current life state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity|State", Replicated)
	ELyraNPCLifeState CurrentLifeState = ELyraNPCLifeState::Idle;

	// Home location in the world
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity|World")
	FVector HomeLocation = FVector::ZeroVector;

	// Workplace location
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity|World")
	FVector WorkplaceLocation = FVector::ZeroVector;

	// Favorite places (tagged locations)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity|World")
	TMap<FGameplayTag, FVector> FavoritePlaces;

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Identity|Events")
	FOnNPCLifeStateChanged OnLifeStateChanged;

public:
	// Initialization
	UFUNCTION(BlueprintCallable, Category = "Identity")
	void InitializeIdentity(const FLyraNPCBiography& NewBiography);

	UFUNCTION(BlueprintCallable, Category = "Identity")
	void GenerateRandomIdentity(ELyraNPCArchetype Archetype = ELyraNPCArchetype::Villager);

	// Getters
	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetDisplayName() const { return Biography.GetDisplayName(); }

	UFUNCTION(BlueprintPure, Category = "Identity")
	FString GetFullName() const { return Biography.GetFullName(); }

	UFUNCTION(BlueprintPure, Category = "Identity")
	ELyraNPCArchetype GetArchetype() const { return Biography.Archetype; }

	UFUNCTION(BlueprintPure, Category = "Identity")
	FLyraNPCPersonality GetPersonality() const { return Biography.Personality; }

	UFUNCTION(BlueprintPure, Category = "Identity")
	FGuid GetUniqueId() const { return Biography.UniqueId; }

	// State Management
	UFUNCTION(BlueprintCallable, Category = "Identity")
	void SetLifeState(ELyraNPCLifeState NewState);

	UFUNCTION(BlueprintCallable, Category = "Identity")
	void SetEmotion(ELyraNPCEmotion NewEmotion);

	// Personality-based decision helpers
	UFUNCTION(BlueprintPure, Category = "Identity|Personality")
	float GetDecisionModifier(FGameplayTag DecisionType) const;

	UFUNCTION(BlueprintPure, Category = "Identity|Personality")
	bool WouldMakeRiskyDecision(float RiskLevel) const;

	UFUNCTION(BlueprintPure, Category = "Identity|Personality")
	bool WouldHelpStranger() const;

	UFUNCTION(BlueprintPure, Category = "Identity|Personality")
	bool PrefersGroup() const;

	UFUNCTION(BlueprintPure, Category = "Identity|Personality")
	float GetStressResistance() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// Random name generation data
	static TArray<FString> FirstNames_Male;
	static TArray<FString> FirstNames_Female;
	static TArray<FString> LastNames;
	static TArray<FString> Occupations;

	void InitializeNameData();
};
