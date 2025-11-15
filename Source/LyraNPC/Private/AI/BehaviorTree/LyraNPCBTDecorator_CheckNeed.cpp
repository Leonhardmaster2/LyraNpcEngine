// Copyright LyraNPC Framework. All Rights Reserved.

#include "AI/BehaviorTree/LyraNPCBTDecorator_CheckNeed.h"
#include "AIController.h"
#include "Core/LyraNPCCharacter.h"
#include "Components/LyraNPCNeedsComponent.h"
#include "LyraNPCModule.h"

ULyraNPCBTDecorator_CheckNeed::ULyraNPCBTDecorator_CheckNeed()
{
	NodeName = "Check Need";
}

bool ULyraNPCBTDecorator_CheckNeed::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	ALyraNPCCharacter* NPC = Cast<ALyraNPCCharacter>(AIController->GetPawn());
	if (!NPC || !NPC->NeedsComponent)
	{
		return false;
	}

	float NeedValue = NPC->NeedsComponent->GetNeedValue(NeedType);

	bool bResult = false;
	switch (ComparisonOp)
	{
	case EArithmeticKeyOperation::Equal:
		bResult = FMath::IsNearlyEqual(NeedValue, ThresholdValue, 1.0f);
		break;
	case EArithmeticKeyOperation::NotEqual:
		bResult = !FMath::IsNearlyEqual(NeedValue, ThresholdValue, 1.0f);
		break;
	case EArithmeticKeyOperation::Less:
		bResult = NeedValue < ThresholdValue;
		break;
	case EArithmeticKeyOperation::LessOrEqual:
		bResult = NeedValue <= ThresholdValue;
		break;
	case EArithmeticKeyOperation::Greater:
		bResult = NeedValue > ThresholdValue;
		break;
	case EArithmeticKeyOperation::GreaterOrEqual:
		bResult = NeedValue >= ThresholdValue;
		break;
	}

	return bResult;
}

FString ULyraNPCBTDecorator_CheckNeed::GetStaticDescription() const
{
	FString OpString;
	switch (ComparisonOp)
	{
	case EArithmeticKeyOperation::Equal: OpString = TEXT("=="); break;
	case EArithmeticKeyOperation::NotEqual: OpString = TEXT("!="); break;
	case EArithmeticKeyOperation::Less: OpString = TEXT("<"); break;
	case EArithmeticKeyOperation::LessOrEqual: OpString = TEXT("<="); break;
	case EArithmeticKeyOperation::Greater: OpString = TEXT(">"); break;
	case EArithmeticKeyOperation::GreaterOrEqual: OpString = TEXT(">="); break;
	default: OpString = TEXT("?"); break;
	}

	return FString::Printf(TEXT("Need %s %s %.1f"),
		*UEnum::GetValueAsString(NeedType), *OpString, ThresholdValue);
}
