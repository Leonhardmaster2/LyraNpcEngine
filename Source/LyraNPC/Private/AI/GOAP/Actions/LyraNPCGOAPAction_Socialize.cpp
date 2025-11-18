// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/Actions/LyraNPCGOAPAction_Socialize.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Components/LyraNPCSocialComponent.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Systems/LyraNPCWorldSubsystem.h"

ULyraNPCGOAPAction_Socialize::ULyraNPCGOAPAction_Socialize()
{
	ActionName = FName("Socialize");
	ActionDescription = FText::FromString("Find another NPC and socialize to satisfy social need");

	// Preconditions: Need to be lonely and have access to other NPCs
	Preconditions.StateValues.Add(FName("IsLonely"), 1.0f);
	Preconditions.StateFlags.Add(FName("HasSocialPartner"), true);

	// Effects: No longer lonely
	Effects.StateValues.Add(FName("IsLonely"), 0.0f);
	Effects.StateFlags.Add(FName("HasSocialized"), true);

	// Base cost
	Cost.BaseCost = 4.0f;
	Cost.bRequiresMovement = true;
}

bool ULyraNPCGOAPAction_Socialize::CheckPreconditions(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return false;

	// Check if actually lonely
	ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent;
	if (!Needs) return false;

	float SocialValue = Needs->GetNeedValue(ELyraNPCNeedType::Social);
	if (SocialValue < 30.0f) // Only socialize if social need is low
	{
		return false;
	}

	// Check if other NPCs are nearby
	UWorld* World = NPC->GetWorld();
	if (!World) return false;

	ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
	if (!WorldSubsystem) return false;

	TArray<ALyraNPCCharacter*> NearbyNPCs = WorldSubsystem->GetNPCsInRadius(
		NPC->GetActorLocation(),
		SearchRadius
	);

	// Need at least one other NPC nearby
	return NearbyNPCs.Num() > 1; // More than just this NPC
}

FLyraNPCWorldState ULyraNPCGOAPAction_Socialize::ApplyEffects(const FLyraNPCWorldState& CurrentState) const
{
	FLyraNPCWorldState NewState = CurrentState;
	NewState.ApplyState(Effects);
	return NewState;
}

float ULyraNPCGOAPAction_Socialize::CalculateCost(ALyraNPCCharacter* NPC, const FLyraNPCWorldState& CurrentState) const
{
	if (!NPC) return 9999.0f;

	float TotalCost = Cost.BaseCost;

	// Find nearest friendly NPC
	UWorld* World = NPC->GetWorld();
	if (World)
	{
		ULyraNPCWorldSubsystem* WorldSubsystem = World->GetSubsystem<ULyraNPCWorldSubsystem>();
		if (WorldSubsystem)
		{
			TArray<ALyraNPCCharacter*> NearbyNPCs = WorldSubsystem->GetNPCsInRadius(
				NPC->GetActorLocation(),
				SearchRadius
			);

			float ClosestDistance = SearchRadius;
			bool FoundPartner = false;

			ULyraNPCSocialComponent* SocialComp = NPC->SocialComponent;
			ULyraNPCIdentityComponent* Identity = NPC->IdentityComponent;

			for (ALyraNPCCharacter* OtherNPC : NearbyNPCs)
			{
				if (OtherNPC == NPC) continue; // Skip self
				if (!OtherNPC->IsAlive()) continue; // Skip dead NPCs

				// Prefer friends
				if (SocialComp && Identity)
				{
					ULyraNPCIdentityComponent* OtherIdentity = OtherNPC->IdentityComponent;
					if (OtherIdentity)
					{
						float Affinity = SocialComp->GetAffinityWith(OtherIdentity->GetUniqueId());
						if (Affinity < 0.0f) continue; // Don't socialize with enemies
					}
				}

				float Distance = FVector::Dist(NPC->GetActorLocation(), OtherNPC->GetActorLocation());
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					FoundPartner = true;
				}
			}

			if (FoundPartner)
			{
				TotalCost += (ClosestDistance / 100.0f) * Cost.DistanceCostMultiplier;
			}
			else
			{
				return 9999.0f; // No suitable partner
			}
		}
	}

	return TotalCost;
}

void ULyraNPCGOAPAction_Socialize::OnActionStart(ALyraNPCCharacter* NPC, const FVector& TargetLocation, AActor* TargetActor)
{
	TimeSpentSocializing = 0.0f;
	SocialPartner = Cast<ALyraNPCCharacter>(TargetActor);
}

bool ULyraNPCGOAPAction_Socialize::TickAction(ALyraNPCCharacter* NPC, float DeltaTime)
{
	if (!NPC) return false;

	TimeSpentSocializing += DeltaTime;

	// Gradually restore social need while socializing
	if (ULyraNPCNeedsComponent* Needs = NPC->NeedsComponent)
	{
		float RestorationPerSecond = SocialSatisfaction / SocializeDuration;
		float CurrentSocial = Needs->GetNeedValue(ELyraNPCNeedType::Social);
		Needs->SetNeedValue(ELyraNPCNeedType::Social, FMath::Min(100.0f, CurrentSocial + RestorationPerSecond * DeltaTime));
	}

	// Update relationship with social partner
	if (SocialPartner.IsValid() && NPC->SocialComponent)
	{
		ULyraNPCIdentityComponent* PartnerIdentity = SocialPartner->IdentityComponent;
		if (PartnerIdentity)
		{
			NPC->SocialComponent->OnNeutralInteraction(PartnerIdentity->GetUniqueId());
		}
	}

	// Check if done socializing
	if (TimeSpentSocializing >= SocializeDuration)
	{
		return false; // Action complete
	}

	return true; // Still socializing
}

void ULyraNPCGOAPAction_Socialize::OnActionEnd(ALyraNPCCharacter* NPC, bool bSuccess)
{
	if (bSuccess && SocialPartner.IsValid() && NPC->SocialComponent)
	{
		ULyraNPCIdentityComponent* PartnerIdentity = SocialPartner->IdentityComponent;
		if (PartnerIdentity)
		{
			// Positive interaction on successful socialization
			NPC->SocialComponent->OnPositiveInteraction(PartnerIdentity->GetUniqueId(), 0.5f);
		}
	}

	TimeSpentSocializing = 0.0f;
	SocialPartner = nullptr;
}
