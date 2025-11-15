// Copyright LyraNPC Framework. All Rights Reserved.

#include "Components/LyraNPCScheduleComponent.h"
#include "LyraNPCModule.h"

ULyraNPCScheduleComponent::ULyraNPCScheduleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1.0f;
}

void ULyraNPCScheduleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (DailySchedule.Num() == 0)
	{
		InitializeDefaultSchedule(ELyraNPCArchetype::Villager);
	}

	UpdateCurrentScheduleBlock();
}

void ULyraNPCScheduleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateGameTime(DeltaTime);
	UpdateCurrentScheduleBlock();
}

void ULyraNPCScheduleComponent::InitializeDefaultSchedule(ELyraNPCArchetype Archetype)
{
	DailySchedule.Empty();

	switch (Archetype)
	{
	case ELyraNPCArchetype::Villager:
	case ELyraNPCArchetype::Worker:
	{
		// Wake up and morning routine
		FLyraNPCScheduleBlock Morning;
		Morning.StartHour = 6.0f;
		Morning.EndHour = 7.0f;
		Morning.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Morning"));
		Morning.LocationName = TEXT("Home");
		Morning.Priority = ELyraNPCTaskPriority::Normal;
		Morning.bMandatory = false;
		DailySchedule.Add(Morning);

		// Breakfast
		FLyraNPCScheduleBlock Breakfast;
		Breakfast.StartHour = 7.0f;
		Breakfast.EndHour = 8.0f;
		Breakfast.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
		Breakfast.LocationName = TEXT("Home");
		Breakfast.Priority = ELyraNPCTaskPriority::High;
		Breakfast.bMandatory = true;
		DailySchedule.Add(Breakfast);

		// Work
		FLyraNPCScheduleBlock Work;
		Work.StartHour = 8.0f;
		Work.EndHour = 12.0f;
		Work.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Work"));
		Work.LocationName = TEXT("Workplace");
		Work.Priority = ELyraNPCTaskPriority::High;
		Work.bMandatory = true;
		DailySchedule.Add(Work);

		// Lunch
		FLyraNPCScheduleBlock Lunch;
		Lunch.StartHour = 12.0f;
		Lunch.EndHour = 13.0f;
		Lunch.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
		Lunch.LocationName = TEXT("Tavern");
		Lunch.Priority = ELyraNPCTaskPriority::High;
		Lunch.bMandatory = true;
		DailySchedule.Add(Lunch);

		// More work
		FLyraNPCScheduleBlock AfternoonWork;
		AfternoonWork.StartHour = 13.0f;
		AfternoonWork.EndHour = 18.0f;
		AfternoonWork.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Work"));
		AfternoonWork.LocationName = TEXT("Workplace");
		AfternoonWork.Priority = ELyraNPCTaskPriority::High;
		AfternoonWork.bMandatory = true;
		DailySchedule.Add(AfternoonWork);

		// Dinner
		FLyraNPCScheduleBlock Dinner;
		Dinner.StartHour = 18.0f;
		Dinner.EndHour = 19.0f;
		Dinner.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
		Dinner.LocationName = TEXT("Home");
		Dinner.Priority = ELyraNPCTaskPriority::High;
		Dinner.bMandatory = true;
		DailySchedule.Add(Dinner);

		// Leisure
		FLyraNPCScheduleBlock Leisure;
		Leisure.StartHour = 19.0f;
		Leisure.EndHour = 21.0f;
		Leisure.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Leisure"));
		Leisure.LocationName = TEXT("Tavern");
		Leisure.Priority = ELyraNPCTaskPriority::Low;
		Leisure.bMandatory = false;
		Leisure.FlexibilityMinutes = 60.0f;
		DailySchedule.Add(Leisure);

		// Sleep
		FLyraNPCScheduleBlock Sleep;
		Sleep.StartHour = 21.0f;
		Sleep.EndHour = 6.0f; // Next day
		Sleep.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep"));
		Sleep.LocationName = TEXT("Home");
		Sleep.Priority = ELyraNPCTaskPriority::Critical;
		Sleep.bMandatory = true;
		DailySchedule.Add(Sleep);
		break;
	}
	case ELyraNPCArchetype::Guard:
	{
		// Morning patrol
		FLyraNPCScheduleBlock MorningPatrol;
		MorningPatrol.StartHour = 6.0f;
		MorningPatrol.EndHour = 12.0f;
		MorningPatrol.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Patrol"));
		MorningPatrol.LocationName = TEXT("PatrolRoute");
		MorningPatrol.Priority = ELyraNPCTaskPriority::High;
		MorningPatrol.bMandatory = true;
		DailySchedule.Add(MorningPatrol);

		// Lunch break
		FLyraNPCScheduleBlock Lunch;
		Lunch.StartHour = 12.0f;
		Lunch.EndHour = 13.0f;
		Lunch.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat"));
		Lunch.LocationName = TEXT("Barracks");
		Lunch.Priority = ELyraNPCTaskPriority::High;
		Lunch.bMandatory = true;
		DailySchedule.Add(Lunch);

		// Afternoon patrol
		FLyraNPCScheduleBlock AfternoonPatrol;
		AfternoonPatrol.StartHour = 13.0f;
		AfternoonPatrol.EndHour = 20.0f;
		AfternoonPatrol.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Patrol"));
		AfternoonPatrol.LocationName = TEXT("PatrolRoute");
		AfternoonPatrol.Priority = ELyraNPCTaskPriority::High;
		AfternoonPatrol.bMandatory = true;
		DailySchedule.Add(AfternoonPatrol);

		// Rest
		FLyraNPCScheduleBlock Rest;
		Rest.StartHour = 20.0f;
		Rest.EndHour = 6.0f;
		Rest.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep"));
		Rest.LocationName = TEXT("Barracks");
		Rest.Priority = ELyraNPCTaskPriority::High;
		Rest.bMandatory = true;
		DailySchedule.Add(Rest);
		break;
	}
	case ELyraNPCArchetype::Merchant:
	{
		// Open shop
		FLyraNPCScheduleBlock Shop;
		Shop.StartHour = 8.0f;
		Shop.EndHour = 18.0f;
		Shop.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Trade"));
		Shop.LocationName = TEXT("Shop");
		Shop.Priority = ELyraNPCTaskPriority::High;
		Shop.bMandatory = true;
		DailySchedule.Add(Shop);

		// Evening
		FLyraNPCScheduleBlock Evening;
		Evening.StartHour = 18.0f;
		Evening.EndHour = 22.0f;
		Evening.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Leisure"));
		Evening.LocationName = TEXT("Home");
		Evening.Priority = ELyraNPCTaskPriority::Normal;
		Evening.bMandatory = false;
		DailySchedule.Add(Evening);

		// Sleep
		FLyraNPCScheduleBlock Sleep;
		Sleep.StartHour = 22.0f;
		Sleep.EndHour = 8.0f;
		Sleep.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep"));
		Sleep.LocationName = TEXT("Home");
		Sleep.Priority = ELyraNPCTaskPriority::High;
		Sleep.bMandatory = true;
		DailySchedule.Add(Sleep);
		break;
	}
	default:
		// Generic schedule
		FLyraNPCScheduleBlock Day;
		Day.StartHour = 6.0f;
		Day.EndHour = 22.0f;
		Day.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Idle"));
		Day.LocationName = TEXT("Anywhere");
		Day.Priority = ELyraNPCTaskPriority::Low;
		Day.bMandatory = false;
		DailySchedule.Add(Day);

		FLyraNPCScheduleBlock Night;
		Night.StartHour = 22.0f;
		Night.EndHour = 6.0f;
		Night.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep"));
		Night.LocationName = TEXT("Home");
		Night.Priority = ELyraNPCTaskPriority::High;
		Night.bMandatory = true;
		DailySchedule.Add(Night);
		break;
	}

	UE_LOG(LogLyraNPC, Log, TEXT("Initialized schedule with %d blocks for archetype %d"), DailySchedule.Num(), static_cast<int32>(Archetype));
}

void ULyraNPCScheduleComponent::UpdateGameTime(float DeltaTime)
{
	// Convert real time to game time
	float GameTimeAdvance = (DeltaTime / 3600.0f) * TimeScale;
	CurrentGameHour += GameTimeAdvance;

	// Wrap around midnight
	while (CurrentGameHour >= 24.0f)
	{
		CurrentGameHour -= 24.0f;
	}
}

void ULyraNPCScheduleComponent::UpdateCurrentScheduleBlock()
{
	FLyraNPCScheduleBlock NewBlock = FindScheduleBlockForHour(CurrentGameHour);
	if (!NewBlock.ActivityTag.MatchesTagExact(CurrentScheduleBlock.ActivityTag))
	{
		CurrentScheduleBlock = NewBlock;
		UE_LOG(LogLyraNPC, Verbose, TEXT("Schedule changed to: %s at hour %.1f"),
			*CurrentScheduleBlock.ActivityTag.ToString(), CurrentGameHour);
	}
}

void ULyraNPCScheduleComponent::AddScheduleBlock(const FLyraNPCScheduleBlock& Block)
{
	DailySchedule.Add(Block);
}

void ULyraNPCScheduleComponent::ClearSchedule()
{
	DailySchedule.Empty();
}

FLyraNPCScheduleBlock ULyraNPCScheduleComponent::GetCurrentScheduledActivity() const
{
	return CurrentScheduleBlock;
}

FLyraNPCScheduleBlock ULyraNPCScheduleComponent::GetNextScheduledActivity() const
{
	if (DailySchedule.Num() == 0) return FLyraNPCScheduleBlock();

	float SmallestTimeDiff = 24.0f;
	FLyraNPCScheduleBlock NextBlock;

	for (const FLyraNPCScheduleBlock& Block : DailySchedule)
	{
		float TimeDiff = Block.StartHour - CurrentGameHour;
		if (TimeDiff <= 0.0f) TimeDiff += 24.0f; // Wrap to next day
		if (TimeDiff > 0.0f && TimeDiff < SmallestTimeDiff)
		{
			SmallestTimeDiff = TimeDiff;
			NextBlock = Block;
		}
	}

	return NextBlock;
}

FLyraNPCScheduleBlock ULyraNPCScheduleComponent::FindScheduleBlockForHour(float Hour) const
{
	for (const FLyraNPCScheduleBlock& Block : DailySchedule)
	{
		// Handle overnight blocks (e.g., 22:00 - 06:00)
		if (Block.StartHour > Block.EndHour)
		{
			if (Hour >= Block.StartHour || Hour < Block.EndHour)
			{
				return Block;
			}
		}
		else
		{
			if (Hour >= Block.StartHour && Hour < Block.EndHour)
			{
				return Block;
			}
		}
	}

	// Default idle block if nothing found
	FLyraNPCScheduleBlock IdleBlock;
	IdleBlock.ActivityTag = FGameplayTag::RequestGameplayTag(TEXT("Activity.Idle"));
	IdleBlock.Priority = ELyraNPCTaskPriority::Low;
	return IdleBlock;
}

bool ULyraNPCScheduleComponent::ShouldBeWorking() const
{
	return CurrentScheduleBlock.ActivityTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Activity.Work")));
}

bool ULyraNPCScheduleComponent::ShouldBeSleeping() const
{
	return CurrentScheduleBlock.ActivityTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Activity.Sleep")));
}

bool ULyraNPCScheduleComponent::ShouldBeEating() const
{
	return CurrentScheduleBlock.ActivityTag.MatchesTag(FGameplayTag::RequestGameplayTag(TEXT("Activity.Eat")));
}

void ULyraNPCScheduleComponent::SetGameHour(float NewHour)
{
	CurrentGameHour = FMath::Fmod(NewHour, 24.0f);
	UpdateCurrentScheduleBlock();
}

void ULyraNPCScheduleComponent::AdvanceTime(float Hours)
{
	CurrentGameHour += Hours;
	while (CurrentGameHour >= 24.0f)
	{
		CurrentGameHour -= 24.0f;
	}
	UpdateCurrentScheduleBlock();
}

bool ULyraNPCScheduleComponent::IsNightTime() const
{
	return CurrentGameHour < 6.0f || CurrentGameHour >= 20.0f;
}

bool ULyraNPCScheduleComponent::IsDayTime() const
{
	return !IsNightTime();
}

float ULyraNPCScheduleComponent::GetTimeUntilNextActivity() const
{
	FLyraNPCScheduleBlock NextBlock = GetNextScheduledActivity();
	float TimeDiff = NextBlock.StartHour - CurrentGameHour;
	if (TimeDiff <= 0.0f) TimeDiff += 24.0f;
	return TimeDiff;
}

bool ULyraNPCScheduleComponent::IsCurrentActivityFlexible() const
{
	return !CurrentScheduleBlock.bMandatory && CurrentScheduleBlock.FlexibilityMinutes > 0.0f;
}
