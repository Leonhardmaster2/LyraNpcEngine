// Copyright LyraNPC Framework. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "GameplayTagContainer.h"
#include "LyraNPCEnvQueryGenerator_Tasks.generated.h"

/**
 * EQS Generator: Generates task locations for NPC queries.
 * Finds all available tasks within a specified radius and filters by type.
 */
UCLASS()
class LYRANPC_API ULyraNPCEnvQueryGenerator_Tasks : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	ULyraNPCEnvQueryGenerator_Tasks();

	// Filter by specific task type (empty = all tasks)
	UPROPERTY(EditAnywhere, Category = "Generator")
	FGameplayTag TaskTypeFilter;

	// Maximum search radius
	UPROPERTY(EditAnywhere, Category = "Generator")
	float SearchRadius = 10000.0f;

	// Only include available (not occupied) tasks
	UPROPERTY(EditAnywhere, Category = "Generator")
	bool bOnlyAvailable = true;

	// Only include tasks the NPC can use
	UPROPERTY(EditAnywhere, Category = "Generator")
	bool bCheckNPCAccess = true;

	// Maximum number of tasks to return
	UPROPERTY(EditAnywhere, Category = "Generator")
	int32 MaxResults = 10;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};
