// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCNeedsComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

ULyraNPCNeedsComponent::ULyraNPCNeedsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f; // Update every second
	SetIsReplicatedByDefault(true);
}

void ULyraNPCNeedsComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Needs.Num() == 0)
	{
		InitializeDefaultNeeds(ELyraNPCArchetype::Villager);
	}
}

void ULyraNPCNeedsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULyraNPCNeedsComponent, Needs);
}

void ULyraNPCNeedsComponent::InitializeDefaultNeeds(ELyraNPCArchetype Archetype)
{
	Needs.Empty();

	// Hunger - everyone needs to eat
	FLyraNPCNeedState Hunger;
	Hunger.NeedType = ELyraNPCNeedType::Hunger;
	Hunger.CurrentValue = FMath::FRandRange(60.0f, 100.0f);
	Hunger.DecayRatePerHour = 4.0f;
	Hunger.PriorityWeight = 1.2f;
	Hunger.UrgentThreshold = 25.0f;
	Hunger.CriticalThreshold = 10.0f;
	Needs.Add(Hunger);

	// Energy - everyone needs rest
	FLyraNPCNeedState Energy;
	Energy.NeedType = ELyraNPCNeedType::Energy;
	Energy.CurrentValue = FMath::FRandRange(70.0f, 100.0f);
	Energy.DecayRatePerHour = 6.0f;
	Energy.PriorityWeight = 1.3f;
	Energy.UrgentThreshold = 20.0f;
	Energy.CriticalThreshold = 5.0f;
	Needs.Add(Energy);

	// Social - varies by archetype
	FLyraNPCNeedState Social;
	Social.NeedType = ELyraNPCNeedType::Social;
	Social.CurrentValue = FMath::FRandRange(50.0f, 100.0f);
	Social.DecayRatePerHour = Archetype == ELyraNPCArchetype::Traveler ? 1.0f : 3.0f;
	Social.PriorityWeight = 0.8f;
	Social.UrgentThreshold = 20.0f;
	Social.CriticalThreshold = 5.0f;
	Needs.Add(Social);

	// Safety
	FLyraNPCNeedState Safety;
	Safety.NeedType = ELyraNPCNeedType::Safety;
	Safety.CurrentValue = 100.0f;
	Safety.DecayRatePerHour = 0.0f; // Only decays from events
	Safety.PriorityWeight = 2.0f; // High priority when low
	Safety.UrgentThreshold = 50.0f;
	Safety.CriticalThreshold = 25.0f;
	Needs.Add(Safety);

	// Comfort
	FLyraNPCNeedState Comfort;
	Comfort.NeedType = ELyraNPCNeedType::Comfort;
	Comfort.CurrentValue = FMath::FRandRange(60.0f, 100.0f);
	Comfort.DecayRatePerHour = 2.0f;
	Comfort.PriorityWeight = 0.6f;
	Comfort.UrgentThreshold = 30.0f;
	Comfort.CriticalThreshold = 10.0f;
	Needs.Add(Comfort);

	// Entertainment
	FLyraNPCNeedState Entertainment;
	Entertainment.NeedType = ELyraNPCNeedType::Entertainment;
	Entertainment.CurrentValue = FMath::FRandRange(40.0f, 100.0f);
	Entertainment.DecayRatePerHour = 2.5f;
	Entertainment.PriorityWeight = 0.5f;
	Entertainment.UrgentThreshold = 15.0f;
	Entertainment.CriticalThreshold = 5.0f;
	Needs.Add(Entertainment);

	// Purpose/Work - especially important for workers
	FLyraNPCNeedState Purpose;
	Purpose.NeedType = ELyraNPCNeedType::Purpose;
	Purpose.CurrentValue = FMath::FRandRange(50.0f, 100.0f);
	Purpose.DecayRatePerHour = Archetype == ELyraNPCArchetype::Worker ? 4.0f : 2.0f;
	Purpose.PriorityWeight = Archetype == ELyraNPCArchetype::Worker ? 1.0f : 0.7f;
	Purpose.UrgentThreshold = 25.0f;
	Purpose.CriticalThreshold = 10.0f;
	Needs.Add(Purpose);

	UE_LOG(LogLyraNPC, Log, TEXT("Initialized %d needs for archetype %d"), Needs.Num(), static_cast<int32>(Archetype));
}

void ULyraNPCNeedsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bSimulateNeeds)
	{
		UpdateNeeds(DeltaTime);
		CheckCriticalNeeds();
	}
}

void ULyraNPCNeedsComponent::UpdateNeeds(float DeltaTime)
{
	// Convert delta time to game hours
	float GameHoursPassed = (DeltaTime / 3600.0f) * TimeScale;

	for (FLyraNPCNeedState& Need : Needs)
	{
		float DecayAmount = Need.DecayRatePerHour * GameHoursPassed;
		Need.CurrentValue = FMath::Max(0.0f, Need.CurrentValue - DecayAmount);
	}
}

void ULyraNPCNeedsComponent::CheckCriticalNeeds()
{
	for (const FLyraNPCNeedState& Need : Needs)
	{
		if (Need.CurrentValue <= Need.CriticalThreshold)
		{
			if (AActor* Owner = GetOwner())
			{
				if (ALyraNPCCharacter* NPCChar = Cast<ALyraNPCCharacter>(Owner))
				{
					OnNeedCritical.Broadcast(NPCChar, Need.NeedType);
				}
			}
		}
	}
}

FLyraNPCNeedState ULyraNPCNeedsComponent::GetNeed(ELyraNPCNeedType NeedType) const
{
	const FLyraNPCNeedState* Found = FindNeed(NeedType);
	return Found ? *Found : FLyraNPCNeedState();
}

float ULyraNPCNeedsComponent::GetNeedValue(ELyraNPCNeedType NeedType) const
{
	const FLyraNPCNeedState* Found = FindNeed(NeedType);
	return Found ? Found->CurrentValue : 100.0f;
}

void ULyraNPCNeedsComponent::SetNeedValue(ELyraNPCNeedType NeedType, float NewValue)
{
	if (FLyraNPCNeedState* Found = FindNeed(NeedType))
	{
		Found->CurrentValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
}

void ULyraNPCNeedsComponent::ModifyNeed(ELyraNPCNeedType NeedType, float Delta)
{
	if (FLyraNPCNeedState* Found = FindNeed(NeedType))
	{
		Found->CurrentValue = FMath::Clamp(Found->CurrentValue + Delta, 0.0f, 100.0f);
	}
}

void ULyraNPCNeedsComponent::SatisfyNeed(ELyraNPCNeedType NeedType, float Amount)
{
	ModifyNeed(NeedType, Amount);
}

bool ULyraNPCNeedsComponent::HasCriticalNeed() const
{
	for (const FLyraNPCNeedState& Need : Needs)
	{
		if (Need.CurrentValue <= Need.CriticalThreshold)
		{
			return true;
		}
	}
	return false;
}

bool ULyraNPCNeedsComponent::HasUrgentNeed() const
{
	for (const FLyraNPCNeedState& Need : Needs)
	{
		if (Need.CurrentValue <= Need.UrgentThreshold)
		{
			return true;
		}
	}
	return false;
}

ELyraNPCNeedType ULyraNPCNeedsComponent::GetMostUrgentNeed() const
{
	ELyraNPCNeedType MostUrgent = ELyraNPCNeedType::Hunger;
	float HighestPriority = -1.0f;

	for (const FLyraNPCNeedState& Need : Needs)
	{
		float Priority = GetNeedPriority(Need.NeedType);
		if (Priority > HighestPriority)
		{
			HighestPriority = Priority;
			MostUrgent = Need.NeedType;
		}
	}

	return MostUrgent;
}

float ULyraNPCNeedsComponent::GetOverallWellbeing() const
{
	if (Needs.Num() == 0) return 100.0f;

	float TotalValue = 0.0f;
	float TotalWeight = 0.0f;

	for (const FLyraNPCNeedState& Need : Needs)
	{
		TotalValue += Need.CurrentValue * Need.PriorityWeight;
		TotalWeight += Need.PriorityWeight;
	}

	return TotalWeight > 0.0f ? TotalValue / TotalWeight : 100.0f;
}

float ULyraNPCNeedsComponent::GetNeedPriority(ELyraNPCNeedType NeedType) const
{
	const FLyraNPCNeedState* Found = FindNeed(NeedType);
	if (!Found) return 0.0f;

	// Priority increases as need decreases (inverse relationship)
	float Deficit = 100.0f - Found->CurrentValue;

	// Apply urgency multipliers
	float UrgencyMultiplier = 1.0f;
	if (Found->CurrentValue <= Found->CriticalThreshold)
	{
		UrgencyMultiplier = 3.0f;
	}
	else if (Found->CurrentValue <= Found->UrgentThreshold)
	{
		UrgencyMultiplier = 2.0f;
	}

	return Deficit * Found->PriorityWeight * UrgencyMultiplier;
}

TArray<ELyraNPCNeedType> ULyraNPCNeedsComponent::GetNeedsBelowThreshold(float Threshold) const
{
	TArray<ELyraNPCNeedType> Result;

	for (const FLyraNPCNeedState& Need : Needs)
	{
		if (Need.CurrentValue < Threshold)
		{
			Result.Add(Need.NeedType);
		}
	}

	return Result;
}

FLyraNPCNeedState* ULyraNPCNeedsComponent::FindNeed(ELyraNPCNeedType NeedType)
{
	for (FLyraNPCNeedState& Need : Needs)
	{
		if (Need.NeedType == NeedType)
		{
			return &Need;
		}
	}
	return nullptr;
}

const FLyraNPCNeedState* ULyraNPCNeedsComponent::FindNeed(ELyraNPCNeedType NeedType) const
{
	for (const FLyraNPCNeedState& Need : Needs)
	{
		if (Need.NeedType == NeedType)
		{
			return &Need;
		}
	}
	return nullptr;
}
