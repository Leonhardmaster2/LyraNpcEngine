# LyraNPC - Living NPC AI Framework for Unreal Engine 5

The most comprehensive and intelligent NPC AI system for creating living, breathing game worlds. NPCs with individual personalities, daily routines, needs, relationships, and context-aware decision-making.

## Features

### Core Systems

- **Cognitive Skill System** - NPCs with configurable intelligence levels (0-1 scale) that affects:
  - Perception accuracy and range
  - Decision-making quality
  - Movement precision (smart NPCs move efficiently, dumb ones wobble)
  - Combat accuracy and tactics
  - Memory capacity and retention
  - Mistake probability

- **Identity & Biography** - Full NPC characterization:
  - Unique names, ages, occupations
  - Big Five personality traits (Openness, Conscientiousness, Extraversion, Agreeableness, Neuroticism)
  - Additional traits (Bravery, Honesty, Curiosity, Patience, Loyalty)
  - Character tags for custom attributes
  - Emotional states

- **Needs & Motivation System** - Realistic need simulation:
  - Hunger, Energy, Social, Safety, Comfort, Entertainment, Purpose
  - Automatic decay over game time
  - Priority-based action selection
  - Urgent and critical thresholds

- **Daily Schedule System** - Time-based routines:
  - Configurable schedule blocks (work, eat, sleep, leisure)
  - Flexible vs mandatory activities
  - Automatic schedule generation per archetype

- **Task Interaction System** - World object interaction:
  - Task Actors (beds, workbenches, chairs, forges, etc.)
  - Reservation and capacity system
  - Need satisfaction per task
  - Role-based access control
  - Private ownership support

- **AI LOD (Level of Detail)** - Optimized performance:
  - Full AI - Complete processing for nearby NPCs
  - Reduced AI - Simplified perception and decisions
  - Minimal AI - Schedule-based only
  - Dormant - State preservation, no processing
  - Supports 100-300+ concurrent NPCs

- **Path Following** - Predetermined movement:
  - Patrol routes
  - Travel paths
  - Custom waypoints
  - Wait times and actions at points
  - Loop and ping-pong modes

- **Perception & Alertness** - Awareness system:
  - Sight and hearing senses
  - Intelligence-modified perception
  - Alert levels (Unaware → Combat)
  - Memory of threats and events

- **GOAP (Goal-Oriented Action Planning)** - Dynamic planning system:
  - A* pathfinding-based multi-step action planning
  - NPCs formulate plans to achieve complex goals
  - Cognitive skill affects planning depth (smart NPCs plan further ahead)
  - Automatic replanning when circumstances change
  - World state management with automatic updates
  - Cost-based action selection
  - Built-in actions: Eat, Sleep, Socialize, Use Task

- **Group Coordination** - Tactical group behavior:
  - Shared knowledge between group members
  - Formation-based movement (Line, Column, Wedge, Circle, Shield, Scatter)
  - Coordinated group actions and tactics
  - Emotional contagion (fear/aggression spreading through group)
  - Dynamic role assignment (Leader, Scout, Flanker, Support, etc.)
  - Group decision-making with influence voting
  - Automatic tactical adjustments (retreat, advance, hold)
  - Enemy sighting and threat reporting

- **Social Relationships** - NPC-to-NPC interactions:
  - Relationship tracking (Family, Friend, Enemy, Rival, etc.)
  - Affinity, trust, and familiarity systems
  - Relationship decay over time
  - Personality compatibility calculations
  - Social influences on behavior

## Quick Start Guide

### 1. Add to Your Project

Copy the `LyraNPC` plugin folder into your project's `Plugins` directory.

### 2. Create Your First NPC

**Option A: Blueprint (Easiest)**

1. Create a new Blueprint class inheriting from `ALyraNPCCharacter`
2. Set the `Initial Archetype` (Villager, Guard, Worker, Merchant, etc.)
3. Set the `Initial Cognitive Skill` (0.0 = very dumb, 1.0 = genius)
4. Create an AI Controller Blueprint inheriting from `ALyraNPCAIController`
5. Assign a Behavior Tree (or use the default one)
6. Place in world - the NPC works automatically!

**Option B: C++**

```cpp
// In your game module
ALyraNPCCharacter* NPC = GetWorld()->SpawnActor<ALyraNPCCharacter>(SpawnLocation, SpawnRotation);
NPC->InitialArchetype = ELyraNPCArchetype::Guard;
NPC->InitialCognitiveSkill = 0.8f; // Smart guard
NPC->InitializeNPC();
```

### 3. Add Task Actors to Your World

Add the `ULyraNPCTaskActor` component to any actor to make it usable by NPCs:

```cpp
// Example: Create a bed task
ULyraNPCTaskActor* BedTask = CreateDefaultSubobject<ULyraNPCTaskActor>(TEXT("BedTask"));
BedTask->TaskType = FGameplayTag::RequestGameplayTag(TEXT("Task.Sleep"));
BedTask->TaskName = TEXT("Bed");
BedTask->NeedsSatisfaction.Add(ELyraNPCNeedType::Energy, 50.0f); // Satisfies 50 energy per minute
BedTask->DefaultDuration = 480.0f; // 8 minutes = 8 game hours
BedTask->MaxUsers = 1;
BedTask->bIsPrivate = true; // Owned by specific NPC
```

### 4. Configure Schedules

NPCs automatically get schedules based on their archetype, but you can customize:

```cpp
// In Blueprint or C++
ULyraNPCScheduleComponent* Schedule = NPC->ScheduleComponent;

FLyraNPCScheduleBlock WorkBlock;
WorkBlock.StartHour = 8.0f;
WorkBlock.EndHour = 17.0f;
WorkBlock.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Work"));
WorkBlock.LocationName = TEXT("Blacksmith");
WorkBlock.bMandatory = true;

Schedule->AddScheduleBlock(WorkBlock);
```

### 5. Set Up Patrol Paths

```cpp
ULyraNPCPathFollowingComponent* PathComp = NPC->PathFollowingComponent;

FLyraNPCPathPoint Point1;
Point1.Location = FVector(0, 0, 0);
Point1.WaitTime = 5.0f;
Point1.MovementStyle = ELyraNPCMovementStyle::Walk;

FLyraNPCPathPoint Point2;
Point2.Location = FVector(500, 0, 0);
Point2.WaitTime = 10.0f;

PathComp->AddPathPoint(Point1);
PathComp->AddPathPoint(Point2);
PathComp->bLoopPath = true;
```

## Component Reference

### ULyraNPCIdentityComponent
- Biography (name, age, occupation, backstory)
- Personality traits
- Current emotional state
- Home and workplace locations
- Favorite places

### ULyraNPCCognitiveComponent
- Intelligence level (CognitiveSkill 0-1)
- Alert level management
- Memory system with decay
- Decision quality modifiers
- Mistake probability

### ULyraNPCNeedsComponent
- Need states (Hunger, Energy, Social, etc.)
- Automatic decay over time
- Priority calculation
- Wellbeing score

### ULyraNPCScheduleComponent
- Daily schedule blocks
- Time management
- Activity tags
- Flexibility settings

### ULyraNPCPathFollowingComponent
- Predetermined path points
- Wait times and actions
- Movement styles
- Loop/ping-pong modes

### ULyraNPCSocialComponent
- Relationship management
- Affinity and trust tracking
- Social interaction handling
- Compatibility calculations
- Friend/Enemy lists

### ULyraNPCGOAPComponent
- Goal selection and prioritization
- Action plan creation using A*
- World state management
- Plan execution and monitoring
- Automatic replanning
- Available actions library

### ULyraNPCGroupCoordinationComponent
- Group membership management
- Formation configuration
- Shared knowledge pool
- Coordinated action execution
- Group mood tracking
- Tactical decision making

## Behavior Tree Nodes

### Tasks
- **Find Best Task** - Locates optimal task based on needs/schedule
- **Use Task** - Performs task for duration with need satisfaction
- **Follow Path** - Follows predetermined patrol/travel route
- **Execute GOAP Plan** - Creates and executes multi-step GOAP plans
- **Join Group** - Finds and joins a nearby group
- **Follow Formation** - Maintains formation position within group
- **Share Group Knowledge** - Reports information to group members

### Services
- **Update NPC State** - Keeps blackboard synced with components
- **Update GOAP State** - Refreshes GOAP world state and goal priorities

### Decorators
- **Check Need** - Conditional based on need value
- **Check GOAP Goal** - Checks if specific GOAP goal is active

## EQS Integration

- **Task Generator** - Finds available tasks within radius
- Automatically scores tasks by NPC needs
- Filters by type, availability, and access

## Using GOAP (Goal-Oriented Action Planning)

NPCs with GOAPComponent automatically plan multi-step actions to achieve goals:

```cpp
// GOAP is integrated automatically - NPCs will plan based on their needs
// You can also create custom GOAP actions:

UCLASS()
class UMyCustomGOAPAction : public ULyraNPCGOAPAction
{
    GENERATED_BODY()

public:
    UMyCustomGOAPAction()
    {
        ActionName = FName("MyAction");

        // Define what must be true to execute
        Preconditions.StateFlags.Add(FName("HasTool"), true);

        // Define what becomes true after execution
        Effects.StateFlags.Add(FName("TaskComplete"), true);

        Cost.BaseCost = 5.0f;
    }

    virtual bool TickAction(ALyraNPCCharacter* NPC, float DeltaTime) override
    {
        // Execute your action logic
        return false; // Return false when complete
    }
};
```

**Accessing GOAP in Blueprints:**
- Execute GOAP Plan BT task automatically handles planning
- Check GOAP Goal decorator allows conditional branching
- Update GOAP State service keeps plans fresh

## Using Group Coordination

Create coordinated NPC groups with shared tactics:

```cpp
// Create a group coordination component on an actor (e.g., squad manager)
ULyraNPCGroupCoordinationComponent* Group = CreateDefaultSubobject<ULyraNPCGroupCoordinationComponent>(TEXT("SquadGroup"));
Group->GroupId = FName("GuardSquad01");
Group->MaxGroupSize = 8;
Group->bUseFormations = true;
Group->bShareKnowledge = true;

// NPCs can join groups via BT task or code
Group->AddMember(NPCCharacter, ELyraNPCGroupRole::Leader);

// Set formation and tactics
Group->SetFormation(ELyraNPCGroupFormation::Wedge);
Group->UpdateFormation(SquadCenter, ForwardDirection);
Group->SetTactic(ELyraNPCGroupTactic::Advance);

// Share knowledge with the group
Group->ReportEnemySighting(EnemyActor, EnemyLocation, NPCId, 0.9f);

// Check group status
if (Group->ShouldRetreat())
{
    Group->SetTactic(ELyraNPCGroupTactic::Retreat);
}
```

**Group Formations:**
- **Line** - Horizontal formation, good for defensive positions
- **Column** - Follow-the-leader, good for travel
- **Wedge** - V-shape with leader at point, good for assault
- **Circle** - Defensive perimeter
- **Shield** - Tight defensive line
- **Scatter** - Spread out, good for avoiding area attacks

**Group Roles:**
- **Leader** - Makes decisions, highest influence
- **Scout** - Explores ahead, high perception
- **Flanker** - Attacks from sides
- **Support** - Assists allies
- **HeavyHitter** - Main damage dealer
- **Defender** - Protects group
- **Medic** - Heals allies
- **Follower** - General group member

## World Subsystem

Access global NPC management through `ULyraNPCWorldSubsystem`:

```cpp
ULyraNPCWorldSubsystem* Subsystem = GetWorld()->GetSubsystem<ULyraNPCWorldSubsystem>();

// Get all NPCs
TArray<ALyraNPCCharacter*> AllNPCs = Subsystem->GetAllNPCs();

// Find NPCs near player
TArray<ALyraNPCCharacter*> NearbyNPCs = Subsystem->GetNPCsInRadius(PlayerLocation, 5000.0f);

// Global time control
Subsystem->SetGlobalGameHour(12.0f); // Set to noon
Subsystem->SetTimeScale(48.0f); // 1 real minute = 1 game hour

// Statistics
float AverageWellbeing = Subsystem->GetAverageNPCWellbeing();
int32 NPCsInCombat = Subsystem->GetNPCsInCombatCount();
```

## Performance Optimization

LyraNPC is designed for large-scale NPC populations:

1. **AI LOD System** - Automatic detail reduction based on distance
2. **Task Pooling** - Subsystem-based task lookup instead of world scans
3. **Tick Rate Management** - Adaptive update frequencies
4. **Memory Modulation** - Dumber NPCs use less processing power
5. **Compact Replication** - Only essential state replicated in multiplayer

## Multiplayer Support

- Server-authoritative AI logic
- Efficient state replication
- Compatible with dedicated and listen servers
- AI LOD reduces network traffic

## Best Practices

1. **Set appropriate CognitiveSkill** - 0.3-0.5 for civilians, 0.7-0.9 for guards/soldiers
2. **Use Task Actors generously** - More options = more realistic behavior
3. **Configure schedules thoughtfully** - Match NPC occupation to routine
4. **Monitor with World Subsystem** - Track population statistics
5. **Test with AI LOD** - Ensure NPCs behave correctly at all distances

## Extending the System

The framework is designed for extension:

- Inherit from `ALyraNPCCharacter` for custom NPC types
- Create new Task Actor types with custom need satisfaction
- Add new Behavior Tree tasks for specific behaviors
- Extend personality traits for your game's needs
- Create custom EQS generators for specialized queries

## Support

For questions, issues, or feature requests, please visit the project repository.

---

**LyraNPC** - Creating living, breathing worlds has never been easier.
