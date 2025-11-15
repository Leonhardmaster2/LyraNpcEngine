// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCPathFollowingComponent.generated.h"

/**
 * Component for managing predetermined path following.
 * NPCs can follow patrol routes, travel paths, or custom waypoint systems.
 */
UCLASS(ClassGroup=(LyraNPC), meta=(BlueprintSpawnableComponent, DisplayName="LyraNPC Path Following"))
class LYRANPC_API ULyraNPCPathFollowingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraNPCPathFollowingComponent();

	// ===== PATH DATA =====

	// List of path points to follow
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Data")
	TArray<FLyraNPCPathPoint> PathPoints;

	// Should the path loop back to start?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Settings")
	bool bLoopPath = true;

	// Should the path be reversed after reaching the end? (ping-pong)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Settings")
	bool bPingPongPath = false;

	// Random wait time variance (adds randomness to wait times)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Settings")
	float WaitTimeVariance = 0.2f;

	// Can path be interrupted by higher priority actions?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path|Settings")
	bool bCanBeInterrupted = true;

	// ===== STATE =====

	// Current point index in path
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path|State")
	int32 CurrentPointIndex = 0;

	// Is currently following path?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path|State")
	bool bIsFollowingPath = false;

	// Is waiting at current point?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path|State")
	bool bIsWaitingAtPoint = false;

	// Time remaining to wait
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path|State")
	float RemainingWaitTime = 0.0f;

	// Direction (1 = forward, -1 = backward for ping-pong)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path|State")
	int32 PathDirection = 1;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// ===== PATH CONTROL =====

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void StartFollowingPath();

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void StopFollowingPath();

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void PausePath();

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void ResumePath();

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void ResetPath();

	UFUNCTION(BlueprintCallable, Category = "Path|Control")
	void SetPathPoints(const TArray<FLyraNPCPathPoint>& NewPoints);

	// ===== PATH QUERIES =====

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	FLyraNPCPathPoint GetCurrentPathPoint() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	FLyraNPCPathPoint GetNextPathPoint() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	FVector GetCurrentTargetLocation() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	bool HasReachedCurrentPoint() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	bool IsPathComplete() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	int32 GetRemainingPointCount() const;

	UFUNCTION(BlueprintPure, Category = "Path|Query")
	float GetPathProgress() const;

	// ===== PATH MANIPULATION =====

	UFUNCTION(BlueprintCallable, Category = "Path|Manipulation")
	void AddPathPoint(const FLyraNPCPathPoint& Point);

	UFUNCTION(BlueprintCallable, Category = "Path|Manipulation")
	void InsertPathPoint(int32 Index, const FLyraNPCPathPoint& Point);

	UFUNCTION(BlueprintCallable, Category = "Path|Manipulation")
	void RemovePathPoint(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Path|Manipulation")
	void ClearPath();

	// ===== MOVEMENT INTEGRATION =====

	UFUNCTION(BlueprintCallable, Category = "Path|Movement")
	void OnReachedPathPoint();

	UFUNCTION(BlueprintCallable, Category = "Path|Movement")
	void AdvanceToNextPoint();

	UFUNCTION(BlueprintPure, Category = "Path|Movement")
	ELyraNPCMovementStyle GetCurrentMovementStyle() const;

protected:
	virtual void BeginPlay() override;

private:
	void UpdateWaitTimer(float DeltaTime);
	int32 GetNextPointIndex() const;
	float GetWaitTimeWithVariance(float BaseTime) const;
};
