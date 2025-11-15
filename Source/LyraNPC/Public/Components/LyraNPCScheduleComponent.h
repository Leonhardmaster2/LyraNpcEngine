// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCScheduleComponent.generated.h"

/**
 * Component that manages NPC daily schedules and routines.
 * NPCs follow time-based schedules for work, rest, meals, etc.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Schedule"))
class LYRANPC_API ULyraNPCScheduleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCScheduleComponent();

	// Daily schedule blocks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
	TArray<FLyraNPCScheduleBlock> DailySchedule;

	// Current game hour (0-24)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule|Time")
	float CurrentGameHour = 6.0f;

	// Time scale (how fast game time passes relative to real time)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule|Time")
	float TimeScale = 24.0f; // 1 real hour = 1 game day

	// Whether schedule is active
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule|Settings")
	bool bScheduleActive = true;

	// Allow schedule flexibility based on needs
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule|Settings")
	bool bAllowNeedOverrides = true;

	// Currently active schedule block
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Schedule|State")
	FLyraNPCScheduleBlock CurrentScheduleBlock;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Initialize with default schedule for archetype
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void InitializeDefaultSchedule(ELyraNPCArchetype Archetype = ELyraNPCArchetype::Villager);

	// Add a schedule block
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void AddScheduleBlock(const FLyraNPCScheduleBlock& Block);

	// Clear all schedule blocks
	UFUNCTION(BlueprintCallable, Category = "Schedule")
	void ClearSchedule();

	// Get current scheduled activity
	UFUNCTION(BlueprintPure, Category = "Schedule")
	FLyraNPCScheduleBlock GetCurrentScheduledActivity() const;

	// Get next scheduled activity
	UFUNCTION(BlueprintPure, Category = "Schedule")
	FLyraNPCScheduleBlock GetNextScheduledActivity() const;

	// Check if NPC should be doing something specific
	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool ShouldBeWorking() const;

	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool ShouldBeSleeping() const;

	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool ShouldBeEating() const;

	// Time management
	UFUNCTION(BlueprintCallable, Category = "Schedule|Time")
	void SetGameHour(float NewHour);

	UFUNCTION(BlueprintCallable, Category = "Schedule|Time")
	void AdvanceTime(float Hours);

	UFUNCTION(BlueprintPure, Category = "Schedule|Time")
	float GetCurrentGameHour() const { return CurrentGameHour; }

	UFUNCTION(BlueprintPure, Category = "Schedule|Time")
	bool IsNightTime() const;

	UFUNCTION(BlueprintPure, Category = "Schedule|Time")
	bool IsDayTime() const;

	// Get time until next activity
	UFUNCTION(BlueprintPure, Category = "Schedule")
	float GetTimeUntilNextActivity() const;

	// Check if current activity is flexible (can be overridden by needs)
	UFUNCTION(BlueprintPure, Category = "Schedule")
	bool IsCurrentActivityFlexible() const;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateGameTime(float DeltaTime);
	void UpdateCurrentScheduleBlock();

	FLyraNPCScheduleBlock FindScheduleBlockForHour(float Hour) const;
};
