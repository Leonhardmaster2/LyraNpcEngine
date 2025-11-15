// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/Controllers/LyraNPCAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/LyraNPCIdentityComponent.h"
#include "Components/LyraNPCCognitiveComponent.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "Components/LyraNPCScheduleComponent.h"
#include "Tasks/LyraNPCTaskActor.h"
#include "Net/UnrealNetwork.h"
#include "LyraNPCModule.h"
#include "Kismet/GameplayStatics.h"

ALyraNPCAIController::ALyraNPCAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Create Behavior Tree component
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	// Create Blackboard component
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

	// Create Perception component
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);

	// Setup default LOD update rates
	LODUpdateRates.Add(ELyraNPCAILOD::Full, 0.0f);      // Every tick
	LODUpdateRates.Add(ELyraNPCAILOD::Reduced, 0.5f);   // Every 0.5 seconds
	LODUpdateRates.Add(ELyraNPCAILOD::Minimal, 2.0f);   // Every 2 seconds
	LODUpdateRates.Add(ELyraNPCAILOD::Dormant, 10.0f);  // Every 10 seconds

	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;

	bSetControlRotationFromPawnOrientation = true;
}

void ALyraNPCAIController::BeginPlay()
{
	Super::BeginPlay();
	SetupPerception();
}

void ALyraNPCAIController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALyraNPCAIController, CurrentAILOD);
}

void ALyraNPCAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	CacheComponents();

	// Initialize Blackboard
	if (DefaultBehaviorTree && DefaultBehaviorTree->BlackboardAsset)
	{
		UseBlackboard(DefaultBehaviorTree->BlackboardAsset, BlackboardComponent);
		UpdateBlackboardFromComponents();
	}

	// Start default Behavior Tree
	if (DefaultBehaviorTree)
	{
		StartBehaviorTree(DefaultBehaviorTree);
	}

	// Set initial AI LOD
	UpdateAILOD();

	UE_LOG(LogLyraNPC, Log, TEXT("LyraNPCAIController possessed pawn: %s"), *InPawn->GetName());
}

void ALyraNPCAIController::OnUnPossess()
{
	StopBehaviorTree();
	StopUsingCurrentTask();

	Super::OnUnPossess();
}

void ALyraNPCAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update AI LOD periodically
	TimeSinceLastLODCheck += DeltaTime;
	if (TimeSinceLastLODCheck >= LODCheckInterval)
	{
		UpdateAILOD();
		TimeSinceLastLODCheck = 0.0f;
	}

	// Update task timer
	UpdateTaskTimer(DeltaTime);

	// Update perception based on LOD
	if (CurrentAILOD <= ELyraNPCAILOD::Reduced)
	{
		UpdatePerception();
	}

	// Update Blackboard periodically
	if (CurrentAILOD != ELyraNPCAILOD::Dormant)
	{
		UpdateBlackboardFromComponents();
	}
}

void ALyraNPCAIController::CacheComponents()
{
	if (APawn* ControlledPawn = GetPawn())
	{
		IdentityComponent = ControlledPawn->FindComponentByClass<ULyraNPCIdentityComponent>();
		CognitiveComponent = ControlledPawn->FindComponentByClass<ULyraNPCCognitiveComponent>();
		NeedsComponent = ControlledPawn->FindComponentByClass<ULyraNPCNeedsComponent>();
		ScheduleComponent = ControlledPawn->FindComponentByClass<ULyraNPCScheduleComponent>();

		UE_LOG(LogLyraNPC, Log, TEXT("Cached components - Identity: %s, Cognitive: %s, Needs: %s, Schedule: %s"),
			IdentityComponent ? TEXT("Found") : TEXT("Missing"),
			CognitiveComponent ? TEXT("Found") : TEXT("Missing"),
			NeedsComponent ? TEXT("Found") : TEXT("Missing"),
			ScheduleComponent ? TEXT("Found") : TEXT("Missing"));
	}
}

void ALyraNPCAIController::SetupPerception()
{
	if (!AIPerceptionComponent) return;

	// Configure Sight sense
	UAISenseConfig_Sight* SightConfig = NewObject<UAISenseConfig_Sight>(this);
	SightConfig->SightRadius = 3000.0f;
	SightConfig->LoseSightRadius = 3500.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->SetMaxAge(10.0f);
	AIPerceptionComponent->ConfigureSense(*SightConfig);

	// Configure Hearing sense
	UAISenseConfig_Hearing* HearingConfig = NewObject<UAISenseConfig_Hearing>(this);
	HearingConfig->HearingRange = 2000.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->SetMaxAge(5.0f);
	AIPerceptionComponent->ConfigureSense(*HearingConfig);

	AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

	// Bind perception events
	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ALyraNPCAIController::OnPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ALyraNPCAIController::OnTargetPerceptionUpdated);
}

void ALyraNPCAIController::UpdateAILOD()
{
	float Distance = GetDistanceToNearestPlayer();

	ELyraNPCAILOD NewLOD = ELyraNPCAILOD::Dormant;

	if (Distance <= FullLODDistance)
	{
		NewLOD = ELyraNPCAILOD::Full;
	}
	else if (Distance <= ReducedLODDistance)
	{
		NewLOD = ELyraNPCAILOD::Reduced;
	}
	else if (Distance <= MinimalLODDistance)
	{
		NewLOD = ELyraNPCAILOD::Minimal;
	}

	SetAILOD(NewLOD);
}

void ALyraNPCAIController::SetAILOD(ELyraNPCAILOD NewLOD)
{
	if (CurrentAILOD != NewLOD)
	{
		ELyraNPCAILOD OldLOD = CurrentAILOD;
		CurrentAILOD = NewLOD;

		ApplyLODSettings();

		if (APawn* ControlledPawn = GetPawn())
		{
			if (ALyraNPCCharacter* NPCChar = Cast<ALyraNPCCharacter>(ControlledPawn))
			{
				OnAILODChanged.Broadcast(NPCChar, NewLOD);
			}
		}

		UE_LOG(LogLyraNPC, Verbose, TEXT("AI LOD changed from %d to %d"), static_cast<int32>(OldLOD), static_cast<int32>(NewLOD));
	}
}

float ALyraNPCAIController::GetDistanceToNearestPlayer() const
{
	if (!GetPawn()) return MAX_FLT;

	float NearestDistance = MAX_FLT;
	FVector MyLocation = GetPawn()->GetActorLocation();

	// Get all player controllers
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		APlayerController* PC = Iterator->Get();
		if (PC && PC->GetPawn())
		{
			float Distance = FVector::Dist(MyLocation, PC->GetPawn()->GetActorLocation());
			NearestDistance = FMath::Min(NearestDistance, Distance);
		}
	}

	return NearestDistance;
}

void ALyraNPCAIController::ApplyLODSettings()
{
	// Adjust tick rate
	float* TickRate = LODUpdateRates.Find(CurrentAILOD);
	if (TickRate)
	{
		if (*TickRate > 0.0f)
		{
			PrimaryActorTick.TickInterval = *TickRate;
		}
		else
		{
			PrimaryActorTick.TickInterval = 0.0f;
		}
	}

	// Adjust perception based on LOD
	if (AIPerceptionComponent)
	{
		if (CurrentAILOD >= ELyraNPCAILOD::Minimal)
		{
			AIPerceptionComponent->Deactivate();
		}
		else
		{
			AIPerceptionComponent->Activate();
		}
	}

	// Pause/Resume BT based on LOD
	if (CurrentAILOD == ELyraNPCAILOD::Dormant)
	{
		PauseBehaviorTree();
	}
	else
	{
		ResumeBehaviorTree();
	}
}

bool ALyraNPCAIController::StartBehaviorTree(UBehaviorTree* TreeToRun)
{
	if (!TreeToRun || !BehaviorTreeComponent) return false;

	if (TreeToRun->BlackboardAsset && !BlackboardComponent->IsCompatibleWith(TreeToRun->BlackboardAsset))
	{
		UseBlackboard(TreeToRun->BlackboardAsset, BlackboardComponent);
	}

	BehaviorTreeComponent->StartTree(*TreeToRun);

	UE_LOG(LogLyraNPC, Log, TEXT("Started Behavior Tree: %s"), *TreeToRun->GetName());
	return true;
}

void ALyraNPCAIController::StopBehaviorTree()
{
	if (BehaviorTreeComponent)
	{
		BehaviorTreeComponent->StopTree();
	}
}

void ALyraNPCAIController::PauseBehaviorTree()
{
	if (BehaviorTreeComponent)
	{
		BehaviorTreeComponent->PauseLogic(TEXT("AI LOD"));
	}
}

void ALyraNPCAIController::ResumeBehaviorTree()
{
	if (BehaviorTreeComponent)
	{
		BehaviorTreeComponent->ResumeLogic(TEXT("AI LOD"));
	}
}

void ALyraNPCAIController::UpdateBlackboardFromComponents()
{
	if (!BlackboardComponent) return;

	// Update from Identity
	if (IdentityComponent)
	{
		BlackboardComponent->SetValueAsEnum(TEXT("LifeState"), static_cast<uint8>(IdentityComponent->CurrentLifeState));
		BlackboardComponent->SetValueAsEnum(TEXT("Emotion"), static_cast<uint8>(IdentityComponent->CurrentEmotion));
		BlackboardComponent->SetValueAsVector(TEXT("HomeLocation"), IdentityComponent->HomeLocation);
		BlackboardComponent->SetValueAsVector(TEXT("WorkplaceLocation"), IdentityComponent->WorkplaceLocation);
	}

	// Update from Cognitive
	if (CognitiveComponent)
	{
		BlackboardComponent->SetValueAsEnum(TEXT("AlertLevel"), static_cast<uint8>(CognitiveComponent->AlertLevel));
		BlackboardComponent->SetValueAsFloat(TEXT("CognitiveSkill"), CognitiveComponent->CognitiveSkill);
	}

	// Update from Needs
	if (NeedsComponent)
	{
		BlackboardComponent->SetValueAsBool(TEXT("HasCriticalNeed"), NeedsComponent->HasCriticalNeed());
		BlackboardComponent->SetValueAsBool(TEXT("HasUrgentNeed"), NeedsComponent->HasUrgentNeed());
		BlackboardComponent->SetValueAsFloat(TEXT("Wellbeing"), NeedsComponent->GetOverallWellbeing());
		BlackboardComponent->SetValueAsEnum(TEXT("MostUrgentNeed"), static_cast<uint8>(NeedsComponent->GetMostUrgentNeed()));
	}

	// Update from Schedule
	if (ScheduleComponent)
	{
		BlackboardComponent->SetValueAsFloat(TEXT("CurrentHour"), ScheduleComponent->GetCurrentGameHour());
		BlackboardComponent->SetValueAsBool(TEXT("IsNightTime"), ScheduleComponent->IsNightTime());

		FLyraNPCScheduleBlock CurrentBlock = ScheduleComponent->GetCurrentScheduledActivity();
		BlackboardComponent->SetValueAsName(TEXT("ScheduledLocation"), CurrentBlock.LocationName);
	}

	// Update AI LOD
	BlackboardComponent->SetValueAsEnum(TEXT("AILOD"), static_cast<uint8>(CurrentAILOD));

	// Update threats
	BlackboardComponent->SetValueAsBool(TEXT("HasThreats"), HasThreats());
	BlackboardComponent->SetValueAsObject(TEXT("CurrentThreat"), GetHighestPriorityThreat());

	// Update current task
	BlackboardComponent->SetValueAsObject(TEXT("CurrentTask"), CurrentTask.Get());
	BlackboardComponent->SetValueAsBool(TEXT("IsUsingTask"), IsUsingTask());
}

void ALyraNPCAIController::SetBlackboardTask(ULyraNPCTaskActor* Task)
{
	if (BlackboardComponent)
	{
		BlackboardComponent->SetValueAsObject(TEXT("TargetTask"), Task);
		if (Task)
		{
			FTransform InteractionPoint = Task->GetBestInteractionPoint(Cast<ALyraNPCCharacter>(GetPawn()));
			BlackboardComponent->SetValueAsVector(TEXT("TaskLocation"), InteractionPoint.GetLocation());
			BlackboardComponent->SetValueAsRotator(TEXT("TaskRotation"), InteractionPoint.Rotator());
		}
	}
}

void ALyraNPCAIController::ClearBlackboardTask()
{
	if (BlackboardComponent)
	{
		BlackboardComponent->ClearValue(TEXT("TargetTask"));
		BlackboardComponent->ClearValue(TEXT("TaskLocation"));
		BlackboardComponent->ClearValue(TEXT("TaskRotation"));
	}
}

ULyraNPCTaskActor* ALyraNPCAIController::FindBestTask(FGameplayTag TaskType)
{
	TArray<AActor*> TaskActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), TaskActors);

	ULyraNPCTaskActor* BestTask = nullptr;
	float BestScore = 0.0f;

	ALyraNPCCharacter* NPCPawn = Cast<ALyraNPCCharacter>(GetPawn());
	if (!NPCPawn) return nullptr;

	for (AActor* Actor : TaskActors)
	{
		TArray<ULyraNPCTaskActor*> TaskComponents;
		Actor->GetComponents<ULyraNPCTaskActor>(TaskComponents);

		for (ULyraNPCTaskActor* TaskComponent : TaskComponents)
		{
			// Filter by task type if specified
			if (TaskType.IsValid() && !TaskComponent->TaskType.MatchesTag(TaskType))
			{
				continue;
			}

			float Score = TaskComponent->GetScoreForNPC(NPCPawn);
			if (Score > BestScore)
			{
				BestScore = Score;
				BestTask = TaskComponent;
			}
		}
	}

	return BestTask;
}

bool ALyraNPCAIController::StartUsingTask(ULyraNPCTaskActor* Task)
{
	if (!Task) return false;

	ALyraNPCCharacter* NPCPawn = Cast<ALyraNPCCharacter>(GetPawn());
	if (!NPCPawn) return false;

	if (Task->StartUsing(NPCPawn))
	{
		CurrentTask = Task;
		CurrentTaskRemainingTime = Task->GetRandomDuration();

		// Apply needs satisfaction
		if (NeedsComponent)
		{
			for (const auto& Pair : Task->NeedsSatisfaction)
			{
				// Initial satisfaction boost
				NeedsComponent->SatisfyNeed(Pair.Key, Pair.Value * 0.1f);
			}
		}

		// Update identity state
		if (IdentityComponent)
		{
			IdentityComponent->SetLifeState(ELyraNPCLifeState::UsingTask);
		}

		return true;
	}

	return false;
}

void ALyraNPCAIController::StopUsingCurrentTask()
{
	if (CurrentTask.IsValid())
	{
		ALyraNPCCharacter* NPCPawn = Cast<ALyraNPCCharacter>(GetPawn());
		CurrentTask->StopUsing(NPCPawn);
		CurrentTask.Reset();
		CurrentTaskRemainingTime = 0.0f;

		if (IdentityComponent)
		{
			IdentityComponent->SetLifeState(ELyraNPCLifeState::Idle);
		}
	}
}

void ALyraNPCAIController::UpdateTaskTimer(float DeltaTime)
{
	if (CurrentTask.IsValid() && CurrentTaskRemainingTime > 0.0f)
	{
		CurrentTaskRemainingTime -= DeltaTime;

		// Apply continuous needs satisfaction
		if (NeedsComponent)
		{
			float MinuteFraction = DeltaTime / 60.0f;
			for (const auto& Pair : CurrentTask->NeedsSatisfaction)
			{
				NeedsComponent->SatisfyNeed(Pair.Key, Pair.Value * MinuteFraction);
			}
		}

		if (CurrentTaskRemainingTime <= 0.0f)
		{
			StopUsingCurrentTask();
		}
	}
}

float ALyraNPCAIController::CalculateActionScore(FGameplayTag ActionType) const
{
	float Score = 0.0f;

	// Base score from needs
	if (NeedsComponent)
	{
		// Map action types to needs they satisfy
		// This is a simplified version - ideally use a data table
		if (ActionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Action.Eat"))))
		{
			Score = NeedsComponent->GetNeedPriority(ELyraNPCNeedType::Hunger);
		}
		else if (ActionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Action.Sleep"))))
		{
			Score = NeedsComponent->GetNeedPriority(ELyraNPCNeedType::Energy);
		}
		else if (ActionType.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Action.Socialize"))))
		{
			Score = NeedsComponent->GetNeedPriority(ELyraNPCNeedType::Social);
		}
	}

	// Apply intelligence variance
	if (CognitiveComponent)
	{
		Score = CognitiveComponent->ApplyIntelligenceVariance(Score);
	}

	return Score;
}

FGameplayTag ALyraNPCAIController::GetBestAction() const
{
	// This would be expanded with a full list of possible actions
	TArray<FGameplayTag> PossibleActions;
	PossibleActions.Add(FGameplayTag::RequestGameplayTag(TEXT("Action.Eat")));
	PossibleActions.Add(FGameplayTag::RequestGameplayTag(TEXT("Action.Sleep")));
	PossibleActions.Add(FGameplayTag::RequestGameplayTag(TEXT("Action.Work")));
	PossibleActions.Add(FGameplayTag::RequestGameplayTag(TEXT("Action.Socialize")));

	FGameplayTag BestAction;
	float BestScore = -1.0f;

	for (const FGameplayTag& Action : PossibleActions)
	{
		float Score = CalculateActionScore(Action);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestAction = Action;
		}
	}

	return BestAction;
}

AActor* ALyraNPCAIController::GetHighestPriorityThreat() const
{
	if (PerceivedThreats.Num() == 0) return nullptr;

	const FLyraNPCPerceptionResult* HighestThreat = &PerceivedThreats[0];
	for (const FLyraNPCPerceptionResult& Threat : PerceivedThreats)
	{
		if (Threat.ThreatLevel > HighestThreat->ThreatLevel)
		{
			HighestThreat = &Threat;
		}
	}

	return HighestThreat->PerceivedActor.Get();
}

void ALyraNPCAIController::UpdatePerception()
{
	if (!AIPerceptionComponent) return;

	// Clean up old perceptions
	for (int32 i = PerceivedThreats.Num() - 1; i >= 0; --i)
	{
		if (!PerceivedThreats[i].PerceivedActor.IsValid())
		{
			PerceivedThreats.RemoveAt(i);
		}
	}
}

void ALyraNPCAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (!AIPerceptionComponent || !CognitiveComponent) return;

	for (AActor* Actor : UpdatedActors)
	{
		FActorPerceptionBlueprintInfo Info;
		AIPerceptionComponent->GetActorsPerception(Actor, Info);

		// Apply cognitive skill to perception
		float NoticeChance = CognitiveComponent->GetNoticeChance(0.3f);
		if (FMath::FRand() > NoticeChance)
		{
			continue; // Didn't notice this actor based on intelligence
		}

		// Determine if this is a threat
		float ThreatLevel = 0.0f;
		// This would be expanded based on game logic
		// For now, just check if actor is tagged as threat
		if (Actor->ActorHasTag(TEXT("Threat")))
		{
			ThreatLevel = 1.0f;
		}

		if (ThreatLevel > 0.0f)
		{
			FLyraNPCPerceptionResult Result;
			Result.PerceivedActor = Actor;
			Result.LastKnownLocation = Actor->GetActorLocation();
			Result.LastSeenTime = GetWorld()->GetTimeSeconds();
			Result.ThreatLevel = ThreatLevel;
			Result.bIsVisible = Info.LastSensedStimuli.Num() > 0 && Info.LastSensedStimuli[0].WasSuccessfullySensed();

			// Add or update perception
			bool bFound = false;
			for (FLyraNPCPerceptionResult& Existing : PerceivedThreats)
			{
				if (Existing.PerceivedActor.Get() == Actor)
				{
					Existing = Result;
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				PerceivedThreats.Add(Result);
			}

			// Increase alertness
			CognitiveComponent->IncreaseAlertness(ThreatLevel * 0.5f);
		}
	}
}

void ALyraNPCAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (CognitiveComponent && Stimulus.WasSuccessfullySensed())
	{
		// Apply hearing or sight based on stimulus
		float AlertIncrease = 0.1f;
		if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>())
		{
			AlertIncrease = 0.2f;
		}

		CognitiveComponent->IncreaseAlertness(AlertIncrease);
	}
}
