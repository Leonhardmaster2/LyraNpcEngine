// Copyright LyraNPC Framework. All Rights Reserved.

#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Components/LyraNPCScheduleComponent.h"
#include "Navigation/LyraNPCPathFollowingComponent.h"
#include "Components/LyraNPCSocialComponent.h"
#include "Components/LyraNPCGOAPComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"

ALyraNPCCharacter::ALyraNPCCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create all LyraNPC components
	IdentityComponent = CreateDefaultSubobject<ULyraNPCIdentityComponent>(TEXT("IdentityComponent"));
	CognitiveComponent = CreateDefaultSubobject<ULyraNPCCognitiveComponent>(TEXT("CognitiveComponent"));
	NeedsComponent = CreateDefaultSubobject<ULyraNPCNeedsComponent>(TEXT("NeedsComponent"));
	ScheduleComponent = CreateDefaultSubobject<ULyraNPCScheduleComponent>(TEXT("ScheduleComponent"));
	PathFollowingComponent = CreateDefaultSubobject<ULyraNPCPathFollowingComponent>(TEXT("PathFollowingComponent"));
	SocialComponent = CreateDefaultSubobject<ULyraNPCSocialComponent>(TEXT("SocialComponent"));
	GOAPComponent = CreateDefaultSubobject<ULyraNPCGOAPComponent>(TEXT("GOAPComponent"));

	// Set default AI controller class
	AIControllerClass = nullptr; // Will be set to LyraNPCAIController in Blueprint or manually

	// Setup character movement defaults
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->MaxWalkSpeed = 300.0f;
		MovementComp->bUseControllerDesiredRotation = true;
		MovementComp->bOrientRotationToMovement = false;
		MovementComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	}

	// Auto possess AI
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// Initialize combat stats
	CombatStats.MaxHealth = 100.0f;
	CombatStats.CurrentHealth = 100.0f;
}

void ALyraNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoInitialize)
	{
		InitializeNPC();
	}
}

void ALyraNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Apply cognitive skill effects to movement
	ApplyCognitiveSkillToMovement();
}

void ALyraNPCCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALyraNPCCharacter, CombatStats);
}

void ALyraNPCCharacter::InitializeNPC()
{
	// Initialize identity
	if (IdentityComponent)
	{
		IdentityComponent->GenerateRandomIdentity(InitialArchetype);
		IdentityComponent->HomeLocation = GetActorLocation();
		IdentityComponent->WorkplaceLocation = GetActorLocation() + FVector(FMath::FRandRange(-1000.0f, 1000.0f), FMath::FRandRange(-1000.0f, 1000.0f), 0.0f);
	}

	// Initialize cognitive skill
	if (CognitiveComponent)
	{
		CognitiveComponent->CognitiveSkill = InitialCognitiveSkill;
	}

	// Initialize needs for archetype
	if (NeedsComponent)
	{
		NeedsComponent->InitializeDefaultNeeds(InitialArchetype);
	}

	// Initialize schedule for archetype
	if (ScheduleComponent)
	{
		ScheduleComponent->InitializeDefaultSchedule(InitialArchetype);
	}

	// Apply cognitive skill to combat stats
	if (CognitiveComponent)
	{
		CombatStats.Accuracy = 0.5f + (CognitiveComponent->CognitiveSkill * 0.4f);
	}

	UE_LOG(LogLyraNPC, Log, TEXT("NPC Initialized: %s (%s)"), *GetNPCName(), *UEnum::GetValueAsString(InitialArchetype));
}

void ALyraNPCCharacter::InitializeFromData(const FLyraNPCBiography& Biography, float CognitiveSkillLevel)
{
	if (IdentityComponent)
	{
		IdentityComponent->InitializeIdentity(Biography);
	}

	if (CognitiveComponent)
	{
		CognitiveComponent->CognitiveSkill = CognitiveSkillLevel;
	}

	InitialArchetype = Biography.Archetype;
	InitialCognitiveSkill = CognitiveSkillLevel;

	// Initialize other components based on new data
	if (NeedsComponent)
	{
		NeedsComponent->InitializeDefaultNeeds(Biography.Archetype);
	}

	if (ScheduleComponent)
	{
		ScheduleComponent->InitializeDefaultSchedule(Biography.Archetype);
	}

	UE_LOG(LogLyraNPC, Log, TEXT("NPC Initialized from data: %s"), *Biography.GetFullName());
}

FString ALyraNPCCharacter::GetNPCName() const
{
	if (IdentityComponent)
	{
		return IdentityComponent->GetDisplayName();
	}
	return TEXT("Unknown NPC");
}

ELyraNPCArchetype ALyraNPCCharacter::GetArchetype() const
{
	if (IdentityComponent)
	{
		return IdentityComponent->GetArchetype();
	}
	return ELyraNPCArchetype::Neutral;
}

ELyraNPCLifeState ALyraNPCCharacter::GetLifeState() const
{
	if (IdentityComponent)
	{
		return IdentityComponent->CurrentLifeState;
	}
	return ELyraNPCLifeState::Idle;
}

ELyraNPCAlertLevel ALyraNPCCharacter::GetAlertLevel() const
{
	if (CognitiveComponent)
	{
		return CognitiveComponent->AlertLevel;
	}
	return ELyraNPCAlertLevel::Unaware;
}

float ALyraNPCCharacter::GetCognitiveSkill() const
{
	if (CognitiveComponent)
	{
		return CognitiveComponent->CognitiveSkill;
	}
	return 0.5f;
}

float ALyraNPCCharacter::GetOverallWellbeing() const
{
	if (NeedsComponent)
	{
		return NeedsComponent->GetOverallWellbeing();
	}
	return 100.0f;
}

void ALyraNPCCharacter::TakeDamage(float DamageAmount, AActor* DamageCauser)
{
	if (!IsAlive()) return;

	// Apply defense
	float ActualDamage = DamageAmount * (1.0f - (CombatStats.Defense / 100.0f));
	CombatStats.CurrentHealth -= ActualDamage;

	// Increase alertness
	if (CognitiveComponent)
	{
		CognitiveComponent->SetAlertLevel(ELyraNPCAlertLevel::Combat);
	}

	// Decrease safety need
	if (NeedsComponent)
	{
		NeedsComponent->ModifyNeed(ELyraNPCNeedType::Safety, -20.0f);
	}

	// Check for death
	if (CombatStats.CurrentHealth <= 0.0f)
	{
		CombatStats.CurrentHealth = 0.0f;
		OnDeath();
	}

	UE_LOG(LogLyraNPC, Verbose, TEXT("%s took %.1f damage (%.1f health remaining)"),
		*GetNPCName(), ActualDamage, CombatStats.CurrentHealth);
}

void ALyraNPCCharacter::Heal(float HealAmount)
{
	CombatStats.CurrentHealth = FMath::Min(CombatStats.CurrentHealth + HealAmount, CombatStats.MaxHealth);
}

bool ALyraNPCCharacter::IsAlive() const
{
	return CombatStats.CurrentHealth > 0.0f;
}

float ALyraNPCCharacter::GetHealthPercent() const
{
	return (CombatStats.CurrentHealth / CombatStats.MaxHealth) * 100.0f;
}

void ALyraNPCCharacter::SetMovementStyle(ELyraNPCMovementStyle Style)
{
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		float NewSpeed = GetMovementSpeedForStyle(Style);
		MovementComp->MaxWalkSpeed = NewSpeed;
	}
}

float ALyraNPCCharacter::GetMovementSpeedForStyle(ELyraNPCMovementStyle Style) const
{
	switch (Style)
	{
	case ELyraNPCMovementStyle::Walk:
		return 150.0f;
	case ELyraNPCMovementStyle::Jog:
		return 300.0f;
	case ELyraNPCMovementStyle::Run:
		return 450.0f;
	case ELyraNPCMovementStyle::Sprint:
		return 600.0f;
	case ELyraNPCMovementStyle::Sneak:
		return 100.0f;
	case ELyraNPCMovementStyle::Limp:
		return 75.0f;
	case ELyraNPCMovementStyle::Drunk:
		return 120.0f;
	default:
		return 150.0f;
	}
}

void ALyraNPCCharacter::OnDeath()
{
	if (IdentityComponent)
	{
		IdentityComponent->SetLifeState(ELyraNPCLifeState::Dead);
	}

	// Disable movement
	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->DisableMovement();
	}

	// Disable collision
	SetActorEnableCollision(false);

	UE_LOG(LogLyraNPC, Log, TEXT("%s has died"), *GetNPCName());
}

void ALyraNPCCharacter::ApplyCognitiveSkillToMovement()
{
	if (!CognitiveComponent) return;

	// Add movement wobble for lower intelligence NPCs
	float PathAccuracy = CognitiveComponent->GetPathAccuracy();
	if (PathAccuracy < 0.9f && GetVelocity().Size() > 10.0f)
	{
		// Add slight random offset to movement (simulating imprecise movement)
		float WobbleAmount = (1.0f - PathAccuracy) * 5.0f;
		FVector WobbleOffset = FMath::VRand() * WobbleAmount;
		WobbleOffset.Z = 0.0f;
		AddMovementInput(WobbleOffset, 0.1f);
	}
}
