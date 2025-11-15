// Copyright LyraNPC Framework. All Rights Reserved.

#include "Tasks/LyraNPCTaskActor.h"
#include "LyraNPCModule.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Components/LyraNPCNeedsComponent.h"

ULyraNPCTaskActor::ULyraNPCTaskActor()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 5.0f; // Cleanup every 5 seconds

	// Default to all archetypes allowed
	AllowedArchetypes.Add(ELyraNPCArchetype::Villager);
	AllowedArchetypes.Add(ELyraNPCArchetype::Worker);
	AllowedArchetypes.Add(ELyraNPCArchetype::Guard);
	AllowedArchetypes.Add(ELyraNPCArchetype::Merchant);
	AllowedArchetypes.Add(ELyraNPCArchetype::Traveler);
	AllowedArchetypes.Add(ELyraNPCArchetype::Companion);
	AllowedArchetypes.Add(ELyraNPCArchetype::Neutral);

	// Add default interaction point
	InteractionPoints.Add(FTransform::Identity);
}

void ULyraNPCTaskActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateAvailability();
}

void ULyraNPCTaskActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	CleanupInvalidReferences();
	UpdateAvailability();
}

bool ULyraNPCTaskActor::CanReserve(ALyraNPCCharacter* NPC) const
{
	if (!NPC || !bIsEnabled || !bIsAvailable) return false;
	if (!CanNPCUseTask(NPC)) return false;

	// Check if already reserved by this NPC
	if (IsReservedBy(NPC)) return true;

	// Check capacity
	return GetAvailableSlots() > 0;
}

bool ULyraNPCTaskActor::Reserve(ALyraNPCCharacter* NPC)
{
	if (!CanReserve(NPC)) return false;

	if (!IsReservedBy(NPC))
	{
		ReservedBy.Add(NPC);
		UpdateAvailability();

		UE_LOG(LogLyraNPC, Verbose, TEXT("Task %s reserved by NPC"), *TaskName);
	}
	return true;
}

void ULyraNPCTaskActor::CancelReservation(ALyraNPCCharacter* NPC)
{
	for (int32 i = ReservedBy.Num() - 1; i >= 0; --i)
	{
		if (ReservedBy[i].Get() == NPC)
		{
			ReservedBy.RemoveAt(i);
			UE_LOG(LogLyraNPC, Verbose, TEXT("Task %s reservation cancelled"), *TaskName);
		}
	}
	UpdateAvailability();
}

bool ULyraNPCTaskActor::IsReservedBy(ALyraNPCCharacter* NPC) const
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& User : ReservedBy)
	{
		if (User.Get() == NPC) return true;
	}
	return false;
}

int32 ULyraNPCTaskActor::GetAvailableSlots() const
{
	int32 TotalOccupied = CurrentUsers.Num() + ReservedBy.Num();
	return FMath::Max(0, MaxUsers - TotalOccupied);
}

bool ULyraNPCTaskActor::StartUsing(ALyraNPCCharacter* NPC)
{
	if (!NPC || !bIsEnabled) return false;
	if (IsBeingUsedBy(NPC)) return true; // Already using

	// Check if reserved by this NPC or has available slot
	if (!IsReservedBy(NPC) && GetAvailableSlots() <= 0)
	{
		return false;
	}

	// Remove from reserved list if was reserved
	CancelReservation(NPC);

	// Add to current users
	CurrentUsers.Add(NPC);
	UpdateAvailability();

	OnTaskStarted.Broadcast(NPC, this, GetRandomDuration());

	UE_LOG(LogLyraNPC, Verbose, TEXT("NPC started using task %s"), *TaskName);
	return true;
}

void ULyraNPCTaskActor::StopUsing(ALyraNPCCharacter* NPC)
{
	for (int32 i = CurrentUsers.Num() - 1; i >= 0; --i)
	{
		if (CurrentUsers[i].Get() == NPC)
		{
			CurrentUsers.RemoveAt(i);
			OnTaskCompleted.Broadcast(NPC, this);
			UE_LOG(LogLyraNPC, Verbose, TEXT("NPC stopped using task %s"), *TaskName);
		}
	}
	UpdateAvailability();
}

bool ULyraNPCTaskActor::IsBeingUsedBy(ALyraNPCCharacter* NPC) const
{
	for (const TWeakObjectPtr<ALyraNPCCharacter>& User : CurrentUsers)
	{
		if (User.Get() == NPC) return true;
	}
	return false;
}

int32 ULyraNPCTaskActor::GetCurrentUserCount() const
{
	return CurrentUsers.Num();
}

bool ULyraNPCTaskActor::CanNPCUseTask(ALyraNPCCharacter* NPC) const
{
	if (!NPC || !bIsEnabled) return false;

	// Check if private task
	if (bIsPrivate)
	{
		ULyraNPCIdentityComponent* Identity = NPC->FindComponentByClass<ULyraNPCIdentityComponent>();
		if (!Identity || Identity->GetUniqueId() != OwnerNPCId)
		{
			return false;
		}
	}

	// Check archetype
	ULyraNPCIdentityComponent* Identity = NPC->FindComponentByClass<ULyraNPCIdentityComponent>();
	if (Identity && !CheckArchetypeAccess(Identity->GetArchetype()))
	{
		return false;
	}

	// Check tags
	if (Identity && !CheckTagAccess(Identity->Biography.CharacterTags))
	{
		return false;
	}

	return true;
}

float ULyraNPCTaskActor::GetScoreForNPC(ALyraNPCCharacter* NPC) const
{
	if (!CanNPCUseTask(NPC)) return 0.0f;

	float Score = TaskPriority;

	// Boost score based on needs satisfaction
	ULyraNPCNeedsComponent* Needs = NPC->FindComponentByClass<ULyraNPCNeedsComponent>();
	if (Needs)
	{
		for (const auto& Pair : NeedsSatisfaction)
		{
			float NeedValue = Needs->GetNeedValue(Pair.Key);
			float NeedDeficit = 100.0f - NeedValue;

			// Higher deficit = higher score bonus
			Score += (NeedDeficit / 100.0f) * Pair.Value * 0.1f;
		}
	}

	// Reduce score if far away (distance penalty)
	float Distance = FVector::Dist(NPC->GetActorLocation(), GetTaskLocation());
	float DistancePenalty = Distance / 10000.0f; // 1 point penalty per 10000 units
	Score -= DistancePenalty;

	return FMath::Max(0.0f, Score);
}

FTransform ULyraNPCTaskActor::GetBestInteractionPoint(ALyraNPCCharacter* NPC) const
{
	if (InteractionPoints.Num() == 0)
	{
		return GetComponentTransform();
	}

	// Find closest unoccupied interaction point
	FTransform BestPoint = InteractionPoints[0];
	float BestDistance = MAX_FLT;

	FVector NPCLocation = NPC ? NPC->GetActorLocation() : FVector::ZeroVector;

	for (int32 i = 0; i < InteractionPoints.Num(); ++i)
	{
		FTransform WorldTransform = InteractionPoints[i] * GetComponentTransform();
		FVector PointLocation = WorldTransform.GetLocation();

		// Check if point is occupied
		bool bOccupied = false;
		for (const TWeakObjectPtr<ALyraNPCCharacter>& User : CurrentUsers)
		{
			if (User.IsValid())
			{
				float UserDist = FVector::Dist(User->GetActorLocation(), PointLocation);
				if (UserDist < 50.0f)
				{
					bOccupied = true;
					break;
				}
			}
		}

		if (!bOccupied)
		{
			float Distance = FVector::Dist(NPCLocation, PointLocation);
			if (Distance < BestDistance)
			{
				BestDistance = Distance;
				BestPoint = InteractionPoints[i];
			}
		}
	}

	return BestPoint * GetComponentTransform();
}

FVector ULyraNPCTaskActor::GetTaskLocation() const
{
	return GetComponentLocation();
}

float ULyraNPCTaskActor::GetRandomDuration() const
{
	return FMath::FRandRange(MinDuration, MaxDuration);
}

void ULyraNPCTaskActor::CleanupInvalidReferences()
{
	// Clean up current users
	for (int32 i = CurrentUsers.Num() - 1; i >= 0; --i)
	{
		if (!CurrentUsers[i].IsValid())
		{
			CurrentUsers.RemoveAt(i);
		}
	}

	// Clean up reservations
	for (int32 i = ReservedBy.Num() - 1; i >= 0; --i)
	{
		if (!ReservedBy[i].IsValid())
		{
			ReservedBy.RemoveAt(i);
		}
	}

	UpdateAvailability();
}

void ULyraNPCTaskActor::SetEnabled(bool bEnabled)
{
	bIsEnabled = bEnabled;
	UpdateAvailability();
}

void ULyraNPCTaskActor::SetOwner(const FGuid& NPCId)
{
	OwnerNPCId = NPCId;
	bIsPrivate = NPCId.IsValid();
}

void ULyraNPCTaskActor::UpdateAvailability()
{
	bIsAvailable = bIsEnabled && GetAvailableSlots() > 0;
}

bool ULyraNPCTaskActor::CheckArchetypeAccess(ELyraNPCArchetype Archetype) const
{
	if (AllowedArchetypes.Num() == 0) return true; // No restrictions
	return AllowedArchetypes.Contains(Archetype);
}

bool ULyraNPCTaskActor::CheckTagAccess(const FGameplayTagContainer& NPCTags) const
{
	// Check required tags
	if (RequiredTags.Num() > 0)
	{
		if (!NPCTags.HasAll(RequiredTags))
		{
			return false;
		}
	}

	// Check blocking tags
	if (BlockingTags.Num() > 0)
	{
		if (NPCTags.HasAny(BlockingTags))
		{
			return false;
		}
	}

	return true;
}
