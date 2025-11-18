// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCCharacter.generated.h"

class ULyraNPCIdentityComponent;
class ULyraNPCCognitiveComponent;
class ULyraNPCNeedsComponent;
class ULyraNPCScheduleComponent;
class ULyraNPCPathFollowingComponent;
class ULyraNPCSocialComponent;
class ULyraNPCGOAPComponent;

/**
 * Base character class for LyraNPC.
 * This character comes pre-configured with all necessary components.
 * Simply place this in your world or inherit from it for custom NPCs.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(DisplayName="LyraNPC Character"))
class LYRANPC_API ALyraNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ALyraNPCCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ===== CORE NPC COMPONENTS =====

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCIdentityComponent> IdentityComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCCognitiveComponent> CognitiveComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCNeedsComponent> NeedsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCScheduleComponent> ScheduleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCPathFollowingComponent> PathFollowingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCSocialComponent> SocialComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LyraNPC|Components")
	TObjectPtr<ULyraNPCGOAPComponent> GOAPComponent;

	// ===== CONFIGURATION =====

	// Initial archetype for this NPC
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Setup")
	ELyraNPCArchetype InitialArchetype = ELyraNPCArchetype::Villager;

	// Initial cognitive skill level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Setup", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float InitialCognitiveSkill = 0.5f;

	// Auto-initialize components on begin play
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Setup")
	bool bAutoInitialize = true;

	// Combat stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LyraNPC|Combat", Replicated)
	FLyraNPCCombatStats CombatStats;

public:
	// ===== LIFECYCLE =====

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ===== INITIALIZATION =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Setup")
	void InitializeNPC();

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Setup")
	void InitializeFromData(const FLyraNPCBiography& Biography, float CognitiveSkillLevel);

	// ===== QUICK ACCESS FUNCTIONS =====

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Identity")
	FString GetNPCName() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Identity")
	ELyraNPCArchetype GetArchetype() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|State")
	ELyraNPCLifeState GetLifeState() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|State")
	ELyraNPCAlertLevel GetAlertLevel() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Cognitive")
	float GetCognitiveSkill() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Needs")
	float GetOverallWellbeing() const;

	// ===== COMBAT =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Combat")
	void TakeDamage(float DamageAmount, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Combat")
	void Heal(float HealAmount);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Combat")
	bool IsAlive() const;

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Combat")
	float GetHealthPercent() const;

	// ===== MOVEMENT STYLE =====

	UFUNCTION(BlueprintCallable, Category = "LyraNPC|Movement")
	void SetMovementStyle(ELyraNPCMovementStyle Style);

	UFUNCTION(BlueprintPure, Category = "LyraNPC|Movement")
	float GetMovementSpeedForStyle(ELyraNPCMovementStyle Style) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnDeath();

private:
	void ApplyCognitiveSkillToMovement();
};
