# LyraNPC Framework - Complete Integration Guide

This comprehensive guide covers every aspect of integrating and using the LyraNPC AI Framework in your Unreal Engine 5 project.

---

## Table of Contents

1. [Installation & Setup](#installation--setup)
2. [C++ Integration](#c-integration)
3. [Blueprint Integration](#blueprint-integration)
4. [Common Scenarios](#common-scenarios)
5. [Advanced Usage](#advanced-usage)
6. [Performance Tuning](#performance-tuning)
7. [Troubleshooting](#troubleshooting)

---

## Installation & Setup

### Step 1: Add Plugin to Project

1. Copy the `LyraNPC` folder to your project's `Plugins` directory
2. Regenerate project files (right-click `.uproject` → Generate Visual Studio Project Files)
3. Open project and enable the plugin if not auto-enabled

### Step 2: Module Dependencies

Add to your game module's `Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "LyraNPC",
    "GameplayTags",
    "AIModule"
});
```

### Step 3: GameplayTags Setup

The plugin includes `Config/DefaultLyraNPC.ini` with predefined tags. To use them:

1. Copy the file to your project's `Config` folder, OR
2. Add to your `DefaultGameplayTags.ini`:

```ini
+GameplayTagList=(Tag="Activity.Work",DevComment="NPC is working")
+GameplayTagList=(Tag="Activity.Sleep",DevComment="NPC is sleeping")
+GameplayTagList=(Tag="Task.Eat",DevComment="Food consumption task")
// ... add all tags from DefaultLyraNPC.ini
```

---

## C++ Integration

### Basic NPC Creation

**Option 1: Spawn with Default Settings**

```cpp
#include "Core/LyraNPCCharacter.h"
#include "Core/LyraNPCFunctionLibrary.h"

void AMyGameMode::SpawnVillagers()
{
    // Spawn a villager with average intelligence
    ALyraNPCCharacter* Villager = ULyraNPCFunctionLibrary::SpawnNPC(
        this,
        ALyraNPCCharacter::StaticClass(),
        FVector(0, 0, 100),
        FRotator::ZeroRotator,
        ELyraNPCArchetype::Villager,
        0.5f  // Cognitive skill: 0.5 = average
    );
}
```

**Option 2: Manual Configuration**

```cpp
void AMyGameMode::SpawnCustomNPC()
{
    FActorSpawnParameters SpawnParams;
    ALyraNPCCharacter* NPC = GetWorld()->SpawnActor<ALyraNPCCharacter>(
        ALyraNPCCharacter::StaticClass(),
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    // Disable auto-initialization
    NPC->bAutoInitialize = false;

    // Configure before initialization
    NPC->InitialArchetype = ELyraNPCArchetype::Guard;
    NPC->InitialCognitiveSkill = 0.85f;  // Smart guard

    // Custom combat stats
    NPC->CombatStats.MaxHealth = 150.0f;
    NPC->CombatStats.CurrentHealth = 150.0f;
    NPC->CombatStats.AttackDamage = 25.0f;
    NPC->CombatStats.Defense = 15.0f;

    // Now initialize
    NPC->InitializeNPC();

    // Register with world subsystem
    if (ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>())
    {
        Subsystem->RegisterNPC(NPC);
    }
}
```

**Option 3: Full Custom Biography**

```cpp
void AMyGameMode::SpawnNamedNPC()
{
    ALyraNPCCharacter* NPC = GetWorld()->SpawnActor<ALyraNPCCharacter>(
        ALyraNPCCharacter::StaticClass(),
        SpawnLocation,
        SpawnRotation
    );

    NPC->bAutoInitialize = false;

    // Create custom biography
    FLyraNPCBiography Bio;
    Bio.UniqueId = FGuid::NewGuid();
    Bio.FirstName = TEXT("Marcus");
    Bio.LastName = TEXT("Ironforge");
    Bio.Nickname = TEXT("The Hammer");
    Bio.Age = 45;
    Bio.Occupation = TEXT("Master Blacksmith");
    Bio.Origin = TEXT("Northern Mountains");
    Bio.Backstory = TEXT("Former soldier turned craftsman after the war.");
    Bio.Archetype = ELyraNPCArchetype::Worker;

    // Custom personality
    Bio.Personality.Conscientiousness = 0.9f;  // Very hardworking
    Bio.Personality.Extraversion = 0.3f;       // Introverted
    Bio.Personality.Agreeableness = 0.6f;      // Generally friendly
    Bio.Personality.Openness = 0.4f;           // Traditional
    Bio.Personality.Neuroticism = 0.2f;        // Calm
    Bio.Personality.Bravery = 0.8f;            // Brave
    Bio.Personality.Honesty = 0.95f;           // Very honest
    Bio.Personality.Patience = 0.85f;          // Patient craftsman

    // Initialize with custom data
    NPC->InitializeFromData(Bio, 0.7f);  // Skilled craftsman
}
```

### Creating Custom NPC Classes

```cpp
// MyCustomNPC.h
#pragma once

#include "Core/LyraNPCCharacter.h"
#include "MyCustomNPC.generated.h"

UCLASS()
class MYGAME_API AMyCustomNPC : public ALyraNPCCharacter
{
    GENERATED_BODY()

public:
    AMyCustomNPC();

    // Custom functionality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Custom")
    float CustomSkillLevel = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Custom")
    void PerformSpecialAbility();

protected:
    virtual void BeginPlay() override;
};

// MyCustomNPC.cpp
#include "MyCustomNPC.h"
#include "AI/Controllers/LyraNPCAIController.h"

AMyCustomNPC::AMyCustomNPC()
{
    // Set custom AI Controller
    AIControllerClass = ALyraNPCAIController::StaticClass();

    // Override defaults
    InitialArchetype = ELyraNPCArchetype::Custom;
    InitialCognitiveSkill = 0.75f;

    // Adjust combat stats
    CombatStats.MaxHealth = 200.0f;
    CombatStats.CurrentHealth = 200.0f;
}

void AMyCustomNPC::BeginPlay()
{
    Super::BeginPlay();

    // Custom initialization after components are ready
    if (IdentityComponent)
    {
        IdentityComponent->HomeLocation = GetActorLocation();
    }
}

void AMyCustomNPC::PerformSpecialAbility()
{
    // Custom ability based on cognitive skill
    float SuccessChance = GetCognitiveSkill() * CustomSkillLevel;
    if (FMath::FRand() < SuccessChance)
    {
        // Ability succeeds
    }
}
```

### Setting Up Task Actors

**Creating a Bed (Sleep Task)**

```cpp
// In your Bed actor class
#include "Tasks/LyraNPCTaskActor.h"

ABedActor::ABedActor()
{
    // Create task component
    NPCTaskComponent = CreateDefaultSubobject<ULyraNPCTaskActor>(TEXT("NPCTask"));

    // Configure as sleep task
    NPCTaskComponent->TaskType = FGameplayTag::RequestGameplayTag(TEXT("Task.Sleep"));
    NPCTaskComponent->TaskName = TEXT("Bed");
    NPCTaskComponent->TaskDescription = TEXT("A comfortable bed for sleeping");

    // Duration: 6-10 hours (in game seconds, assuming 1 minute = 1 hour)
    NPCTaskComponent->MinDuration = 360.0f;
    NPCTaskComponent->MaxDuration = 600.0f;
    NPCTaskComponent->DefaultDuration = 480.0f;

    // Needs satisfaction (per minute of use)
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Energy, 60.0f);
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Comfort, 30.0f);

    // Single occupancy, can be private
    NPCTaskComponent->MaxUsers = 1;
    NPCTaskComponent->bIsPrivate = false;  // Set true to assign owner

    // Animation
    NPCTaskComponent->AnimationTag = FGameplayTag::RequestGameplayTag(TEXT("Animation.Sleeping"));
    NPCTaskComponent->bSnapToPosition = true;
    NPCTaskComponent->bSnapRotation = true;

    // Add interaction point (where NPC lies down)
    FTransform InteractionPoint;
    InteractionPoint.SetLocation(FVector(0, 0, 50));  // Local space
    InteractionPoint.SetRotation(FQuat::Identity);
    NPCTaskComponent->InteractionPoints.Empty();
    NPCTaskComponent->InteractionPoints.Add(InteractionPoint);
}

void ABedActor::BeginPlay()
{
    Super::BeginPlay();

    // Register with world subsystem
    if (ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>())
    {
        Subsystem->RegisterTaskActor(NPCTaskComponent);
    }
}
```

**Creating a Workbench (Work Task)**

```cpp
AWorkbenchActor::AWorkbenchActor()
{
    NPCTaskComponent = CreateDefaultSubobject<ULyraNPCTaskActor>(TEXT("NPCTask"));

    NPCTaskComponent->TaskType = FGameplayTag::RequestGameplayTag(TEXT("Task.Work"));
    NPCTaskComponent->TaskName = TEXT("Workbench");

    // Only workers can use this
    NPCTaskComponent->AllowedArchetypes.Empty();
    NPCTaskComponent->AllowedArchetypes.Add(ELyraNPCArchetype::Worker);

    // Work duration: 1-4 hours
    NPCTaskComponent->MinDuration = 60.0f;
    NPCTaskComponent->MaxDuration = 240.0f;

    // Satisfies purpose/work need
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Purpose, 40.0f);

    // Multiple workers can use
    NPCTaskComponent->MaxUsers = 3;

    // Can be interrupted for urgent needs
    NPCTaskComponent->bCanBeInterrupted = true;

    // Add multiple interaction points
    for (int32 i = 0; i < 3; ++i)
    {
        FTransform Point;
        Point.SetLocation(FVector(i * 100.0f, 0, 0));
        NPCTaskComponent->InteractionPoints.Add(Point);
    }
}
```

**Creating a Tavern Table (Social Task)**

```cpp
ATavernTableActor::ATavernTableActor()
{
    NPCTaskComponent = CreateDefaultSubobject<ULyraNPCTaskActor>(TEXT("NPCTask"));

    NPCTaskComponent->TaskType = FGameplayTag::RequestGameplayTag(TEXT("Task.Social"));
    NPCTaskComponent->TaskName = TEXT("Tavern Table");

    // Duration: 30 min to 2 hours
    NPCTaskComponent->MinDuration = 30.0f;
    NPCTaskComponent->MaxDuration = 120.0f;

    // Satisfies multiple needs
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Social, 50.0f);
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Entertainment, 30.0f);
    NPCTaskComponent->NeedsSatisfaction.Add(ELyraNPCNeedType::Hunger, 20.0f);  // Food served

    // Multiple seats
    NPCTaskComponent->MaxUsers = 4;

    // Higher priority than some other tasks
    NPCTaskComponent->TaskPriority = 1.5f;
}
```

### Configuring Schedules Programmatically

```cpp
void ConfigureBlacksmithSchedule(ALyraNPCCharacter* NPC)
{
    ULyraNPCScheduleComponent* Schedule = NPC->ScheduleComponent;
    if (!Schedule) return;

    Schedule->ClearSchedule();

    // Wake up: 5:00 AM
    FLyraNPCScheduleBlock WakeUp;
    WakeUp.StartHour = 5.0f;
    WakeUp.EndHour = 6.0f;
    WakeUp.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Morning"));
    WakeUp.LocationName = TEXT("Home");
    WakeUp.bMandatory = false;
    Schedule->AddScheduleBlock(WakeUp);

    // Breakfast: 6:00 - 7:00
    FLyraNPCScheduleBlock Breakfast;
    Breakfast.StartHour = 6.0f;
    Breakfast.EndHour = 7.0f;
    Breakfast.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
    Breakfast.LocationName = TEXT("Home");
    Breakfast.Priority = ELyraNPCTaskPriority::High;
    Breakfast.bMandatory = true;
    Schedule->AddScheduleBlock(Breakfast);

    // Work at forge: 7:00 - 12:00
    FLyraNPCScheduleBlock MorningWork;
    MorningWork.StartHour = 7.0f;
    MorningWork.EndHour = 12.0f;
    MorningWork.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Work"));
    MorningWork.LocationName = TEXT("Blacksmith");
    MorningWork.Priority = ELyraNPCTaskPriority::High;
    MorningWork.bMandatory = true;
    Schedule->AddScheduleBlock(MorningWork);

    // Lunch break: 12:00 - 13:00
    FLyraNPCScheduleBlock Lunch;
    Lunch.StartHour = 12.0f;
    Lunch.EndHour = 13.0f;
    Lunch.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
    Lunch.LocationName = TEXT("Tavern");
    Lunch.bMandatory = true;
    Schedule->AddScheduleBlock(Lunch);

    // Afternoon work: 13:00 - 18:00
    FLyraNPCScheduleBlock AfternoonWork;
    AfternoonWork.StartHour = 13.0f;
    AfternoonWork.EndHour = 18.0f;
    AfternoonWork.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Work"));
    AfternoonWork.LocationName = TEXT("Blacksmith");
    AfternoonWork.bMandatory = true;
    Schedule->AddScheduleBlock(AfternoonWork);

    // Dinner: 18:00 - 19:00
    FLyraNPCScheduleBlock Dinner;
    Dinner.StartHour = 18.0f;
    Dinner.EndHour = 19.0f;
    Dinner.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
    Dinner.LocationName = TEXT("Tavern");
    Dinner.bMandatory = true;
    Schedule->AddScheduleBlock(Dinner);

    // Evening leisure: 19:00 - 21:00
    FLyraNPCScheduleBlock Leisure;
    Leisure.StartHour = 19.0f;
    Leisure.EndHour = 21.0f;
    Leisure.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Leisure"));
    Leisure.LocationName = TEXT("Tavern");
    Leisure.bMandatory = false;
    Leisure.FlexibilityMinutes = 60.0f;  // Can skip if tired
    Schedule->AddScheduleBlock(Leisure);

    // Sleep: 21:00 - 5:00
    FLyraNPCScheduleBlock Sleep;
    Sleep.StartHour = 21.0f;
    Sleep.EndHour = 5.0f;  // Wraps to next day
    Sleep.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep"));
    Sleep.LocationName = TEXT("Home");
    Sleep.Priority = ELyraNPCTaskPriority::Critical;
    Sleep.bMandatory = true;
    Schedule->AddScheduleBlock(Sleep);
}
```

### Setting Up Patrol Routes

```cpp
void ConfigureGuardPatrol(ALyraNPCCharacter* Guard)
{
    ULyraNPCPathFollowingComponent* PathComp = Guard->PathFollowingComponent;
    if (!PathComp) return;

    PathComp->ClearPath();

    // Gate post
    FLyraNPCPathPoint GatePost;
    GatePost.Location = FVector(0, 0, 0);
    GatePost.WaitTime = 30.0f;  // Wait 30 seconds
    GatePost.MovementStyle = ELyraNPCMovementStyle::Walk;
    GatePost.AcceptanceRadius = 50.0f;
    PathComp->AddPathPoint(GatePost);

    // Wall corner 1
    FLyraNPCPathPoint Corner1;
    Corner1.Location = FVector(1000, 0, 0);
    Corner1.WaitTime = 10.0f;
    Corner1.MovementStyle = ELyraNPCMovementStyle::Walk;
    PathComp->AddPathPoint(Corner1);

    // Tower
    FLyraNPCPathPoint Tower;
    Tower.Location = FVector(1000, 1000, 0);
    Tower.WaitTime = 60.0f;  // Longer observation post
    Tower.MovementStyle = ELyraNPCMovementStyle::Walk;
    Tower.ActionAtPoint = FGameplayTag::RequestGameplayTag(TEXT("Action.Investigate"));
    PathComp->AddPathPoint(Tower);

    // Wall corner 2
    FLyraNPCPathPoint Corner2;
    Corner2.Location = FVector(0, 1000, 0);
    Corner2.WaitTime = 10.0f;
    Corner2.MovementStyle = ELyraNPCMovementStyle::Walk;
    PathComp->AddPathPoint(Corner2);

    // Configure patrol behavior
    PathComp->bLoopPath = true;  // Continuous patrol
    PathComp->bPingPongPath = false;
    PathComp->WaitTimeVariance = 0.3f;  // 30% random variance
    PathComp->bCanBeInterrupted = true;  // Can respond to threats

    // Start patrolling
    PathComp->StartFollowingPath();
}
```

### Querying NPCs

```cpp
void AMyGameManager::UpdateNPCTracking()
{
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();
    if (!Subsystem) return;

    // Get all NPCs
    TArray<ALyraNPCCharacter*> AllNPCs = Subsystem->GetAllNPCs();
    UE_LOG(LogTemp, Log, TEXT("Total NPCs: %d"), AllNPCs.Num());

    // Get NPCs near player
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            TArray<ALyraNPCCharacter*> NearbyNPCs = Subsystem->GetNPCsInRadius(
                PlayerPawn->GetActorLocation(),
                5000.0f
            );
            UE_LOG(LogTemp, Log, TEXT("NPCs within 5000 units: %d"), NearbyNPCs.Num());
        }
    }

    // Get all guards
    TArray<ALyraNPCCharacter*> Guards = Subsystem->GetNPCsByArchetype(ELyraNPCArchetype::Guard);

    // Get statistics
    float AvgWellbeing = Subsystem->GetAverageNPCWellbeing();
    int32 InCombat = Subsystem->GetNPCsInCombatCount();
    int32 FullLOD = Subsystem->GetNPCCountByLOD(ELyraNPCAILOD::Full);
    int32 Dormant = Subsystem->GetNPCCountByLOD(ELyraNPCAILOD::Dormant);

    UE_LOG(LogTemp, Log, TEXT("Avg Wellbeing: %.1f%%, In Combat: %d, Full LOD: %d, Dormant: %d"),
        AvgWellbeing, InCombat, FullLOD, Dormant);
}
```

### Managing Relationships

```cpp
void CreateFriendship(ALyraNPCCharacter* NPC1, ALyraNPCCharacter* NPC2)
{
    ULyraNPCSocialComponent* Social1 = NPC1->SocialComponent;
    ULyraNPCSocialComponent* Social2 = NPC2->SocialComponent;

    if (Social1 && Social2)
    {
        // Create mutual friendship
        Social1->AddRelationship(NPC2, ELyraNPCRelationshipType::Friend);
        Social2->AddRelationship(NPC1, ELyraNPCRelationshipType::Friend);

        // They've had positive interactions
        FGuid NPC2Id = NPC2->IdentityComponent->GetUniqueId();
        FGuid NPC1Id = NPC1->IdentityComponent->GetUniqueId();

        Social1->OnPositiveInteraction(NPC2Id, 2.0f);  // Strong positive
        Social2->OnPositiveInteraction(NPC1Id, 2.0f);
    }
}

void CreateFamilyBond(ALyraNPCCharacter* Parent, ALyraNPCCharacter* Child)
{
    ULyraNPCSocialComponent* ParentSocial = Parent->SocialComponent;
    ULyraNPCSocialComponent* ChildSocial = Child->SocialComponent;

    if (ParentSocial && ChildSocial)
    {
        ParentSocial->AddRelationship(Child, ELyraNPCRelationshipType::Family);
        ChildSocial->AddRelationship(Parent, ELyraNPCRelationshipType::Family);
    }
}

void CheckRelationshipStatus(ALyraNPCCharacter* NPC)
{
    ULyraNPCSocialComponent* Social = NPC->SocialComponent;
    if (!Social) return;

    // Get friends
    TArray<FGuid> Friends = Social->GetFriends();
    UE_LOG(LogTemp, Log, TEXT("%s has %d friends"), *NPC->GetNPCName(), Friends.Num());

    // Get best friend
    FGuid BestFriendId = Social->GetBestFriend();
    if (BestFriendId.IsValid())
    {
        if (ULyraNPCWorldSubsystem* Subsystem = NPC->GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>())
        {
            if (ALyraNPCCharacter* BestFriend = Subsystem->FindNPCById(BestFriendId))
            {
                float Affinity = Social->GetAffinityWith(BestFriendId);
                UE_LOG(LogTemp, Log, TEXT("Best friend: %s (Affinity: %.1f)"),
                    *BestFriend->GetNPCName(), Affinity);
            }
        }
    }

    // Check social decisions
    for (const FGuid& FriendId : Friends)
    {
        bool WouldHelp = Social->WouldHelpNPC(FriendId);
        bool WouldFight = Social->WouldFightForNPC(FriendId);
        bool WouldTrust = Social->WouldTrustNPC(FriendId);

        UE_LOG(LogTemp, Log, TEXT("Would help: %s, fight for: %s, trust: %s"),
            WouldHelp ? TEXT("Yes") : TEXT("No"),
            WouldFight ? TEXT("Yes") : TEXT("No"),
            WouldTrust ? TEXT("Yes") : TEXT("No"));
    }
}
```

### Handling Combat

```cpp
void OnNPCAttacked(ALyraNPCCharacter* Victim, AActor* Attacker, float Damage)
{
    // Apply damage
    Victim->TakeDamage(Damage, Attacker);

    // Check if still alive
    if (!Victim->IsAlive())
    {
        UE_LOG(LogTemp, Log, TEXT("%s was killed"), *Victim->GetNPCName());
        return;
    }

    // Victim's cognitive component handles alertness automatically
    // But we can add memory manually
    if (ULyraNPCCognitiveComponent* Cognitive = Victim->CognitiveComponent)
    {
        // Remember the attacker
        Cognitive->AddSimpleMemory(
            FGameplayTag::RequestGameplayTag(TEXT("Memory.Event.Combat")),
            FString::Printf(TEXT("Was attacked by %s"), *Attacker->GetName()),
            Attacker->GetActorLocation(),
            90.0f  // High importance
        );
    }

    // Alert nearby NPCs
    if (ULyraNPCWorldSubsystem* Subsystem = Victim->GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>())
    {
        TArray<ALyraNPCCharacter*> NearbyNPCs = Subsystem->GetNPCsInRadius(
            Victim->GetActorLocation(),
            3000.0f
        );

        for (ALyraNPCCharacter* NPC : NearbyNPCs)
        {
            if (NPC == Victim) continue;

            if (ULyraNPCCognitiveComponent* Cognitive = NPC->CognitiveComponent)
            {
                // Increase their alertness
                Cognitive->IncreaseAlertness(0.3f);

                // Friends become alert faster
                if (ULyraNPCSocialComponent* Social = NPC->SocialComponent)
                {
                    FGuid VictimId = Victim->IdentityComponent->GetUniqueId();
                    if (Social->WouldFightForNPC(VictimId))
                    {
                        Cognitive->SetAlertLevel(ELyraNPCAlertLevel::Combat);
                    }
                }
            }
        }
    }
}
```

### Time Management

```cpp
void AMyGameMode::ManageWorldTime()
{
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();
    if (!Subsystem) return;

    // Set time to noon
    Subsystem->SetGlobalGameHour(12.0f);

    // Get current time as string
    FString TimeStr = Subsystem->GetTimeString();  // "12:00"

    // Check day/night
    bool bIsNight = Subsystem->IsGlobalNightTime();  // false

    // Set time scale
    // 1.0 = real time
    // 24.0 = 1 real hour = 1 game day (default)
    // 60.0 = 1 real minute = 1 game hour
    Subsystem->SetTimeScale(60.0f);

    // Advance time manually (hours)
    Subsystem->AdvanceGlobalTime(2.0f);  // Jump forward 2 hours
}
```

---

## Blueprint Integration

### Creating a Blueprint NPC

1. **Create Blueprint Class**
   - Right-click in Content Browser → Blueprint Class
   - Select `LyraNPCCharacter` as parent
   - Name it (e.g., `BP_VillagerNPC`)

2. **Configure Defaults**
   - Open Blueprint
   - In Details panel, set:
     - Initial Archetype: Villager
     - Initial Cognitive Skill: 0.5
     - Auto Initialize: true

3. **Assign AI Controller**
   - Create another Blueprint inheriting from `LyraNPCAIController`
   - In your NPC Blueprint, set AIControllerClass to your new controller
   - Or use the C++ `ALyraNPCAIController` directly

### Blueprint Event Graph Examples

**On NPC Spawned - Custom Initialization**

```
Event BeginPlay
  → Branch (Auto Initialize == false)
    → True:
      → Set Initial Archetype (Get from Data Table or Variable)
      → Set Initial Cognitive Skill (Random Float in Range 0.3 - 0.8)
      → Initialize NPC
      → Get NPC World Subsystem
      → Register NPC (self)
```

**Checking NPC State**

```
Custom Event: CheckNPCStatus
  → Get NPC Name → Print String
  → Get Archetype → Print String
  → Get Life State → Get Life State Name → Print String
  → Get Alert Level → Get Alert Level Name → Print String
  → Get Cognitive Skill → Print String
  → Get Overall Wellbeing → Print String (format: "Wellbeing: {0}%")
```

**Responding to Needs**

```
Event Tick (or Timer)
  → Get Component (NeedsComponent)
  → Has Critical Need?
    → True:
      → Get Most Urgent Need
      → Switch on Need Type
        → Hunger: → Find and Move to Food Task
        → Energy: → Find and Move to Bed
        → Safety: → Flee from Danger
        → ... other needs
```

**Setting Up Patrol in Blueprint**

```
Event BeginPlay
  → Get Component (PathFollowingComponent)
  → Make Array of Path Points:
    → Make LyraNPCPathPoint:
      → Location: (0, 0, 0)
      → Wait Time: 30.0
      → Movement Style: Walk
    → Make LyraNPCPathPoint:
      → Location: (1000, 0, 0)
      → Wait Time: 10.0
    → (add more points)
  → Set Path Points
  → Set Loop Path: true
  → Start Following Path
```

**Blueprint Function Library Usage**

```
// Get all guards in area
Get NPCs by Archetype (Guard)
  → For Each Loop
    → Get NPC Name
    → Get Alert Level
    → Print Status

// Spawn NPC at runtime
Spawn NPC (
  World Context: self,
  NPC Class: BP_GuardNPC,
  Location: GetActorLocation,
  Rotation: GetActorRotation,
  Archetype: Guard,
  Cognitive Skill: 0.75
) → Store as variable

// Time queries
Get Global Game Time → Format as int (hour)
Get Global Time String → Display on UI
Is Global Night Time? → Update lighting
```

### Creating Task Actors in Blueprint

1. **Create Actor Blueprint**
   - Create new Actor Blueprint (e.g., `BP_Bed`)

2. **Add Task Component**
   - Add Component → LyraNPCTaskActor
   - Configure in Details panel:
     - Task Type: Task.Sleep
     - Task Name: "Bed"
     - Max Users: 1
     - Default Duration: 480 (8 hours in seconds)
     - Min Duration: 360
     - Max Duration: 600

3. **Configure Needs Satisfaction**
   - In Details panel, expand Needs Satisfaction map
   - Add entry: Energy → 60.0
   - Add entry: Comfort → 30.0

4. **Add Interaction Point**
   - Clear default interaction points
   - Add new entry with Transform
   - Set location relative to actor

5. **Register in Begin Play**
   ```
   Event BeginPlay
     → Get NPC World Subsystem
     → Register Task Actor (self.NPCTaskComponent)
   ```

### Blueprint AI Controller Setup

1. **Create Controller Blueprint**
   - Parent: LyraNPCAIController

2. **Assign Behavior Tree**
   - In Details panel, set Default Behavior Tree

3. **Custom LOD Settings (Optional)**
   ```
   Construction Script:
     → Set Full LOD Distance: 3000.0
     → Set Reduced LOD Distance: 7000.0
     → Set Minimal LOD Distance: 15000.0
   ```

4. **Listen to Events**
   ```
   Event BeginPlay
     → Bind Event to On NPC Life State Changed
     → Bind Event to On AI LOD Changed
     → Bind Event to On NPC Need Critical

   Custom Event: On Life State Changed (NPC, New State)
     → Print: "NPC changed to state: {New State}"

   Custom Event: On Need Critical (NPC, Need Type)
     → Get Need Type Name → Print
     → Force NPC to address need
   ```

---

## Common Scenarios

### Scenario 1: Village with Day/Night Cycle

**Goal**: NPCs go to work during day, eat meals, socialize in evening, sleep at night.

**Setup**:

```cpp
void SetupVillage()
{
    // Set time scale: 1 real minute = 1 game hour
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();
    Subsystem->SetTimeScale(60.0f);
    Subsystem->SetGlobalGameHour(6.0f);  // Start at 6 AM

    // Spawn villagers with default schedules
    for (int32 i = 0; i < 10; ++i)
    {
        ALyraNPCCharacter* Villager = ULyraNPCFunctionLibrary::SpawnNPC(
            this, ALyraNPCCharacter::StaticClass(),
            GetRandomSpawnPoint(),
            FRotator::ZeroRotator,
            ELyraNPCArchetype::Villager,
            FMath::FRandRange(0.3f, 0.7f)
        );
        Subsystem->RegisterNPC(Villager);
    }

    // Place task actors in world
    // - Beds in houses
    // - Workbenches in workshops
    // - Tables in tavern
    // - Chairs for resting
}
```

**Result**: NPCs will automatically follow their schedules, finding appropriate tasks based on time of day and needs.

### Scenario 2: Guard Patrol System

**Goal**: Guards patrol predetermined routes, become alert when detecting threats.

```cpp
void SetupGuardPatrol()
{
    // Spawn guard
    ALyraNPCCharacter* Guard = ULyraNPCFunctionLibrary::SpawnNPC(
        this, ALyraNPCCharacter::StaticClass(),
        PatrolStartPoint,
        FRotator::ZeroRotator,
        ELyraNPCArchetype::Guard,
        0.8f  // Smart guard
    );

    // Configure patrol route
    ULyraNPCPathFollowingComponent* PathComp = Guard->PathFollowingComponent;

    // Add patrol points (your actual world locations)
    TArray<FVector> PatrolPoints = GetPatrolRoute();
    for (const FVector& Point : PatrolPoints)
    {
        FLyraNPCPathPoint PathPoint;
        PathPoint.Location = Point;
        PathPoint.WaitTime = 20.0f;
        PathPoint.MovementStyle = ELyraNPCMovementStyle::Walk;
        PathComp->AddPathPoint(PathPoint);
    }

    PathComp->bLoopPath = true;
    PathComp->StartFollowingPath();

    // Tag player or enemies as threats
    PlayerCharacter->Tags.Add(TEXT("Threat"));

    // Guard will automatically increase alertness when seeing threats
}
```

### Scenario 3: Dynamic Difficulty Based on NPC Intelligence

**Goal**: Adjust game difficulty by modifying NPC cognitive skills.

```cpp
void SetDifficulty(EDifficulty Difficulty)
{
    float MinCognitiveSkill, MaxCognitiveSkill;

    switch (Difficulty)
    {
    case EDifficulty::Easy:
        MinCognitiveSkill = 0.1f;
        MaxCognitiveSkill = 0.4f;
        break;
    case EDifficulty::Normal:
        MinCognitiveSkill = 0.3f;
        MaxCognitiveSkill = 0.7f;
        break;
    case EDifficulty::Hard:
        MinCognitiveSkill = 0.6f;
        MaxCognitiveSkill = 0.95f;
        break;
    }

    // Update all enemy NPCs
    TArray<ALyraNPCCharacter*> Enemies = ULyraNPCFunctionLibrary::GetNPCsByArchetype(
        this, ELyraNPCArchetype::Enemy
    );

    for (ALyraNPCCharacter* Enemy : Enemies)
    {
        float NewSkill = FMath::FRandRange(MinCognitiveSkill, MaxCognitiveSkill);
        Enemy->CognitiveComponent->CognitiveSkill = NewSkill;

        // This affects:
        // - Combat accuracy (dumb enemies miss more)
        // - Reaction time (dumb enemies react slower)
        // - Tactical decisions (dumb enemies don't flank)
        // - Perception (dumb enemies notice less)
        // - Movement (dumb enemies move erratically)
    }
}
```

### Scenario 4: NPC Conversations and Social Events

**Goal**: NPCs meet at tavern, form friendships based on personality compatibility.

```cpp
void SimulateSocialEvent()
{
    // Get NPCs at tavern
    FVector TavernLocation = GetTavernLocation();
    TArray<ALyraNPCCharacter*> NPCsAtTavern = ULyraNPCFunctionLibrary::GetNPCsInRadius(
        this, TavernLocation, 500.0f
    );

    // Each NPC interacts with others
    for (int32 i = 0; i < NPCsAtTavern.Num(); ++i)
    {
        for (int32 j = i + 1; j < NPCsAtTavern.Num(); ++j)
        {
            ALyraNPCCharacter* NPC1 = NPCsAtTavern[i];
            ALyraNPCCharacter* NPC2 = NPCsAtTavern[j];

            // Check personality compatibility
            float Compatibility = ULyraNPCFunctionLibrary::GetPersonalityCompatibility(
                NPC1->IdentityComponent->GetPersonality(),
                NPC2->IdentityComponent->GetPersonality()
            );

            // Add relationships if not exists
            FGuid NPC1Id = NPC1->IdentityComponent->GetUniqueId();
            FGuid NPC2Id = NPC2->IdentityComponent->GetUniqueId();

            if (!NPC1->SocialComponent->HasRelationship(NPC2Id))
            {
                NPC1->SocialComponent->AddRelationship(NPC2);
                NPC2->SocialComponent->AddRelationship(NPC1);
            }

            // Interaction based on compatibility
            if (Compatibility > 70.0f)
            {
                // Great conversation
                NPC1->SocialComponent->OnPositiveInteraction(NPC2Id, 1.5f);
                NPC2->SocialComponent->OnPositiveInteraction(NPC1Id, 1.5f);
            }
            else if (Compatibility > 40.0f)
            {
                // Neutral interaction
                NPC1->SocialComponent->OnNeutralInteraction(NPC2Id);
                NPC2->SocialComponent->OnNeutralInteraction(NPC1Id);
            }
            else
            {
                // They don't get along
                NPC1->SocialComponent->OnNegativeInteraction(NPC2Id, 0.5f);
                NPC2->SocialComponent->OnNegativeInteraction(NPC1Id, 0.5f);
            }

            // Satisfy social needs
            NPC1->NeedsComponent->SatisfyNeed(ELyraNPCNeedType::Social, 10.0f);
            NPC2->NeedsComponent->SatisfyNeed(ELyraNPCNeedType::Social, 10.0f);
        }
    }
}
```

### Scenario 5: Emergency Response

**Goal**: When danger occurs, NPCs react based on their archetype and personality.

```cpp
void OnDangerDetected(FVector DangerLocation, float ThreatLevel)
{
    TArray<ALyraNPCCharacter*> AllNPCs = ULyraNPCFunctionLibrary::GetAllNPCs(this);

    for (ALyraNPCCharacter* NPC : AllNPCs)
    {
        float Distance = FVector::Dist(NPC->GetActorLocation(), DangerLocation);
        if (Distance > 5000.0f) continue;  // Too far to notice

        // Check if NPC notices danger (affected by intelligence)
        float NoticeChance = NPC->CognitiveComponent->GetNoticeChance(0.5f);
        NoticeChance *= (1.0f - (Distance / 5000.0f));  // Closer = more likely

        if (FMath::FRand() > NoticeChance)
        {
            continue;  // Didn't notice
        }

        // Set alert level
        NPC->CognitiveComponent->IncreaseAlertness(ThreatLevel);

        // Remember danger location
        NPC->CognitiveComponent->AddSimpleMemory(
            FGameplayTag::RequestGameplayTag(TEXT("Memory.Location.Threat")),
            TEXT("Danger detected"),
            DangerLocation,
            80.0f
        );

        // Reduce safety need
        NPC->NeedsComponent->ModifyNeed(ELyraNPCNeedType::Safety, -30.0f * ThreatLevel);

        // Response based on archetype and personality
        switch (NPC->GetArchetype())
        {
        case ELyraNPCArchetype::Guard:
            // Guards investigate/fight
            NPC->IdentityComponent->SetLifeState(ELyraNPCLifeState::Investigating);
            // Move towards danger
            break;

        case ELyraNPCArchetype::Villager:
        case ELyraNPCArchetype::Worker:
            // Civilians flee based on bravery
            if (!NPC->IdentityComponent->WouldMakeRiskyDecision(ThreatLevel))
            {
                NPC->IdentityComponent->SetLifeState(ELyraNPCLifeState::Fleeing);
                // Move away from danger
            }
            break;

        case ELyraNPCArchetype::Merchant:
            // Protect shop
            NPC->IdentityComponent->SetLifeState(ELyraNPCLifeState::Alert);
            break;
        }
    }
}
```

### Scenario 6: Resource Scarcity

**Goal**: When food becomes scarce, NPCs compete for limited resources.

```cpp
void OnFoodShortage()
{
    // Reduce all food task satisfaction rates
    TArray<ULyraNPCTaskActor*> AllTasks = ULyraNPCFunctionLibrary::GetAllTasks(this);

    for (ULyraNPCTaskActor* Task : AllTasks)
    {
        if (Task->TaskType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Task.Eat"))))
        {
            // Reduce hunger satisfaction
            float* HungerSatisfaction = Task->NeedsSatisfaction.Find(ELyraNPCNeedType::Hunger);
            if (HungerSatisfaction)
            {
                *HungerSatisfaction *= 0.5f;  // Half as effective
            }

            // Reduce capacity
            Task->MaxUsers = FMath::Max(1, Task->MaxUsers - 1);
        }
    }

    // NPCs will now:
    // - Have higher hunger priority (competing for limited food)
    // - Make decisions based on personality (aggressive NPCs might hoard)
    // - Remember where food was found (memory system)

    // Monitor social tension
    float AvgWellbeing = ULyraNPCFunctionLibrary::GetAverageNPCWellbeing(this);
    if (AvgWellbeing < 40.0f)
    {
        // Population is stressed, increase social conflicts
    }
}
```

### Scenario 7: Player Reputation System

**Goal**: NPCs remember player actions and adjust relationships accordingly.

```cpp
void OnPlayerHelpedNPC(ALyraNPCCharacter* NPC)
{
    // NPC remembers being helped
    NPC->CognitiveComponent->AddSimpleMemory(
        FGameplayTag::RequestGameplayTag(TEXT("Memory.Person.Friend")),
        TEXT("Player helped me"),
        NPC->GetActorLocation(),
        75.0f
    );

    // Would this NPC share the good news? (based on extraversion)
    if (NPC->IdentityComponent->PrefersGroup())
    {
        // Tell friends
        TArray<FGuid> Friends = NPC->SocialComponent->GetFriends();
        ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();

        for (const FGuid& FriendId : Friends)
        {
            if (ALyraNPCCharacter* Friend = Subsystem->FindNPCById(FriendId))
            {
                // Friend hears good things about player
                Friend->CognitiveComponent->AddSimpleMemory(
                    FGameplayTag::RequestGameplayTag(TEXT("Memory.Person.Friend")),
                    TEXT("Heard player is helpful"),
                    Friend->GetActorLocation(),
                    50.0f  // Second-hand info, less important
                );
            }
        }
    }
}

void OnPlayerAttackedNPC(ALyraNPCCharacter* Victim)
{
    // Victim remembers attack
    Victim->CognitiveComponent->AddSimpleMemory(
        FGameplayTag::RequestGameplayTag(TEXT("Memory.Person.Enemy")),
        TEXT("Player attacked me!"),
        Victim->GetActorLocation(),
        95.0f  // Very important memory
    );

    // Alert nearby NPCs
    TArray<ALyraNPCCharacter*> Witnesses = ULyraNPCFunctionLibrary::GetNPCsInRadius(
        this, Victim->GetActorLocation(), 2000.0f
    );

    for (ALyraNPCCharacter* Witness : Witnesses)
    {
        if (Witness == Victim) continue;

        // Did they notice? (based on perception)
        float NoticeChance = Witness->CognitiveComponent->GetNoticeChance(0.3f);
        if (FMath::FRand() < NoticeChance)
        {
            Witness->CognitiveComponent->AddSimpleMemory(
                FGameplayTag::RequestGameplayTag(TEXT("Memory.Person.Enemy")),
                TEXT("Saw player attack someone"),
                Victim->GetActorLocation(),
                85.0f
            );

            // Guards respond more aggressively
            if (Witness->GetArchetype() == ELyraNPCArchetype::Guard)
            {
                Witness->CognitiveComponent->SetAlertLevel(ELyraNPCAlertLevel::Combat);
            }
        }
    }
}
```

---

## Advanced Usage

### Custom Behavior Tree for Complex NPC

Create a Behavior Tree asset that uses the LyraNPC tasks:

```
Root
├── Selector (Main Decision)
│   ├── Sequence (Critical Needs)
│   │   ├── Decorator: Check Need (Safety < 30)
│   │   └── Task: Flee to Safety
│   │
│   ├── Sequence (Combat)
│   │   ├── Decorator: Has Threats
│   │   └── Selector (Combat Response)
│   │       ├── Sequence (Fight - Guards)
│   │       │   ├── Decorator: Is Guard Archetype
│   │       │   └── Task: Engage Threat
│   │       └── Sequence (Flee - Civilians)
│   │           └── Task: Flee from Threat
│   │
│   ├── Sequence (Scheduled Activities)
│   │   ├── Service: Update NPC State
│   │   ├── Task: Find Best Task (use schedule)
│   │   ├── Task: Move to Task Location
│   │   └── Task: Use Task
│   │
│   ├── Sequence (Urgent Needs)
│   │   ├── Decorator: Has Urgent Need
│   │   ├── Task: Find Task for Need
│   │   ├── Task: Move to Task
│   │   └── Task: Use Task
│   │
│   ├── Sequence (Patrol - Guards)
│   │   ├── Decorator: Is Guard AND Has Patrol Path
│   │   └── Task: Follow Path
│   │
│   └── Task: Idle/Wander
```

### Custom EQS Query

Create Environment Query asset for finding optimal task:

1. Create new EnvQuery asset
2. Add Generator: LyraNPC Tasks Generator
   - Task Type Filter: (leave empty for all, or set specific type)
   - Search Radius: 10000
   - Only Available: true
   - Check NPC Access: true
   - Max Results: 20

3. Add Tests:
   - Distance: Score (closer = better)
   - Dot Product: Facing direction (prefer tasks in front)
   - Custom: Need satisfaction score

### Extending Components

**Custom Need Type**:

```cpp
// In your game, extend the needs system
void AddCustomNeed(ALyraNPCCharacter* NPC)
{
    FLyraNPCNeedState MagicNeed;
    MagicNeed.NeedType = ELyraNPCNeedType::Custom;  // Add custom enum value
    MagicNeed.CurrentValue = 100.0f;
    MagicNeed.DecayRatePerHour = 5.0f;
    MagicNeed.PriorityWeight = 1.0f;
    MagicNeed.UrgentThreshold = 30.0f;
    MagicNeed.CriticalThreshold = 10.0f;

    NPC->NeedsComponent->Needs.Add(MagicNeed);
}
```

**Custom Memory Types**:

Add new tags to your GameplayTags config:
```ini
+GameplayTagList=(Tag="Memory.Spell.Learned",DevComment="Learned a new spell")
+GameplayTagList=(Tag="Memory.Quest.Started",DevComment="Started a quest")
```

Then use:
```cpp
NPC->CognitiveComponent->AddSimpleMemory(
    FGameplayTag::RequestGameplayTag(TEXT("Memory.Spell.Learned")),
    TEXT("Learned Fireball spell"),
    SpellbookLocation,
    70.0f
);
```

---

## Performance Tuning

### Optimizing for Large NPC Counts

```cpp
void OptimizeForMassivePopulation()
{
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();

    // Reduce LOD distances for aggressive optimization
    for (ALyraNPCCharacter* NPC : Subsystem->GetAllNPCs())
    {
        if (ALyraNPCAIController* Controller = Cast<ALyraNPCAIController>(NPC->GetController()))
        {
            Controller->FullLODDistance = 1000.0f;     // Only full AI very close
            Controller->ReducedLODDistance = 3000.0f;  // Reduced AI medium distance
            Controller->MinimalLODDistance = 8000.0f;  // Minimal AI far
            // Beyond 8000 units: Dormant
        }

        // Reduce tick rates
        NPC->NeedsComponent->SetComponentTickInterval(2.0f);  // Update every 2 seconds
        NPC->ScheduleComponent->SetComponentTickInterval(2.0f);
        NPC->SocialComponent->SetComponentTickInterval(30.0f);  // Social is less critical
    }

    // Disable needs simulation for very distant NPCs
    for (ALyraNPCCharacter* NPC : Subsystem->GetAllNPCs())
    {
        if (ALyraNPCAIController* Controller = Cast<ALyraNPCAIController>(NPC->GetController()))
        {
            if (Controller->CurrentAILOD == ELyraNPCAILOD::Dormant)
            {
                NPC->NeedsComponent->bSimulateNeeds = false;
            }
        }
    }
}
```

### Memory Management

```cpp
void OptimizeMemory()
{
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();

    for (ALyraNPCCharacter* NPC : Subsystem->GetAllNPCs())
    {
        // Reduce memory capacity for dumb NPCs
        if (NPC->CognitiveComponent->CognitiveSkill < 0.3f)
        {
            NPC->CognitiveComponent->MaxMemories = 10;  // Very limited memory
        }

        // Increase memory decay
        NPC->CognitiveComponent->MemoryDecayRate = 5.0f;  // Forget faster

        // Limit relationship count
        if (NPC->SocialComponent->Relationships.Num() > 20)
        {
            // Remove oldest, least important relationships
            // (implement cleanup logic)
        }
    }
}
```

### Task Actor Pooling

```cpp
void SetupTaskPool()
{
    // Pre-register all task actors at level start
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();

    for (AActor* Actor : AllActors)
    {
        TArray<ULyraNPCTaskActor*> TaskComponents;
        Actor->GetComponents<ULyraNPCTaskActor>(TaskComponents);

        for (ULyraNPCTaskActor* TaskComp : TaskComponents)
        {
            Subsystem->RegisterTaskActor(TaskComp);
        }
    }

    // Now all task queries use the subsystem's optimized pool
    // instead of searching the entire world
}
```

---

## Troubleshooting

### NPC Not Moving

**Check**:
1. AI Controller is assigned and possessing pawn
2. Navigation mesh exists in level
3. Character movement component is enabled
4. Blackboard has valid target location

```cpp
void DebugNPCMovement(ALyraNPCCharacter* NPC)
{
    AAIController* Controller = Cast<AAIController>(NPC->GetController());
    if (!Controller)
    {
        UE_LOG(LogTemp, Error, TEXT("No AI Controller!"));
        return;
    }

    UPathFollowingComponent* PathComp = Controller->GetPathFollowingComponent();
    if (PathComp)
    {
        EPathFollowingStatus::Type Status = PathComp->GetStatus();
        UE_LOG(LogTemp, Log, TEXT("Path Status: %d"), static_cast<int32>(Status));
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        UE_LOG(LogTemp, Error, TEXT("No navigation system!"));
    }
}
```

### NPC Not Using Tasks

**Check**:
1. Task actors are registered with world subsystem
2. Task is enabled and available
3. NPC has correct archetype/tags
4. Task is within search radius

```cpp
void DebugTaskUsage(ALyraNPCCharacter* NPC)
{
    ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();
    TArray<ULyraNPCTaskActor*> AvailableTasks = Subsystem->GetAvailableTasksForNPC(NPC);

    UE_LOG(LogTemp, Log, TEXT("Available tasks for %s: %d"), *NPC->GetNPCName(), AvailableTasks.Num());

    for (ULyraNPCTaskActor* Task : AvailableTasks)
    {
        float Score = Task->GetScoreForNPC(NPC);
        UE_LOG(LogTemp, Log, TEXT("  - %s (Score: %.2f)"), *Task->TaskName, Score);
    }
}
```

### Schedule Not Working

**Check**:
1. Time scale is not zero
2. Schedule blocks don't overlap incorrectly
3. Activity tags match task types

```cpp
void DebugSchedule(ALyraNPCCharacter* NPC)
{
    ULyraNPCScheduleComponent* Schedule = NPC->ScheduleComponent;

    UE_LOG(LogTemp, Log, TEXT("Current hour: %.2f"), Schedule->GetCurrentGameHour());

    FLyraNPCScheduleBlock CurrentBlock = Schedule->GetCurrentScheduledActivity();
    UE_LOG(LogTemp, Log, TEXT("Current activity: %s at %s"),
        *CurrentBlock.ActivityTag.ToString(),
        *CurrentBlock.LocationName.ToString());

    UE_LOG(LogTemp, Log, TEXT("Time scale: %.1f"), Schedule->TimeScale);
}
```

### Cognitive Skill Not Affecting Behavior

**Check**:
1. CognitiveComponent exists and initialized
2. Behavior Tree uses cognitive modifiers
3. Combat/perception systems check cognitive values

```cpp
void DebugCognitive(ALyraNPCCharacter* NPC)
{
    ULyraNPCCognitiveComponent* Cog = NPC->CognitiveComponent;

    UE_LOG(LogTemp, Log, TEXT("Cognitive Skill: %.2f"), Cog->CognitiveSkill);
    UE_LOG(LogTemp, Log, TEXT("Perception Radius Modifier: %.2f"), Cog->GetPerceptionRadiusModifier());
    UE_LOG(LogTemp, Log, TEXT("Combat Accuracy: %.2f"), Cog->GetCombatAccuracyModifier());
    UE_LOG(LogTemp, Log, TEXT("Decision Quality: %.2f"), Cog->GetDecisionQuality());
    UE_LOG(LogTemp, Log, TEXT("Planning Depth: %d"), Cog->GetPlanningDepth());

    // Test mistake probability
    for (int32 i = 0; i < 10; ++i)
    {
        bool WillMistake = Cog->WillMakeMistake(0.5f);
        UE_LOG(LogTemp, Log, TEXT("Mistake test %d: %s"), i, WillMistake ? TEXT("Yes") : TEXT("No"));
    }
}
```

### Common Issues and Solutions

| Issue | Possible Cause | Solution |
|-------|---------------|----------|
| NPC stands still | No Behavior Tree assigned | Assign BT to AI Controller |
| Tasks not found | Not registered | Call `RegisterTaskActor()` in BeginPlay |
| Schedule jumps | Time scale too high | Reduce `TimeScale` |
| Memory full quickly | Low `MaxMemories` | Increase based on intelligence |
| Relationships decay fast | High `DecayRate` | Reduce `AffinityChangeRate` |
| Combat too easy/hard | Wrong cognitive skill | Adjust `InitialCognitiveSkill` |
| LOD changes too often | Distance thresholds wrong | Adjust LOD distance values |

---

This guide should provide comprehensive coverage of the LyraNPC framework. For specific scenarios not covered here, examine the source code comments and Blueprint tooltips, which provide additional context for each function and property.

**Key Files for Reference**:
- `LyraNPCTypes.h` - All data structures and enums
- `LyraNPCFunctionLibrary.h` - Blueprint/C++ utility functions
- `LyraNPCAIController.h` - Main AI orchestration
- `LyraNPCWorldSubsystem.h` - Global management
