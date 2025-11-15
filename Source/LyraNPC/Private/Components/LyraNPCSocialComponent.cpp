// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCSocialComponent.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

ULyraNPCSocialComponent::ULyraNPCSocialComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 10.0f; // Update every 10 seconds
	SetIsReplicatedByDefault(true);
}

void ULyraNPCSocialComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULyraNPCSocialComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ULyraNPCSocialComponent, Relationships);
}

void ULyraNPCSocialComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastDecay += DeltaTime;
	if (TimeSinceLastDecay >= 60.0f) // Decay every minute
	{
		DecayRelationships(TimeSinceLastDecay);
		UpdateRelationshipTypes();
		TimeSinceLastDecay = 0.0f;
	}
}

void ULyraNPCSocialComponent::AddRelationship(ALyraNPCCharacter* OtherNPC, ELyraNPCRelationshipType Type)
{
	if (!OtherNPC) return;

	ULyraNPCIdentityComponent* OtherIdentity = OtherNPC->IdentityComponent;
	if (!OtherIdentity) return;

	FGuid OtherId = OtherIdentity->GetUniqueId();

	// Check if relationship already exists
	if (HasRelationship(OtherId)) return;

	FLyraNPCRelationship NewRelation;
	NewRelation.OtherNPC = OtherNPC;
	NewRelation.OtherNPCId = OtherId;
	NewRelation.RelationshipType = Type;
	NewRelation.LastInteractionTime = GetWorld()->GetTimeSeconds();

	// Set initial values based on type
	switch (Type)
	{
	case ELyraNPCRelationshipType::Family:
		NewRelation.Affinity = 80.0f;
		NewRelation.Trust = 90.0f;
		NewRelation.Familiarity = 100.0f;
		break;
	case ELyraNPCRelationshipType::Friend:
		NewRelation.Affinity = 50.0f;
		NewRelation.Trust = 60.0f;
		NewRelation.Familiarity = 60.0f;
		break;
	case ELyraNPCRelationshipType::Enemy:
		NewRelation.Affinity = -50.0f;
		NewRelation.Trust = 10.0f;
		NewRelation.Familiarity = 30.0f;
		break;
	default:
		NewRelation.Affinity = 0.0f;
		NewRelation.Trust = 50.0f;
		NewRelation.Familiarity = 0.0f;
		break;
	}

	Relationships.Add(NewRelation);
	UE_LOG(LogLyraNPC, Verbose, TEXT("Added relationship: %s"), *OtherNPC->GetNPCName());
}

void ULyraNPCSocialComponent::RemoveRelationship(const FGuid& OtherNPCId)
{
	for (int32 i = Relationships.Num() - 1; i >= 0; --i)
	{
		if (Relationships[i].OtherNPCId == OtherNPCId)
		{
			Relationships.RemoveAt(i);
			break;
		}
	}
}

void ULyraNPCSocialComponent::UpdateRelationship(const FGuid& OtherNPCId, float AffinityDelta, float FamiliarityDelta)
{
	FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	if (Relation)
	{
		Relation->Affinity = FMath::Clamp(Relation->Affinity + (AffinityDelta * AffinityChangeRate), -100.0f, 100.0f);
		Relation->Familiarity = FMath::Clamp(Relation->Familiarity + FamiliarityDelta, 0.0f, 100.0f);
		Relation->LastInteractionTime = GetWorld()->GetTimeSeconds();

		// Trust follows affinity but more slowly
		float TrustDelta = AffinityDelta * 0.3f;
		Relation->Trust = FMath::Clamp(Relation->Trust + TrustDelta, 0.0f, 100.0f);
	}
}

void ULyraNPCSocialComponent::SetRelationshipType(const FGuid& OtherNPCId, ELyraNPCRelationshipType NewType)
{
	FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	if (Relation)
	{
		Relation->RelationshipType = NewType;
	}
}

FLyraNPCRelationship ULyraNPCSocialComponent::GetRelationship(const FGuid& OtherNPCId) const
{
	const FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	return Relation ? *Relation : FLyraNPCRelationship();
}

bool ULyraNPCSocialComponent::HasRelationship(const FGuid& OtherNPCId) const
{
	return FindRelationship(OtherNPCId) != nullptr;
}

float ULyraNPCSocialComponent::GetAffinityWith(const FGuid& OtherNPCId) const
{
	const FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	return Relation ? Relation->Affinity : 0.0f;
}

float ULyraNPCSocialComponent::GetTrustLevel(const FGuid& OtherNPCId) const
{
	const FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	return Relation ? Relation->Trust : 50.0f;
}

ELyraNPCRelationshipType ULyraNPCSocialComponent::GetRelationshipType(const FGuid& OtherNPCId) const
{
	const FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	return Relation ? Relation->RelationshipType : ELyraNPCRelationshipType::Stranger;
}

TArray<FGuid> ULyraNPCSocialComponent::GetFriends() const
{
	TArray<FGuid> Friends;
	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.RelationshipType == ELyraNPCRelationshipType::Friend ||
			Relation.RelationshipType == ELyraNPCRelationshipType::CloseFriend)
		{
			Friends.Add(Relation.OtherNPCId);
		}
	}
	return Friends;
}

TArray<FGuid> ULyraNPCSocialComponent::GetEnemies() const
{
	TArray<FGuid> Enemies;
	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.RelationshipType == ELyraNPCRelationshipType::Enemy ||
			Relation.RelationshipType == ELyraNPCRelationshipType::Rival)
		{
			Enemies.Add(Relation.OtherNPCId);
		}
	}
	return Enemies;
}

TArray<FGuid> ULyraNPCSocialComponent::GetFamily() const
{
	TArray<FGuid> Family;
	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.RelationshipType == ELyraNPCRelationshipType::Family)
		{
			Family.Add(Relation.OtherNPCId);
		}
	}
	return Family;
}

int32 ULyraNPCSocialComponent::GetFriendCount() const
{
	return GetFriends().Num();
}

FGuid ULyraNPCSocialComponent::GetBestFriend() const
{
	FGuid BestFriendId;
	float HighestAffinity = -101.0f;

	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.Affinity > HighestAffinity && Relation.Affinity > 0.0f)
		{
			HighestAffinity = Relation.Affinity;
			BestFriendId = Relation.OtherNPCId;
		}
	}

	return BestFriendId;
}

FGuid ULyraNPCSocialComponent::GetWorstEnemy() const
{
	FGuid WorstEnemyId;
	float LowestAffinity = 101.0f;

	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.Affinity < LowestAffinity && Relation.Affinity < 0.0f)
		{
			LowestAffinity = Relation.Affinity;
			WorstEnemyId = Relation.OtherNPCId;
		}
	}

	return WorstEnemyId;
}

float ULyraNPCSocialComponent::GetAverageSocialStanding() const
{
	if (Relationships.Num() == 0) return 0.0f;

	float TotalAffinity = 0.0f;
	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		TotalAffinity += Relation.Affinity;
	}

	return TotalAffinity / Relationships.Num();
}

void ULyraNPCSocialComponent::OnPositiveInteraction(const FGuid& OtherNPCId, float Magnitude)
{
	UpdateRelationship(OtherNPCId, 5.0f * Magnitude, 2.0f * Magnitude);
}

void ULyraNPCSocialComponent::OnNegativeInteraction(const FGuid& OtherNPCId, float Magnitude)
{
	UpdateRelationship(OtherNPCId, -10.0f * Magnitude, 2.0f * Magnitude);
}

void ULyraNPCSocialComponent::OnNeutralInteraction(const FGuid& OtherNPCId)
{
	UpdateRelationship(OtherNPCId, 0.0f, 1.0f);
}

bool ULyraNPCSocialComponent::WouldHelpNPC(const FGuid& OtherNPCId) const
{
	float Affinity = GetAffinityWith(OtherNPCId);
	return Affinity > 20.0f;
}

bool ULyraNPCSocialComponent::WouldFightForNPC(const FGuid& OtherNPCId) const
{
	const FLyraNPCRelationship* Relation = FindRelationship(OtherNPCId);
	if (!Relation) return false;

	// Would fight for family or close friends with high trust
	if (Relation->RelationshipType == ELyraNPCRelationshipType::Family)
	{
		return true;
	}

	return Relation->Affinity > 60.0f && Relation->Trust > 70.0f;
}

bool ULyraNPCSocialComponent::WouldTrustNPC(const FGuid& OtherNPCId) const
{
	float Trust = GetTrustLevel(OtherNPCId);
	return Trust > 60.0f;
}

FLyraNPCRelationship* ULyraNPCSocialComponent::FindRelationship(const FGuid& OtherNPCId)
{
	for (FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.OtherNPCId == OtherNPCId)
		{
			return &Relation;
		}
	}
	return nullptr;
}

const FLyraNPCRelationship* ULyraNPCSocialComponent::FindRelationship(const FGuid& OtherNPCId) const
{
	for (const FLyraNPCRelationship& Relation : Relationships)
	{
		if (Relation.OtherNPCId == OtherNPCId)
		{
			return &Relation;
		}
	}
	return nullptr;
}

void ULyraNPCSocialComponent::UpdateRelationshipTypes()
{
	for (FLyraNPCRelationship& Relation : Relationships)
	{
		// Automatically upgrade/downgrade relationship type based on affinity
		if (Relation.RelationshipType != ELyraNPCRelationshipType::Family)
		{
			if (Relation.Affinity >= 70.0f)
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::CloseFriend;
			}
			else if (Relation.Affinity >= 40.0f)
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::Friend;
			}
			else if (Relation.Affinity >= 10.0f)
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::Acquaintance;
			}
			else if (Relation.Affinity <= -50.0f)
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::Enemy;
			}
			else if (Relation.Affinity <= -20.0f)
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::Rival;
			}
			else
			{
				Relation.RelationshipType = ELyraNPCRelationshipType::Stranger;
			}
		}
	}
}

void ULyraNPCSocialComponent::DecayRelationships(float DeltaTime)
{
	float HoursPassed = DeltaTime / 3600.0f;

	for (FLyraNPCRelationship& Relation : Relationships)
	{
		// Affinity slowly decays towards neutral if not interacted with
		if (Relation.Affinity > 0.0f)
		{
			Relation.Affinity = FMath::Max(0.0f, Relation.Affinity - (HoursPassed * 0.1f));
		}
		else if (Relation.Affinity < 0.0f)
		{
			Relation.Affinity = FMath::Min(0.0f, Relation.Affinity + (HoursPassed * 0.05f));
		}

		// Familiarity decays slowly
		Relation.Familiarity = FMath::Max(0.0f, Relation.Familiarity - (HoursPassed * 0.02f));
	}
}
