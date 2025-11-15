// Copyright LyraNPC Framework. All Rights Reserved.

#include "Navigation/LyraNPCPathFollowingComponent.h"
#include "LyraNPCModule.h"

ULyraNPCPathFollowingComponent::ULyraNPCPathFollowingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f;
}

void ULyraNPCPathFollowingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULyraNPCPathFollowingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsWaitingAtPoint)
	{
		UpdateWaitTimer(DeltaTime);
	}
}

void ULyraNPCPathFollowingComponent::StartFollowingPath()
{
	if (PathPoints.Num() > 0)
	{
		bIsFollowingPath = true;
		bIsWaitingAtPoint = false;
		UE_LOG(LogLyraNPC, Verbose, TEXT("Started following path with %d points"), PathPoints.Num());
	}
}

void ULyraNPCPathFollowingComponent::StopFollowingPath()
{
	bIsFollowingPath = false;
	bIsWaitingAtPoint = false;
	UE_LOG(LogLyraNPC, Verbose, TEXT("Stopped following path"));
}

void ULyraNPCPathFollowingComponent::PausePath()
{
	bIsFollowingPath = false;
}

void ULyraNPCPathFollowingComponent::ResumePath()
{
	if (PathPoints.Num() > 0)
	{
		bIsFollowingPath = true;
	}
}

void ULyraNPCPathFollowingComponent::ResetPath()
{
	CurrentPointIndex = 0;
	PathDirection = 1;
	bIsWaitingAtPoint = false;
	RemainingWaitTime = 0.0f;
}

void ULyraNPCPathFollowingComponent::SetPathPoints(const TArray<FLyraNPCPathPoint>& NewPoints)
{
	PathPoints = NewPoints;
	ResetPath();
}

FLyraNPCPathPoint ULyraNPCPathFollowingComponent::GetCurrentPathPoint() const
{
	if (PathPoints.IsValidIndex(CurrentPointIndex))
	{
		return PathPoints[CurrentPointIndex];
	}
	return FLyraNPCPathPoint();
}

FLyraNPCPathPoint ULyraNPCPathFollowingComponent::GetNextPathPoint() const
{
	int32 NextIndex = GetNextPointIndex();
	if (PathPoints.IsValidIndex(NextIndex))
	{
		return PathPoints[NextIndex];
	}
	return FLyraNPCPathPoint();
}

FVector ULyraNPCPathFollowingComponent::GetCurrentTargetLocation() const
{
	return GetCurrentPathPoint().Location;
}

bool ULyraNPCPathFollowingComponent::HasReachedCurrentPoint() const
{
	if (!GetOwner() || PathPoints.Num() == 0) return false;

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FLyraNPCPathPoint CurrentPoint = GetCurrentPathPoint();
	float Distance = FVector::Dist(OwnerLocation, CurrentPoint.Location);

	return Distance <= CurrentPoint.AcceptanceRadius;
}

bool ULyraNPCPathFollowingComponent::IsPathComplete() const
{
	if (!bLoopPath && !bPingPongPath)
	{
		return CurrentPointIndex >= PathPoints.Num() - 1 && HasReachedCurrentPoint();
	}
	return false;
}

int32 ULyraNPCPathFollowingComponent::GetRemainingPointCount() const
{
	if (PathDirection > 0)
	{
		return PathPoints.Num() - CurrentPointIndex - 1;
	}
	else
	{
		return CurrentPointIndex;
	}
}

float ULyraNPCPathFollowingComponent::GetPathProgress() const
{
	if (PathPoints.Num() == 0) return 0.0f;
	return static_cast<float>(CurrentPointIndex) / static_cast<float>(PathPoints.Num());
}

void ULyraNPCPathFollowingComponent::AddPathPoint(const FLyraNPCPathPoint& Point)
{
	PathPoints.Add(Point);
}

void ULyraNPCPathFollowingComponent::InsertPathPoint(int32 Index, const FLyraNPCPathPoint& Point)
{
	if (Index >= 0 && Index <= PathPoints.Num())
	{
		PathPoints.Insert(Point, Index);
		if (Index <= CurrentPointIndex)
		{
			CurrentPointIndex++;
		}
	}
}

void ULyraNPCPathFollowingComponent::RemovePathPoint(int32 Index)
{
	if (PathPoints.IsValidIndex(Index))
	{
		PathPoints.RemoveAt(Index);
		if (Index < CurrentPointIndex)
		{
			CurrentPointIndex--;
		}
		else if (Index == CurrentPointIndex && CurrentPointIndex >= PathPoints.Num())
		{
			CurrentPointIndex = FMath::Max(0, PathPoints.Num() - 1);
		}
	}
}

void ULyraNPCPathFollowingComponent::ClearPath()
{
	PathPoints.Empty();
	ResetPath();
}

void ULyraNPCPathFollowingComponent::OnReachedPathPoint()
{
	if (!bIsFollowingPath || PathPoints.Num() == 0) return;

	FLyraNPCPathPoint CurrentPoint = GetCurrentPathPoint();

	// Start waiting if there's wait time
	if (CurrentPoint.WaitTime > 0.0f)
	{
		bIsWaitingAtPoint = true;
		RemainingWaitTime = GetWaitTimeWithVariance(CurrentPoint.WaitTime);
		UE_LOG(LogLyraNPC, Verbose, TEXT("Waiting at path point %d for %.1f seconds"), CurrentPointIndex, RemainingWaitTime);
	}
	else
	{
		AdvanceToNextPoint();
	}
}

void ULyraNPCPathFollowingComponent::AdvanceToNextPoint()
{
	int32 NextIndex = GetNextPointIndex();

	if (NextIndex == CurrentPointIndex && !bLoopPath && !bPingPongPath)
	{
		// Path complete
		bIsFollowingPath = false;
		UE_LOG(LogLyraNPC, Verbose, TEXT("Path following complete"));
		return;
	}

	CurrentPointIndex = NextIndex;
	bIsWaitingAtPoint = false;

	UE_LOG(LogLyraNPC, Verbose, TEXT("Advanced to path point %d"), CurrentPointIndex);
}

ELyraNPCMovementStyle ULyraNPCPathFollowingComponent::GetCurrentMovementStyle() const
{
	return GetCurrentPathPoint().MovementStyle;
}

void ULyraNPCPathFollowingComponent::UpdateWaitTimer(float DeltaTime)
{
	RemainingWaitTime -= DeltaTime;
	if (RemainingWaitTime <= 0.0f)
	{
		AdvanceToNextPoint();
	}
}

int32 ULyraNPCPathFollowingComponent::GetNextPointIndex() const
{
	if (PathPoints.Num() == 0) return 0;

	int32 NextIndex = CurrentPointIndex + PathDirection;

	if (bPingPongPath)
	{
		if (NextIndex >= PathPoints.Num())
		{
			return PathPoints.Num() - 2; // Reverse direction
		}
		else if (NextIndex < 0)
		{
			return 1; // Reverse direction
		}
	}
	else if (bLoopPath)
	{
		if (NextIndex >= PathPoints.Num())
		{
			return 0;
		}
		else if (NextIndex < 0)
		{
			return PathPoints.Num() - 1;
		}
	}
	else
	{
		// Clamp to valid range
		NextIndex = FMath::Clamp(NextIndex, 0, PathPoints.Num() - 1);
	}

	return NextIndex;
}

float ULyraNPCPathFollowingComponent::GetWaitTimeWithVariance(float BaseTime) const
{
	float Variance = BaseTime * WaitTimeVariance;
	return BaseTime + FMath::FRandRange(-Variance, Variance);
}
