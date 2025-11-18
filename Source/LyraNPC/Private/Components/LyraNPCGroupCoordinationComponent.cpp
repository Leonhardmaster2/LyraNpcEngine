// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCGroupCoordinationComponent.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

ULyraNPCGroupCoordinationComponent::ULyraNPCGroupCoordinationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f; // Update twice per second
	SetIsReplicatedByDefault(true);
}

void ULyraNPCGroupCoordinationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize group mood
	GroupMood.Fear = 0.0f;
	GroupMood.Aggression = 0.5f;
	GroupMood.Morale = 0.7f;
	GroupMood.Cohesion = 0.5f;
}

void ULyraNPCGroupCoordinationComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, Members);
	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, CurrentFormation);
	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, CurrentTactic);
	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, SharedKnowledge);
	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, CoordinatedActions);
	DOREPLIFETIME(ULyraNPCGroupCoordinationComponent, GroupMood);
}

void ULyraNPCGroupCoordinationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastUpdate += DeltaTime;
	TimeSinceLastMoodUpdate += DeltaTime;

	// Update every 0.5 seconds
	if (TimeSinceLastUpdate >= 0.5f)
	{
		CleanupInvalidMembers();
		ClearOldKnowledge();
		UpdateCoordinatedActions(TimeSinceLastUpdate);
		TimeSinceLastUpdate = 0.0f;
	}

	// Update mood less frequently
	if (TimeSinceLastMoodUpdate >= 2.0f)
	{
		UpdateGroupMood();
		if (bEmotionalContagion)
		{
			ApplyEmotionalContagion(TimeSinceLastMoodUpdate);
		}
		TimeSinceLastMoodUpdate = 0.0f;
	}
}

// ===== GROUP MEMBERSHIP =====

bool ULyraNPCGroupCoordinationComponent::AddMember(ALyraNPCCharacter* NPC, ELyraNPCGroupRole Role)
{
	if (!NPC) return false;
	if (Members.Num() >= MaxGroupSize) return false;

	ULyraNPCIdentityComponent* Identity = NPC->IdentityComponent;
	if (!Identity) return false;

	FGuid NPCId = Identity->GetUniqueId();

	// Check if already a member
	if (IsMember(NPCId)) return false;

	FLyraNPCGroupMember NewMember;
	NewMember.NPC = NPC;
	NewMember.NPCId = NPCId;
	NewMember.Role = Role;
	NewMember.JoinTime = GetWorld()->GetTimeSeconds();
	NewMember.FormationSlot = -1;
	NewMember.LastKnownHealth = 100.0f;

	// Calculate influence based on role and cognitive skill
	if (ULyraNPCCognitiveComponent* Cognitive = NPC->CognitiveComponent)
	{
		NewMember.Influence = Cognitive->GetCognitiveSkill();
		if (Role == ELyraNPCGroupRole::Leader)
		{
			NewMember.Influence += 0.3f;
		}
		NewMember.Influence = FMath::Clamp(NewMember.Influence, 0.0f, 1.0f);
	}

	// Auto-assign role if enabled
	if (bAutoAssignRoles && Role == ELyraNPCGroupRole::Follower)
	{
		AutoAssignRole(NewMember);
	}

	Members.Add(NewMember);

	// Reassign formation slots
	if (bUseFormations)
	{
		AssignFormationSlots();
	}

	OnGroupMemberJoined.Broadcast(NPC, NewMember.Role);
	UE_LOG(LogLyraNPC, Log, TEXT("Group %s: Added member %s with role %d"), *GroupId.ToString(), *NPC->GetNPCName(), (int32)NewMember.Role);

	return true;
}

bool ULyraNPCGroupCoordinationComponent::RemoveMember(const FGuid& NPCId)
{
	for (int32 i = Members.Num() - 1; i >= 0; --i)
	{
		if (Members[i].NPCId == NPCId)
		{
			ELyraNPCGroupRole FormerRole = Members[i].Role;
			Members.RemoveAt(i);

			// Reassign formation slots
			if (bUseFormations)
			{
				AssignFormationSlots();
			}

			OnGroupMemberLeft.Broadcast(NPCId, FormerRole);
			return true;
		}
	}
	return false;
}

void ULyraNPCGroupCoordinationComponent::SetMemberRole(const FGuid& NPCId, ELyraNPCGroupRole NewRole)
{
	FLyraNPCGroupMember* Member = FindMember(NPCId);
	if (Member)
	{
		Member->Role = NewRole;

		// Adjust influence
		if (NewRole == ELyraNPCGroupRole::Leader)
		{
			Member->Influence = FMath::Min(1.0f, Member->Influence + 0.3f);
		}

		// Reassign formation slots
		if (bUseFormations)
		{
			AssignFormationSlots();
		}
	}
}

ELyraNPCGroupRole ULyraNPCGroupCoordinationComponent::GetMemberRole(const FGuid& NPCId) const
{
	const FLyraNPCGroupMember* Member = FindMember(NPCId);
	return Member ? Member->Role : ELyraNPCGroupRole::None;
}

bool ULyraNPCGroupCoordinationComponent::IsMember(const FGuid& NPCId) const
{
	return FindMember(NPCId) != nullptr;
}

ALyraNPCCharacter* ULyraNPCGroupCoordinationComponent::GetLeader() const
{
	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (Member.Role == ELyraNPCGroupRole::Leader && Member.NPC.IsValid())
		{
			return Member.NPC.Get();
		}
	}
	return nullptr;
}

TArray<ALyraNPCCharacter*> ULyraNPCGroupCoordinationComponent::GetMembersByRole(ELyraNPCGroupRole Role) const
{
	TArray<ALyraNPCCharacter*> Result;
	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (Member.Role == Role && Member.NPC.IsValid())
		{
			Result.Add(Member.NPC.Get());
		}
	}
	return Result;
}

// ===== SHARED KNOWLEDGE =====

void ULyraNPCGroupCoordinationComponent::ShareKnowledge(const FLyraNPCGroupKnowledge& Knowledge)
{
	if (!bShareKnowledge) return;

	// Check if similar knowledge already exists (within 500 units)
	for (const FLyraNPCGroupKnowledge& Existing : SharedKnowledge)
	{
		if (Existing.Type == Knowledge.Type)
		{
			float Distance = FVector::Dist(Existing.Location, Knowledge.Location);
			if (Distance < 500.0f)
			{
				// Similar knowledge exists, don't add duplicate
				return;
			}
		}
	}

	SharedKnowledge.Add(Knowledge);
	OnGroupKnowledgeAdded.Broadcast(Knowledge);

	UE_LOG(LogLyraNPC, Verbose, TEXT("Group %s: Shared knowledge type %d"), *GroupId.ToString(), (int32)Knowledge.Type);
}

TArray<FLyraNPCGroupKnowledge> ULyraNPCGroupCoordinationComponent::GetKnowledgeByType(ELyraNPCGroupKnowledgeType Type) const
{
	TArray<FLyraNPCGroupKnowledge> Result;
	for (const FLyraNPCGroupKnowledge& Knowledge : SharedKnowledge)
	{
		if (Knowledge.Type == Type)
		{
			Result.Add(Knowledge);
		}
	}
	return Result;
}

FLyraNPCGroupKnowledge ULyraNPCGroupCoordinationComponent::GetMostUrgentKnowledge() const
{
	FLyraNPCGroupKnowledge MostUrgent;
	float HighestUrgency = 0.0f;

	for (const FLyraNPCGroupKnowledge& Knowledge : SharedKnowledge)
	{
		if (Knowledge.Urgency > HighestUrgency)
		{
			HighestUrgency = Knowledge.Urgency;
			MostUrgent = Knowledge;
		}
	}

	return MostUrgent;
}

void ULyraNPCGroupCoordinationComponent::ClearOldKnowledge()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = SharedKnowledge.Num() - 1; i >= 0; --i)
	{
		float Age = CurrentTime - SharedKnowledge[i].Timestamp;
		if (Age > KnowledgeDecayTime)
		{
			SharedKnowledge.RemoveAt(i);
		}
	}
}

void ULyraNPCGroupCoordinationComponent::ReportEnemySighting(AActor* Enemy, FVector Location, const FGuid& ReporterId, float Urgency)
{
	FLyraNPCGroupKnowledge Knowledge;
	Knowledge.Type = ELyraNPCGroupKnowledgeType::EnemySighting;
	Knowledge.Location = Location;
	Knowledge.RelatedActor = Enemy;
	Knowledge.ReporterId = ReporterId;
	Knowledge.Timestamp = GetWorld()->GetTimeSeconds();
	Knowledge.Urgency = Urgency;
	Knowledge.Accuracy = 0.9f;

	ShareKnowledge(Knowledge);
}

void ULyraNPCGroupCoordinationComponent::ReportThreat(FVector Location, const FGuid& ReporterId, float Urgency)
{
	FLyraNPCGroupKnowledge Knowledge;
	Knowledge.Type = ELyraNPCGroupKnowledgeType::ThreatLocation;
	Knowledge.Location = Location;
	Knowledge.ReporterId = ReporterId;
	Knowledge.Timestamp = GetWorld()->GetTimeSeconds();
	Knowledge.Urgency = Urgency;
	Knowledge.Accuracy = 0.8f;

	ShareKnowledge(Knowledge);
}

void ULyraNPCGroupCoordinationComponent::ReportAllyDown(ALyraNPCCharacter* Ally, const FGuid& ReporterId)
{
	if (!Ally) return;

	FLyraNPCGroupKnowledge Knowledge;
	Knowledge.Type = ELyraNPCGroupKnowledgeType::DeadAlly;
	Knowledge.Location = Ally->GetActorLocation();
	Knowledge.RelatedActor = Ally;
	Knowledge.ReporterId = ReporterId;
	Knowledge.Timestamp = GetWorld()->GetTimeSeconds();
	Knowledge.Urgency = 0.9f;
	Knowledge.Accuracy = 1.0f;

	ShareKnowledge(Knowledge);

	// Lower morale when ally dies
	GroupMood.Morale = FMath::Max(0.0f, GroupMood.Morale - 0.2f);
}

// ===== FORMATIONS =====

void ULyraNPCGroupCoordinationComponent::SetFormation(ELyraNPCGroupFormation Formation)
{
	if (CurrentFormation == Formation) return;

	CurrentFormation = Formation;
	GenerateFormationSlots();
	AssignFormationSlots();

	UE_LOG(LogLyraNPC, Log, TEXT("Group %s: Formation changed to %d"), *GroupId.ToString(), (int32)Formation);
}

void ULyraNPCGroupCoordinationComponent::UpdateFormation(FVector Center, FVector Direction)
{
	FormationCenter = Center;
	FormationDirection = Direction.GetSafeNormal();
}

FVector ULyraNPCGroupCoordinationComponent::GetFormationPositionForMember(const FGuid& NPCId) const
{
	const FLyraNPCGroupMember* Member = FindMember(NPCId);
	if (Member && Member->FormationSlot >= 0 && Member->FormationSlot < FormationSlots.Num())
	{
		return GetFormationPositionForSlot(Member->FormationSlot);
	}
	return FormationCenter;
}

FVector ULyraNPCGroupCoordinationComponent::GetFormationPositionForSlot(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= FormationSlots.Num())
	{
		return FormationCenter;
	}

	const FLyraNPCFormationSlot& Slot = FormationSlots[SlotIndex];

	// Transform relative offset to world space
	FRotator FormationRotation = FormationDirection.Rotation();
	FVector WorldOffset = FormationRotation.RotateVector(Slot.RelativeOffset);

	return FormationCenter + WorldOffset;
}

void ULyraNPCGroupCoordinationComponent::AssignFormationSlots()
{
	if (!bUseFormations || CurrentFormation == ELyraNPCGroupFormation::None)
	{
		return;
	}

	// Clear current assignments
	for (FLyraNPCFormationSlot& Slot : FormationSlots)
	{
		Slot.AssignedNPC = nullptr;
		Slot.bOccupied = false;
	}

	// Assign members to slots based on role preference
	for (FLyraNPCGroupMember& Member : Members)
	{
		Member.FormationSlot = -1;

		// Find best slot for this member's role
		int32 BestSlot = -1;
		for (int32 i = 0; i < FormationSlots.Num(); ++i)
		{
			if (!FormationSlots[i].bOccupied)
			{
				if (FormationSlots[i].PreferredRole == Member.Role)
				{
					BestSlot = i;
					break;
				}
				else if (BestSlot == -1)
				{
					BestSlot = i; // Fallback to first available
				}
			}
		}

		// Assign to best slot
		if (BestSlot >= 0)
		{
			Member.FormationSlot = BestSlot;
			FormationSlots[BestSlot].AssignedNPC = Member.NPC;
			FormationSlots[BestSlot].bOccupied = true;
		}
	}
}

void ULyraNPCGroupCoordinationComponent::GenerateFormationSlots()
{
	FormationSlots.Empty();

	int32 MemberCount = FMath::Max(Members.Num(), 4); // At least 4 slots
	float Spacing = 150.0f; // Distance between NPCs

	switch (CurrentFormation)
	{
	case ELyraNPCGroupFormation::Line:
		{
			// Horizontal line formation
			int32 HalfCount = MemberCount / 2;
			for (int32 i = 0; i < MemberCount; ++i)
			{
				FLyraNPCFormationSlot Slot;
				Slot.RelativeOffset = FVector(0.0f, (i - HalfCount) * Spacing, 0.0f);
				Slot.PreferredRole = ELyraNPCGroupRole::Follower;
				FormationSlots.Add(Slot);
			}
			// Leader in center
			if (FormationSlots.Num() > 0)
			{
				FormationSlots[MemberCount / 2].PreferredRole = ELyraNPCGroupRole::Leader;
			}
		}
		break;

	case ELyraNPCGroupFormation::Column:
		{
			// Vertical column formation
			for (int32 i = 0; i < MemberCount; ++i)
			{
				FLyraNPCFormationSlot Slot;
				Slot.RelativeOffset = FVector(i * Spacing, 0.0f, 0.0f);
				Slot.PreferredRole = ELyraNPCGroupRole::Follower;
				FormationSlots.Add(Slot);
			}
			// Leader at front
			if (FormationSlots.Num() > 0)
			{
				FormationSlots[0].PreferredRole = ELyraNPCGroupRole::Leader;
			}
		}
		break;

	case ELyraNPCGroupFormation::Wedge:
		{
			// V-shaped wedge formation
			FormationSlots.Add(FLyraNPCFormationSlot()); // Leader at point
			FormationSlots[0].RelativeOffset = FVector(200.0f, 0.0f, 0.0f);
			FormationSlots[0].PreferredRole = ELyraNPCGroupRole::Leader;

			int32 SideCount = (MemberCount - 1) / 2;
			for (int32 i = 1; i <= SideCount; ++i)
			{
				// Left side
				FLyraNPCFormationSlot LeftSlot;
				LeftSlot.RelativeOffset = FVector(-i * Spacing * 0.7f, -i * Spacing, 0.0f);
				LeftSlot.PreferredRole = ELyraNPCGroupRole::Flanker;
				FormationSlots.Add(LeftSlot);

				// Right side
				FLyraNPCFormationSlot RightSlot;
				RightSlot.RelativeOffset = FVector(-i * Spacing * 0.7f, i * Spacing, 0.0f);
				RightSlot.PreferredRole = ELyraNPCGroupRole::Flanker;
				FormationSlots.Add(RightSlot);
			}
		}
		break;

	case ELyraNPCGroupFormation::Circle:
		{
			// Circular formation
			float AngleStep = 360.0f / MemberCount;
			float Radius = Spacing * 2.0f;

			for (int32 i = 0; i < MemberCount; ++i)
			{
				float Angle = FMath::DegreesToRadians(i * AngleStep);
				FLyraNPCFormationSlot Slot;
				Slot.RelativeOffset = FVector(
					FMath::Cos(Angle) * Radius,
					FMath::Sin(Angle) * Radius,
					0.0f
				);
				Slot.PreferredRole = ELyraNPCGroupRole::Defender;
				FormationSlots.Add(Slot);
			}
		}
		break;

	case ELyraNPCGroupFormation::Shield:
		{
			// Shield wall - tight line with defenders in front
			int32 HalfCount = MemberCount / 2;
			for (int32 i = 0; i < MemberCount; ++i)
			{
				FLyraNPCFormationSlot Slot;
				Slot.RelativeOffset = FVector(0.0f, (i - HalfCount) * (Spacing * 0.7f), 0.0f);
				Slot.PreferredRole = ELyraNPCGroupRole::Defender;
				FormationSlots.Add(Slot);
			}
		}
		break;

	case ELyraNPCGroupFormation::Scatter:
		{
			// Scattered positions - random but controlled
			for (int32 i = 0; i < MemberCount; ++i)
			{
				FLyraNPCFormationSlot Slot;
				Slot.RelativeOffset = FVector(
					FMath::FRandRange(-Spacing * 2, Spacing * 2),
					FMath::FRandRange(-Spacing * 2, Spacing * 2),
					0.0f
				);
				Slot.PreferredRole = ELyraNPCGroupRole::Scout;
				FormationSlots.Add(Slot);
			}
		}
		break;

	default:
		break;
	}
}

// ===== TACTICS =====

void ULyraNPCGroupCoordinationComponent::SetTactic(ELyraNPCGroupTactic NewTactic)
{
	if (CurrentTactic == NewTactic) return;

	CurrentTactic = NewTactic;
	OnGroupTacticChanged.Broadcast(NewTactic);

	UE_LOG(LogLyraNPC, Log, TEXT("Group %s: Tactic changed to %d"), *GroupId.ToString(), (int32)NewTactic);
}

void ULyraNPCGroupCoordinationComponent::EvaluateAndSetTactic()
{
	// Count enemies
	int32 EnemyCount = GetKnownEnemyCount();
	float AvgHealth = GetAverageGroupHealth();
	float Morale = GroupMood.Morale;

	// Should we retreat?
	if (ShouldRetreat())
	{
		SetTactic(ELyraNPCGroupTactic::Retreat);
		SetFormation(ELyraNPCGroupFormation::Scatter);
		return;
	}

	// No enemies - idle or patrol
	if (EnemyCount == 0)
	{
		SetTactic(ELyraNPCGroupTactic::Patrol);
		SetFormation(ELyraNPCGroupFormation::Column);
		return;
	}

	// Outnumbered - defensive
	if (EnemyCount > Members.Num() * 1.5f)
	{
		SetTactic(ELyraNPCGroupTactic::Defensive);
		SetFormation(ELyraNPCGroupFormation::Circle);
		return;
	}

	// Good health and morale - advance
	if (AvgHealth > 70.0f && Morale > 0.6f)
	{
		SetTactic(ELyraNPCGroupTactic::Advance);
		SetFormation(ELyraNPCGroupFormation::Wedge);
		return;
	}

	// Default - hold position
	SetTactic(ELyraNPCGroupTactic::Hold);
	SetFormation(ELyraNPCGroupFormation::Line);
}

bool ULyraNPCGroupCoordinationComponent::ShouldRetreat() const
{
	// Retreat if morale is too low
	if (GroupMood.Morale < RetreatMoraleThreshold)
	{
		return true;
	}

	// Retreat if most members are injured
	float AvgHealth = GetAverageGroupHealth();
	if (AvgHealth < 30.0f)
	{
		return true;
	}

	// Retreat if leader is dead and morale is low
	if (!GetLeader() && GroupMood.Morale < 0.5f)
	{
		return true;
	}

	return false;
}

// ===== COORDINATED ACTIONS =====

bool ULyraNPCGroupCoordinationComponent::StartCoordinatedAction(const FLyraNPCCoordinatedAction& Action)
{
	// Check if any assigned NPCs are available
	bool AnyAvailable = false;
	for (const FGuid& NPCId : Action.AssignedNPCs)
	{
		if (IsMember(NPCId) && !IsNPCInCoordinatedAction(NPCId))
		{
			AnyAvailable = true;
			break;
		}
	}

	if (!AnyAvailable)
	{
		return false;
	}

	FLyraNPCCoordinatedAction NewAction = Action;
	NewAction.bActive = true;
	NewAction.bCompleted = false;
	NewAction.ScheduledStartTime = GetWorld()->GetTimeSeconds();

	CoordinatedActions.Add(NewAction);
	OnCoordinatedActionStarted.Broadcast(NewAction);

	UE_LOG(LogLyraNPC, Log, TEXT("Group %s: Started coordinated action"), *GroupId.ToString());
	return true;
}

void ULyraNPCGroupCoordinationComponent::CancelCoordinatedAction(FGameplayTag ActionTag)
{
	for (int32 i = CoordinatedActions.Num() - 1; i >= 0; --i)
	{
		if (CoordinatedActions[i].ActionTag == ActionTag)
		{
			CoordinatedActions.RemoveAt(i);
		}
	}
}

FLyraNPCCoordinatedAction ULyraNPCGroupCoordinationComponent::GetActiveActionForNPC(const FGuid& NPCId) const
{
	for (const FLyraNPCCoordinatedAction& Action : CoordinatedActions)
	{
		if (Action.bActive && Action.AssignedNPCs.Contains(NPCId))
		{
			return Action;
		}
	}
	return FLyraNPCCoordinatedAction();
}

bool ULyraNPCGroupCoordinationComponent::IsNPCInCoordinatedAction(const FGuid& NPCId) const
{
	for (const FLyraNPCCoordinatedAction& Action : CoordinatedActions)
	{
		if (Action.bActive && Action.AssignedNPCs.Contains(NPCId))
		{
			return true;
		}
	}
	return false;
}

// ===== GROUP MOOD & EMOTIONS =====

void ULyraNPCGroupCoordinationComponent::UpdateGroupMood()
{
	if (Members.Num() == 0) return;

	float TotalFear = 0.0f;
	float TotalAggression = 0.0f;
	float TotalMorale = 0.0f;
	int32 ValidMembers = 0;

	for (FLyraNPCGroupMember& Member : Members)
	{
		if (!Member.NPC.IsValid()) continue;

		ALyraNPCCharacter* NPC = Member.NPC.Get();

		// Update last known health
		Member.LastKnownHealth = NPC->GetHealthPercent();

		// Get cognitive component for alert level
		if (ULyraNPCCognitiveComponent* Cognitive = NPC->CognitiveComponent)
		{
			ELyraNPCAlertLevel AlertLevel = Cognitive->GetAlertLevel();

			// Alert level affects mood
			switch (AlertLevel)
			{
			case ELyraNPCAlertLevel::Combat:
				TotalFear += 0.6f;
				TotalAggression += 0.9f;
				break;
			case ELyraNPCAlertLevel::Alert:
				TotalFear += 0.4f;
				TotalAggression += 0.7f;
				break;
			case ELyraNPCAlertLevel::Suspicious:
				TotalFear += 0.2f;
				TotalAggression += 0.5f;
				break;
			default:
				TotalFear += 0.1f;
				TotalAggression += 0.3f;
				break;
			}
		}

		// Health affects morale
		float HealthPercent = Member.LastKnownHealth;
		TotalMorale += HealthPercent / 100.0f;

		ValidMembers++;
	}

	if (ValidMembers > 0)
	{
		GroupMood.Fear = TotalFear / ValidMembers;
		GroupMood.Aggression = TotalAggression / ValidMembers;
		GroupMood.Morale = TotalMorale / ValidMembers;

		// Cohesion based on time together and shared experiences
		GroupMood.Cohesion = FMath::Min(1.0f, Members.Num() / (float)MaxGroupSize + 0.3f);
	}
}

void ULyraNPCGroupCoordinationComponent::ApplyEmotionalContagion(float DeltaTime)
{
	// Spread fear and aggression between group members
	float ContagionRate = 0.1f * DeltaTime;

	for (FLyraNPCGroupMember& Member : Members)
	{
		if (!Member.NPC.IsValid()) continue;

		ULyraNPCCognitiveComponent* Cognitive = Member.NPC->CognitiveComponent;
		if (!Cognitive) continue;

		// Fear spreads quickly
		if (GroupMood.Fear > 0.5f)
		{
			// Increase alert level if group is fearful
			// This would typically call a method on Cognitive component
			// For now, we'll log it
			if (GroupMood.Fear > 0.7f)
			{
				UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s affected by group fear"), *Member.NPC->GetNPCName());
			}
		}

		// Aggression can spread (mob mentality)
		if (GroupMood.Aggression > 0.7f)
		{
			// Members become more aggressive when group is aggressive
			UE_LOG(LogLyraNPC, Verbose, TEXT("NPC %s affected by group aggression"), *Member.NPC->GetNPCName());
		}
	}
}

float ULyraNPCGroupCoordinationComponent::GetAverageGroupHealth() const
{
	if (Members.Num() == 0) return 100.0f;

	float TotalHealth = 0.0f;
	int32 ValidMembers = 0;

	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (Member.NPC.IsValid())
		{
			TotalHealth += Member.NPC->GetHealthPercent();
			ValidMembers++;
		}
	}

	return ValidMembers > 0 ? (TotalHealth / ValidMembers) : 100.0f;
}

int32 ULyraNPCGroupCoordinationComponent::GetKnownEnemyCount() const
{
	TSet<AActor*> UniqueEnemies;

	for (const FLyraNPCGroupKnowledge& Knowledge : SharedKnowledge)
	{
		if (Knowledge.Type == ELyraNPCGroupKnowledgeType::EnemySighting && Knowledge.RelatedActor.IsValid())
		{
			UniqueEnemies.Add(Knowledge.RelatedActor.Get());
		}
	}

	return UniqueEnemies.Num();
}

// ===== DECISION MAKING =====

float ULyraNPCGroupCoordinationComponent::VoteOnDecision(FGameplayTag DecisionTag) const
{
	if (Members.Num() == 0) return 0.0f;

	// Weighted voting based on influence
	float TotalInfluence = 0.0f;
	float VotesInFavor = 0.0f;

	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (!Member.NPC.IsValid()) continue;

		TotalInfluence += Member.Influence;

		// Simple random vote weighted by cognitive skill
		// In a real implementation, this would check NPC's actual opinion
		ULyraNPCCognitiveComponent* Cognitive = Member.NPC->CognitiveComponent;
		if (Cognitive && FMath::FRand() > 0.5f)
		{
			VotesInFavor += Member.Influence;
		}
	}

	return TotalInfluence > 0.0f ? (VotesInFavor / TotalInfluence) : 0.0f;
}

ALyraNPCCharacter* ULyraNPCGroupCoordinationComponent::GetMostInfluentialMember() const
{
	ALyraNPCCharacter* MostInfluential = nullptr;
	float HighestInfluence = 0.0f;

	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (Member.NPC.IsValid() && Member.Influence > HighestInfluence)
		{
			HighestInfluence = Member.Influence;
			MostInfluential = Member.NPC.Get();
		}
	}

	return MostInfluential;
}

// ===== PRIVATE HELPERS =====

FLyraNPCGroupMember* ULyraNPCGroupCoordinationComponent::FindMember(const FGuid& NPCId)
{
	for (FLyraNPCGroupMember& Member : Members)
	{
		if (Member.NPCId == NPCId)
		{
			return &Member;
		}
	}
	return nullptr;
}

const FLyraNPCGroupMember* ULyraNPCGroupCoordinationComponent::FindMember(const FGuid& NPCId) const
{
	for (const FLyraNPCGroupMember& Member : Members)
	{
		if (Member.NPCId == NPCId)
		{
			return &Member;
		}
	}
	return nullptr;
}

void ULyraNPCGroupCoordinationComponent::AutoAssignRole(FLyraNPCGroupMember& Member)
{
	if (!Member.NPC.IsValid()) return;

	ULyraNPCIdentityComponent* Identity = Member.NPC->IdentityComponent;
	if (!Identity) return;

	ELyraNPCArchetype Archetype = Identity->GetArchetype();

	// Auto-assign based on archetype
	switch (Archetype)
	{
	case ELyraNPCArchetype::Guard:
		Member.Role = !GetLeader() ? ELyraNPCGroupRole::Leader : ELyraNPCGroupRole::Defender;
		break;
	case ELyraNPCArchetype::Worker:
		Member.Role = ELyraNPCGroupRole::Support;
		break;
	case ELyraNPCArchetype::Merchant:
		Member.Role = ELyraNPCGroupRole::Follower;
		break;
	case ELyraNPCArchetype::Traveler:
		Member.Role = ELyraNPCGroupRole::Scout;
		break;
	case ELyraNPCArchetype::Enemy:
		Member.Role = ELyraNPCGroupRole::HeavyHitter;
		break;
	default:
		Member.Role = ELyraNPCGroupRole::Follower;
		break;
	}

	// First member becomes leader if no leader exists
	if (Members.Num() == 1)
	{
		Member.Role = ELyraNPCGroupRole::Leader;
	}
}

void ULyraNPCGroupCoordinationComponent::UpdateCoordinatedActions(float DeltaTime)
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (int32 i = CoordinatedActions.Num() - 1; i >= 0; --i)
	{
		FLyraNPCCoordinatedAction& Action = CoordinatedActions[i];

		if (!Action.bActive) continue;

		// Check if action has timed out
		float ActionDuration = CurrentTime - Action.ScheduledStartTime;
		if (ActionDuration > Action.MaxDuration)
		{
			Action.bCompleted = true;
			Action.bActive = false;
			CoordinatedActions.RemoveAt(i);
			continue;
		}

		// Check if all assigned NPCs are still valid
		bool AllValid = true;
		for (const FGuid& NPCId : Action.AssignedNPCs)
		{
			if (!IsMember(NPCId))
			{
				AllValid = false;
				break;
			}
		}

		if (!AllValid)
		{
			CoordinatedActions.RemoveAt(i);
		}
	}
}

void ULyraNPCGroupCoordinationComponent::CleanupInvalidMembers()
{
	for (int32 i = Members.Num() - 1; i >= 0; --i)
	{
		if (!Members[i].NPC.IsValid() || !Members[i].NPC->IsAlive())
		{
			FGuid RemovedId = Members[i].NPCId;
			ELyraNPCGroupRole RemovedRole = Members[i].Role;
			Members.RemoveAt(i);
			OnGroupMemberLeft.Broadcast(RemovedId, RemovedRole);
		}
	}
}
