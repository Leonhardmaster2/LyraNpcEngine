// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/GOAP/LyraNPCGOAPTypes.h"

bool FLyraNPCWorldState::MeetsConditions(const FLyraNPCWorldState& Conditions) const
{
	// Check all required values
	for (const auto& Pair : Conditions.StateValues)
	{
		float MyValue = GetValue(Pair.Key, -MAX_FLT);
		if (!FMath::IsNearlyEqual(MyValue, Pair.Value, 0.01f))
		{
			return false;
		}
	}

	// Check all required flags
	for (const auto& Pair : Conditions.StateFlags)
	{
		bool MyFlag = GetFlag(Pair.Key, !Pair.Value);  // Default to opposite to fail
		if (MyFlag != Pair.Value)
		{
			return false;
		}
	}

	return true;
}

void FLyraNPCWorldState::ApplyState(const FLyraNPCWorldState& Other)
{
	for (const auto& Pair : Other.StateValues)
	{
		SetValue(Pair.Key, Pair.Value);
	}

	for (const auto& Pair : Other.StateFlags)
	{
		SetFlag(Pair.Key, Pair.Value);
	}
}

float FLyraNPCWorldState::GetStateDifference(const FLyraNPCWorldState& Other) const
{
	float Difference = 0.0f;

	// Count value differences
	for (const auto& Pair : Other.StateValues)
	{
		float MyValue = GetValue(Pair.Key, 0.0f);
		Difference += FMath::Abs(MyValue - Pair.Value);
	}

	// Count flag differences
	for (const auto& Pair : Other.StateFlags)
	{
		bool MyFlag = GetFlag(Pair.Key, false);
		if (MyFlag != Pair.Value)
		{
			Difference += 1.0f;
		}
	}

	return Difference;
}
