// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCCognitiveComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

ULyraNPCCognitiveComponent::ULyraNPCCognitiveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
	SetIsReplicatedByDefault(true);
}

void ULyraNPCCognitiveComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize memory capacity based on intelligence
	MaxMemories = FMath::RoundToInt(30.0f + (CognitiveSkill * 70.0f)); // 30-100 memories
	MemoryDecayRate = 3.0f - (CognitiveSkill * 2.0f); // 1-3 decay rate

	// Decision variance is inversely related to intelligence
	DecisionVariance = 0.4f - (CognitiveSkill * 0.35f); // 0.05-0.4 variance
}

void ULyraNPCCognitiveComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULyraNPCCognitiveComponent, AlertLevel);
}

void ULyraNPCCognitiveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateAlertness(DeltaTime);
	UpdateMemoryDecay(DeltaTime);
}

void ULyraNPCCognitiveComponent::UpdateAlertness(float DeltaTime)
{
	TimeSinceLastAlertChange += DeltaTime;

	// Decay alertness over time if not in combat
	if (AlertLevel != ELyraNPCAlertLevel::Combat && AlertLevel != ELyraNPCAlertLevel::Unaware)
	{
		CurrentAlertness -= AlertnessDecayRate * DeltaTime;
		CurrentAlertness = FMath::Max(0.0f, CurrentAlertness);

		// Update alert level based on current alertness
		if (CurrentAlertness < 0.1f && TimeSinceLastAlertChange > 10.0f)
		{
			SetAlertLevel(ELyraNPCAlertLevel::Unaware);
		}
		else if (CurrentAlertness < 0.3f && AlertLevel > ELyraNPCAlertLevel::Curious)
		{
			SetAlertLevel(ELyraNPCAlertLevel::Curious);
		}
		else if (CurrentAlertness < 0.6f && AlertLevel > ELyraNPCAlertLevel::Suspicious)
		{
			SetAlertLevel(ELyraNPCAlertLevel::Suspicious);
		}
	}
}

void ULyraNPCCognitiveComponent::UpdateMemoryDecay(float DeltaTime)
{
	// Update memory decay every few seconds
	static float MemoryUpdateAccumulator = 0.0f;
	MemoryUpdateAccumulator += DeltaTime;

	if (MemoryUpdateAccumulator >= 5.0f)
	{
		MemoryUpdateAccumulator = 0.0f;
		ForgetOldMemories();
	}
}

// ===== PERCEPTION MODIFIERS =====

float ULyraNPCCognitiveComponent::GetPerceptionRadiusModifier() const
{
	// Smart NPCs have 120% base perception, dumb ones have 70%
	return 0.7f + (CognitiveSkill * 0.5f);
}

float ULyraNPCCognitiveComponent::GetSightAngleModifier() const
{
	// Smart NPCs have better peripheral vision
	return 0.8f + (CognitiveSkill * 0.4f);
}

float ULyraNPCCognitiveComponent::GetHearingModifier() const
{
	// Hearing is less affected by intelligence, but still matters
	return 0.85f + (CognitiveSkill * 0.3f);
}

float ULyraNPCCognitiveComponent::GetNoticeChance(float BaseDifficulty) const
{
	float AlertnessBonus = static_cast<float>(AlertLevel) * 0.15f;
	float IntelligenceBonus = CognitiveSkill * 0.4f;
	float ChanceToNotice = (1.0f - BaseDifficulty) + IntelligenceBonus + AlertnessBonus;
	return FMath::Clamp(ChanceToNotice, 0.05f, 0.95f);
}

// ===== MOVEMENT MODIFIERS =====

float ULyraNPCCognitiveComponent::GetPathAccuracy() const
{
	// High intelligence = straighter paths, low = wobble
	return 0.6f + (CognitiveSkill * 0.4f);
}

float ULyraNPCCognitiveComponent::GetObstacleAvoidanceQuality() const
{
	return 0.5f + (CognitiveSkill * 0.5f);
}

float ULyraNPCCognitiveComponent::GetStressedMovementModifier() const
{
	// Dumb NPCs panic more and move erratically under stress
	float StressImpact = 1.0f - CognitiveSkill;
	if (AlertLevel >= ELyraNPCAlertLevel::Alert)
	{
		return 1.0f - (StressImpact * 0.3f); // Up to 30% slower when panicked
	}
	return 1.0f;
}

// ===== COMBAT MODIFIERS =====

float ULyraNPCCognitiveComponent::GetCombatAccuracyModifier() const
{
	// Base 50% accuracy for dumb NPCs, up to 95% for smart ones
	float BaseAccuracy = 0.5f + (CognitiveSkill * 0.45f);

	// Reduce accuracy when stressed (dumb NPCs panic more)
	if (AlertLevel == ELyraNPCAlertLevel::Combat)
	{
		float PanicPenalty = (1.0f - CognitiveSkill) * 0.2f;
		BaseAccuracy -= PanicPenalty;
	}

	return FMath::Clamp(BaseAccuracy, 0.3f, 0.95f);
}

float ULyraNPCCognitiveComponent::GetReactionTimeModifier() const
{
	// Lower is faster. Dumb NPCs react 40% slower
	return 1.4f - (CognitiveSkill * 0.4f);
}

float ULyraNPCCognitiveComponent::GetTacticalQuality() const
{
	return CognitiveSkill;
}

bool ULyraNPCCognitiveComponent::CanUseFlankingTactics() const
{
	return CognitiveSkill >= 0.6f;
}

bool ULyraNPCCognitiveComponent::CanCoordinateWithAllies() const
{
	return CognitiveSkill >= 0.5f;
}

// ===== DECISION MAKING =====

bool ULyraNPCCognitiveComponent::WillRememberTask(float TaskImportance) const
{
	// Dumb NPCs forget tasks more often
	float RememberChance = CognitiveSkill * 0.6f + TaskImportance * 0.4f;
	return FMath::FRand() < RememberChance;
}

float ULyraNPCCognitiveComponent::GetDecisionQuality() const
{
	// Apply some randomness but smarter NPCs make better decisions
	float BaseQuality = 0.5f + (CognitiveSkill * 0.5f);
	float Variance = FMath::FRandRange(-DecisionVariance, DecisionVariance);
	return FMath::Clamp(BaseQuality + Variance, 0.1f, 1.0f);
}

float ULyraNPCCognitiveComponent::ApplyIntelligenceVariance(float BaseScore) const
{
	// For utility AI scoring - smart NPCs pick better options
	float Variance = FMath::FRandRange(-DecisionVariance, DecisionVariance);
	return BaseScore + (Variance * BaseScore);
}

int32 ULyraNPCCognitiveComponent::GetPlanningDepth() const
{
	// Dumb NPCs only think 1 step ahead, smart ones think 3-5 steps
	if (CognitiveSkill < 0.3f) return 1;
	if (CognitiveSkill < 0.5f) return 2;
	if (CognitiveSkill < 0.7f) return 3;
	if (CognitiveSkill < 0.9f) return 4;
	return 5;
}

// ===== ALERTNESS MANAGEMENT =====

void ULyraNPCCognitiveComponent::SetAlertLevel(ELyraNPCAlertLevel NewLevel)
{
	if (AlertLevel != NewLevel)
	{
		ELyraNPCAlertLevel OldLevel = AlertLevel;
		AlertLevel = NewLevel;
		TimeSinceLastAlertChange = 0.0f;

		// Set current alertness to match
		CurrentAlertness = static_cast<float>(NewLevel) * 0.25f;

		if (AActor* Owner = GetOwner())
		{
			if (ALyraNPCCharacter* NPCChar = Cast<ALyraNPCCharacter>(Owner))
			{
				OnAlertLevelChanged.Broadcast(NPCChar, NewLevel);
			}
		}

		UE_LOG(LogLyraNPC, Verbose, TEXT("Alert Level changed from %d to %d"),
			static_cast<int32>(OldLevel), static_cast<int32>(NewLevel));
	}
}

void ULyraNPCCognitiveComponent::IncreaseAlertness(float Amount)
{
	CurrentAlertness = FMath::Clamp(CurrentAlertness + Amount, 0.0f, 1.0f);

	// Update alert level based on new alertness
	if (CurrentAlertness >= 0.9f)
	{
		SetAlertLevel(ELyraNPCAlertLevel::Combat);
	}
	else if (CurrentAlertness >= 0.6f)
	{
		SetAlertLevel(ELyraNPCAlertLevel::Alert);
	}
	else if (CurrentAlertness >= 0.4f)
	{
		SetAlertLevel(ELyraNPCAlertLevel::Suspicious);
	}
	else if (CurrentAlertness >= 0.2f)
	{
		SetAlertLevel(ELyraNPCAlertLevel::Curious);
	}
}

// ===== MEMORY SYSTEM =====

void ULyraNPCCognitiveComponent::AddMemory(const FLyraNPCMemory& NewMemory)
{
	// Check if we should remember this based on intelligence
	if (!WillRememberTask(NewMemory.Importance / 100.0f))
	{
		UE_LOG(LogLyraNPC, Verbose, TEXT("NPC forgot to remember: %s"), *NewMemory.Description);
		return;
	}

	Memories.Add(NewMemory);
	CleanupMemories();

	UE_LOG(LogLyraNPC, Verbose, TEXT("Memory added: %s (Importance: %.1f)"),
		*NewMemory.Description, NewMemory.Importance);
}

void ULyraNPCCognitiveComponent::AddSimpleMemory(FGameplayTag MemoryType, const FString& Description, FVector Location, float Importance)
{
	FLyraNPCMemory NewMemory;
	NewMemory.MemoryType = MemoryType;
	NewMemory.Description = Description;
	NewMemory.Location = Location;
	NewMemory.Importance = Importance;
	NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
	NewMemory.Clarity = 100.0f;

	AddMemory(NewMemory);
}

bool ULyraNPCCognitiveComponent::HasMemoryOfType(FGameplayTag MemoryType) const
{
	for (const FLyraNPCMemory& Memory : Memories)
	{
		if (Memory.MemoryType.MatchesTag(MemoryType) && Memory.Clarity > 10.0f)
		{
			return true;
		}
	}
	return false;
}

FLyraNPCMemory ULyraNPCCognitiveComponent::GetMostRecentMemory(FGameplayTag MemoryType) const
{
	FLyraNPCMemory MostRecent;
	float MostRecentTime = -1.0f;

	for (const FLyraNPCMemory& Memory : Memories)
	{
		if (Memory.MemoryType.MatchesTag(MemoryType) && Memory.Timestamp > MostRecentTime)
		{
			MostRecent = Memory;
			MostRecentTime = Memory.Timestamp;
		}
	}

	return MostRecent;
}

TArray<FLyraNPCMemory> ULyraNPCCognitiveComponent::GetMemoriesNearLocation(FVector Location, float Radius) const
{
	TArray<FLyraNPCMemory> NearbyMemories;

	for (const FLyraNPCMemory& Memory : Memories)
	{
		if (FVector::Dist(Memory.Location, Location) <= Radius && Memory.Clarity > 10.0f)
		{
			NearbyMemories.Add(Memory);
		}
	}

	return NearbyMemories;
}

void ULyraNPCCognitiveComponent::ForgetOldMemories()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = Memories.Num() - 1; i >= 0; --i)
	{
		FLyraNPCMemory& Memory = Memories[i];
		float TimePassed = (CurrentTime - Memory.Timestamp) / 3600.0f; // Convert to hours

		// Decay clarity based on time and memory decay rate
		float DecayAmount = TimePassed * MemoryDecayRate * (1.0f - (Memory.Importance / 200.0f));
		Memory.Clarity -= DecayAmount;

		// Remove completely forgotten memories
		if (Memory.Clarity <= 0.0f)
		{
			UE_LOG(LogLyraNPC, Verbose, TEXT("Memory forgotten: %s"), *Memory.Description);
			Memories.RemoveAt(i);
		}
	}

	CleanupMemories();
}

void ULyraNPCCognitiveComponent::CleanupMemories()
{
	// Remove oldest, least important memories if over capacity
	while (Memories.Num() > MaxMemories)
	{
		int32 LeastImportantIndex = 0;
		float LowestScore = MAX_FLT;

		for (int32 i = 0; i < Memories.Num(); ++i)
		{
			// Score = Importance * Clarity / Time
			float Score = Memories[i].Importance * Memories[i].Clarity;
			if (Score < LowestScore)
			{
				LowestScore = Score;
				LeastImportantIndex = i;
			}
		}

		Memories.RemoveAt(LeastImportantIndex);
	}
}

// ===== MISTAKE SYSTEM =====

bool ULyraNPCCognitiveComponent::WillMakeMistake(float ActionDifficulty) const
{
	// Base mistake chance inversely related to intelligence
	float BaseMistakeChance = (1.0f - CognitiveSkill) * 0.5f;

	// Increase mistake chance based on difficulty
	float DifficultyFactor = ActionDifficulty * 0.3f;

	// Stress increases mistakes for dumb NPCs
	float StressFactor = 0.0f;
	if (AlertLevel >= ELyraNPCAlertLevel::Alert)
	{
		StressFactor = (1.0f - CognitiveSkill) * 0.2f;
	}

	float TotalMistakeChance = BaseMistakeChance + DifficultyFactor + StressFactor;
	return FMath::FRand() < TotalMistakeChance;
}

float ULyraNPCCognitiveComponent::GetMistakeMagnitude() const
{
	// Dumb NPCs make bigger mistakes
	float MaxMagnitude = 1.0f - CognitiveSkill;
	return FMath::FRandRange(0.0f, MaxMagnitude);
}

FVector ULyraNPCCognitiveComponent::ApplyLocationError(FVector TargetLocation, float MaxErrorDistance) const
{
	if (WillMakeMistake(0.3f))
	{
		float ErrorMagnitude = GetMistakeMagnitude() * MaxErrorDistance;
		FVector RandomOffset = FMath::VRand() * ErrorMagnitude;
		return TargetLocation + RandomOffset;
	}
	return TargetLocation;
}
