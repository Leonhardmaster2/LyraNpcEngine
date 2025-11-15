// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Core/LyraNPCTypes.h"
#include "LyraNPCBTDecorator_CheckNeed.generated.h"

/**
 * BT Decorator: Checks if a specific need meets the threshold condition.
 * Useful for branching behavior based on NPC needs.
 */
UCLASS()
class LYRANPC_API ULyraNPCBTDecorator_CheckNeed : public UBTDecorator
{
	GENERATED_BODY()

public:
	ULyraNPCBTDecorator_CheckNeed();

	// Which need to check
	UPROPERTY(EditAnywhere, Category = "Condition")
	ELyraNPCNeedType NeedType = ELyraNPCNeedType::Hunger;

	// Comparison operator
	UPROPERTY(EditAnywhere, Category = "Condition")
	TEnumAsByte<EArithmeticKeyOperation::Type> ComparisonOp = EArithmeticKeyOperation::Less;

	// Threshold value to compare against
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "100.0"))
	float ThresholdValue = 50.0f;

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual FString GetStaticDescription() const override;
};
