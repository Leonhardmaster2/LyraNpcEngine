// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCIdentityComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

// Static name data initialization
TArray<FString> ULyraNPCIdentityComponent::FirstNames_Male = {};
TArray<FString> ULyraNPCIdentityComponent::FirstNames_Female = {};
TArray<FString> ULyraNPCIdentityComponent::LastNames = {};
TArray<FString> ULyraNPCIdentityComponent::Occupations = {};

ULyraNPCIdentityComponent::ULyraNPCIdentityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// Generate a unique ID for this NPC
	Biography.UniqueId = FGuid::NewGuid();

	InitializeNameData();
}

void ULyraNPCIdentityComponent::InitializeNameData()
{
	if (FirstNames_Male.Num() == 0)
	{
		FirstNames_Male = {
			TEXT("James"), TEXT("John"), TEXT("Robert"), TEXT("Michael"), TEXT("William"),
			TEXT("David"), TEXT("Richard"), TEXT("Joseph"), TEXT("Thomas"), TEXT("Charles"),
			TEXT("Erik"), TEXT("Marcus"), TEXT("Alexander"), TEXT("Theodore"), TEXT("Sebastian"),
			TEXT("Finn"), TEXT("Oliver"), TEXT("Henry"), TEXT("Arthur"), TEXT("Felix")
		};
	}

	if (FirstNames_Female.Num() == 0)
	{
		FirstNames_Female = {
			TEXT("Mary"), TEXT("Patricia"), TEXT("Jennifer"), TEXT("Linda"), TEXT("Barbara"),
			TEXT("Elizabeth"), TEXT("Susan"), TEXT("Jessica"), TEXT("Sarah"), TEXT("Karen"),
			TEXT("Elena"), TEXT("Sophia"), TEXT("Isabella"), TEXT("Charlotte"), TEXT("Amelia"),
			TEXT("Evelyn"), TEXT("Abigail"), TEXT("Emily"), TEXT("Harper"), TEXT("Aria")
		};
	}

	if (LastNames.Num() == 0)
	{
		LastNames = {
			TEXT("Smith"), TEXT("Johnson"), TEXT("Williams"), TEXT("Brown"), TEXT("Jones"),
			TEXT("Garcia"), TEXT("Miller"), TEXT("Davis"), TEXT("Rodriguez"), TEXT("Martinez"),
			TEXT("Blackwood"), TEXT("Ironforge"), TEXT("Stormwind"), TEXT("Silverhand"), TEXT("Oakhart"),
			TEXT("Ravencroft"), TEXT("Thornwood"), TEXT("Ashford"), TEXT("Brightwater"), TEXT("Shadowmere")
		};
	}

	if (Occupations.Num() == 0)
	{
		Occupations = {
			TEXT("Farmer"), TEXT("Blacksmith"), TEXT("Baker"), TEXT("Guard"), TEXT("Merchant"),
			TEXT("Innkeeper"), TEXT("Hunter"), TEXT("Fisherman"), TEXT("Carpenter"), TEXT("Tailor"),
			TEXT("Healer"), TEXT("Scholar"), TEXT("Miner"), TEXT("Herbalist"), TEXT("Cook"),
			TEXT("Stable Hand"), TEXT("Priest"), TEXT("Soldier"), TEXT("Artist"), TEXT("Musician")
		};
	}
}

void ULyraNPCIdentityComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Biography.FirstName.IsEmpty())
	{
		GenerateRandomIdentity(Biography.Archetype);
	}
}

void ULyraNPCIdentityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULyraNPCIdentityComponent, Biography);
	DOREPLIFETIME(ULyraNPCIdentityComponent, CurrentEmotion);
	DOREPLIFETIME(ULyraNPCIdentityComponent, CurrentLifeState);
}

void ULyraNPCIdentityComponent::InitializeIdentity(const FLyraNPCBiography& NewBiography)
{
	Biography = NewBiography;
	if (!Biography.UniqueId.IsValid())
	{
		Biography.UniqueId = FGuid::NewGuid();
	}

	UE_LOG(LogLyraNPC, Log, TEXT("NPC Identity Initialized: %s"), *Biography.GetFullName());
}

void ULyraNPCIdentityComponent::GenerateRandomIdentity(ELyraNPCArchetype Archetype)
{
	Biography.Archetype = Archetype;

	// Random gender (simple 50/50)
	bool bIsMale = FMath::RandBool();

	// Generate name
	if (bIsMale && FirstNames_Male.Num() > 0)
	{
		Biography.FirstName = FirstNames_Male[FMath::RandRange(0, FirstNames_Male.Num() - 1)];
	}
	else if (!bIsMale && FirstNames_Female.Num() > 0)
	{
		Biography.FirstName = FirstNames_Female[FMath::RandRange(0, FirstNames_Female.Num() - 1)];
	}

	if (LastNames.Num() > 0)
	{
		Biography.LastName = LastNames[FMath::RandRange(0, LastNames.Num() - 1)];
	}

	// Age based on archetype
	switch (Archetype)
	{
	case ELyraNPCArchetype::Guard:
		Biography.Age = FMath::RandRange(20, 45);
		break;
	case ELyraNPCArchetype::Merchant:
		Biography.Age = FMath::RandRange(25, 60);
		break;
	case ELyraNPCArchetype::Worker:
		Biography.Age = FMath::RandRange(18, 55);
		break;
	default:
		Biography.Age = FMath::RandRange(18, 70);
		break;
	}

	// Occupation
	if (Occupations.Num() > 0)
	{
		Biography.Occupation = Occupations[FMath::RandRange(0, Occupations.Num() - 1)];
	}

	// Random personality traits with some correlation to archetype
	Biography.Personality.Openness = FMath::FRandRange(0.2f, 0.8f);
	Biography.Personality.Conscientiousness = FMath::FRandRange(0.3f, 0.9f);
	Biography.Personality.Extraversion = FMath::FRandRange(0.2f, 0.8f);
	Biography.Personality.Agreeableness = FMath::FRandRange(0.3f, 0.9f);
	Biography.Personality.Neuroticism = FMath::FRandRange(0.1f, 0.7f);

	// Archetype-specific personality adjustments
	switch (Archetype)
	{
	case ELyraNPCArchetype::Guard:
		Biography.Personality.Bravery = FMath::FRandRange(0.6f, 1.0f);
		Biography.Personality.Conscientiousness = FMath::FRandRange(0.6f, 1.0f);
		break;
	case ELyraNPCArchetype::Merchant:
		Biography.Personality.Extraversion = FMath::FRandRange(0.5f, 0.9f);
		Biography.Personality.Honesty = FMath::FRandRange(0.3f, 0.9f);
		break;
	case ELyraNPCArchetype::Traveler:
		Biography.Personality.Openness = FMath::FRandRange(0.6f, 1.0f);
		Biography.Personality.Curiosity = FMath::FRandRange(0.6f, 1.0f);
		break;
	case ELyraNPCArchetype::Enemy:
		Biography.Personality.Agreeableness = FMath::FRandRange(0.1f, 0.4f);
		break;
	default:
		Biography.Personality.Bravery = FMath::FRandRange(0.3f, 0.7f);
		Biography.Personality.Honesty = FMath::FRandRange(0.4f, 0.9f);
		Biography.Personality.Curiosity = FMath::FRandRange(0.3f, 0.7f);
		Biography.Personality.Patience = FMath::FRandRange(0.3f, 0.8f);
		Biography.Personality.Loyalty = FMath::FRandRange(0.4f, 0.9f);
		break;
	}

	UE_LOG(LogLyraNPC, Log, TEXT("Generated Random NPC: %s, Age %d, %s"),
		*Biography.GetFullName(), Biography.Age, *Biography.Occupation);
}

void ULyraNPCIdentityComponent::SetLifeState(ELyraNPCLifeState NewState)
{
	if (CurrentLifeState != NewState)
	{
		ELyraNPCLifeState OldState = CurrentLifeState;
		CurrentLifeState = NewState;

		if (AActor* Owner = GetOwner())
		{
			if (ALyraNPCCharacter* NPCChar = Cast<ALyraNPCCharacter>(Owner))
			{
				OnLifeStateChanged.Broadcast(NPCChar, NewState);
			}
		}

		UE_LOG(LogLyraNPC, Verbose, TEXT("%s: Life State changed from %d to %d"),
			*GetDisplayName(), static_cast<int32>(OldState), static_cast<int32>(NewState));
	}
}

void ULyraNPCIdentityComponent::SetEmotion(ELyraNPCEmotion NewEmotion)
{
	CurrentEmotion = NewEmotion;
}

float ULyraNPCIdentityComponent::GetDecisionModifier(FGameplayTag DecisionType) const
{
	// Returns a modifier based on personality for specific decision types
	// This can be extended with a data table for more complex mappings
	float Modifier = 1.0f;

	// Example: Risk-taking decisions
	if (DecisionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Decision.Risk"))))
	{
		Modifier = Biography.Personality.Bravery * (1.0f - Biography.Personality.Neuroticism * 0.5f);
	}
	// Social decisions
	else if (DecisionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Decision.Social"))))
	{
		Modifier = Biography.Personality.Extraversion * Biography.Personality.Agreeableness;
	}
	// Work-related decisions
	else if (DecisionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Decision.Work"))))
	{
		Modifier = Biography.Personality.Conscientiousness;
	}

	return FMath::Clamp(Modifier, 0.1f, 2.0f);
}

bool ULyraNPCIdentityComponent::WouldMakeRiskyDecision(float RiskLevel) const
{
	float Threshold = Biography.Personality.Bravery - (Biography.Personality.Neuroticism * 0.3f);
	return RiskLevel < Threshold;
}

bool ULyraNPCIdentityComponent::WouldHelpStranger() const
{
	float HelpChance = (Biography.Personality.Agreeableness + Biography.Personality.Extraversion) * 0.5f;
	return FMath::FRand() < HelpChance;
}

bool ULyraNPCIdentityComponent::PrefersGroup() const
{
	return Biography.Personality.Extraversion > 0.5f;
}

float ULyraNPCIdentityComponent::GetStressResistance() const
{
	return (1.0f - Biography.Personality.Neuroticism) * Biography.Personality.Conscientiousness;
}
